# Stinger: Firmware Specification

> This document details the firmware architecture, concurrency model, and display rendering logic for the Stinger USB Audio Soundboard. For the master pin mapping, refer to the root `SPEC.md`.

## 1. Display Specification

### 1.1 Hardware

- **Module:** Waveshare 2.9" E-paper, 296×128 pixels, portrait orientation (296px tall, 128px wide).
- **Interface:** SPI3 (dedicated bus).

### 1.2 Boot Splash Screen

On power-up, before Bank 01 is loaded, the Stinger displays a full-screen splash for 2 seconds:

```
┌────────────────────────────────┐
│                                │
│                                │
│                                │
│           STINGER              │  ← Large bold centered text (~48px tall)
│                                │
│             JP+                │  ← Smaller centered text (~20px tall)
│                                │
│                                │
│                                │
└────────────────────────────────┘
```

- **STINGER:** Large bold font, centered horizontally and vertically in the upper two-thirds of the display.
- **JP+:** Smaller font, centered beneath, with adequate spacing from STINGER.
- The splash uses the same full E-paper init refresh that occurs at power-on, so no additional refresh cycle is incurred.
- After 2 seconds, transition to the main Bank view (Zone layout, Section 1.3).
- If SD card is absent at boot, splash displays normally, then transitions to `NO SD CARD` state.

### 1.3 Main Screen Layout

```
┌────────────────────────────────┐  ← 0px
│         BANK INDICATOR         │  Zone 1: ~24px
├────────────────────────────────┤  ← 24px
│ ◄ L1  [     label text     ]   │
│ ◄ L2  [     label text     ]   │  Zone 2: 8 rows
│ ◄ L3  [     label text     ]   │  ~26px each = ~208px
│ ◄ L4  [     label text     ]   │
│    [ label text     ]  R1 ►    │
│    [ label text     ]  R2 ►    │
│    [ label text     ]  R3 ►    │
│    [ label text     ]  R4 ►    │
├────────────────────────────────┤  ← 232px
│  VOL: ██████░░  GAIN: ████░░░  │  Zone 3: ~32px
│  USB ●  SD ●  XFER ○           │
└────────────────────────────────┘  ← 296px
        ←── 128px wide ───→
```

**Zone 1 — Bank Indicator (~24px):**
- Centered bold: `Bank 03 / 99`
- Thin horizontal rule below.

**Zone 2 — Button Label Rows (~208px, ~26px per row):**
- Rows 1–4: Left buttons (L1–L4). `◄` at left edge, label left-aligned.
- Rows 5–8: Right buttons (R1–R4). Label right-aligned, `►` at right edge.
- Labels from `labels.cfg` if present, otherwise WAV filename without extension.
- Long labels truncated with ellipsis to fit 110px.
- Missing WAV slot displays `---`, key silenced.

**Zone 3 — Status Bar (~32px):**
- Row 1: Bar indicators for System Volume and SD Gain.
- Row 2: USB `●`, SD `●`, Transfer Mode `●` indicators.
- Lowest rendering priority — omitted cleanly if layout requires.

### 1.4 Bank Switching

- **Control:** Encoder 1. CW = next bank, CCW = previous bank. Wraps at boundaries.
- **At bank switch:** Full E-paper refresh. Active playback not interrupted.
- **Boot default:** Bank 01. Displays `NO BANK LOADED` if `/bank01/` absent.
- **On SD removal:** Displays `NO SD CARD`.
- **On SD insertion:** Reloads Bank 01 and refreshes display.

### 1.5 Label Files

```
# /bank01/labels.cfg
L1=Airhorn
L2=Rimshot
L3=Applause
L4=Sad Trombone
R1=Cash Register
R2=Crickets
R3=Wilhelm
R4=Ba Dum Tss
```

---

## 2. Firmware & Concurrency

### 2.1 Development Environment

- **Framework:** ESP-IDF v6.0 standalone.
- **ES8388 Driver:** Adapted from ESP-ADF `components/audio_hal/driver/es8388/`. Local component — no ESP-ADF pipeline dependency.
- **USB Stack:** TinyUSB (bundled with ESP-IDF v5.x). Composite device via `tusb_config.h`.
- **Target:** `esp32s3`. Dual-core task pinning via `xTaskCreatePinnedToCore`.

### 2.2 USB Device Descriptor Strings

| Descriptor Field   | Value     | Notes |
|--------------------|-----------|-------|
| Manufacturer       | `JP+`     | Appears in host OS device manager. |
| Product            | `Stinger` | Appears as audio device and MSC drive name. |
| Serial Number      | TBD       | Can use ESP32-S3 unique MAC-derived ID at runtime. |
| UAC1 Interface     | `Stinger Audio` | Audio device name shown in host audio settings. |
| MSC Volume Label   | `STINGER` | Drive label shown in host file manager during Transfer Mode. |

### 2.3 FreeRTOS Task Map

| Core   | Task Name      | Priority | Responsibility                                          |
|--------|----------------|----------|---------------------------------------------------------|
| Core 1 | AudioMixerTask | 25 (Max) | SD data fetch, 3-stream mixing, I2S DMA.                |
| Core 0 | UsbStackTask   | 22       | UAC1, HID, and MSC events.                              |
| Core 0 | InputScanTask  | 20       | 4×4 matrix scan (1ms). Encoder rotation via interrupts. |
| Core 1 | SdManagerTask  | 15       | File handles, MSC data piping, card-detect handler.     |
| Core 0 | DisplayTask    | 2        | E-paper SPI refreshes.                                  |

### 2.4 Playback Logic

- **Mode:** Retrigger — new key press restarts sample from beginning.
- **Sample Format:** 16-bit PCM, 48kHz Mono WAV.
- **Routing:** Samples sum into mono mic capture path → UAC1 capture stream to host.
- **Stream Summary:**
  - **UAC1 Capture (to PC):** Mono mic + mono SD sample (summed).
  - **UAC1 Playback (from PC):** Stereo USB audio → headphone output.
  - **Headphone Output:** Stereo USB playback + sidetone.
- **Sample Completion:** Sample channel zeroed on buffer exhaustion. Clean return to mic-only.

### 2.5 Encoder Logic

| Encoder   | Function       | Notes                                         |
|-----------|----------------|-----------------------------------------------|
| Encoder 1 | Bank Switching | CW = next bank, CCW = previous bank. Wraps.   |
| Encoder 2 | SD Gain        | Internal mixer scaling. No HID event emitted. |

### 2.6 Sample File Specification

- **Format:** 16-bit PCM, 48kHz Mono WAV.
- **Typical Length:** Under 6 seconds (~576 KB).
- **Maximum Length:** 30 seconds hard cap (~2.88 MB). Boot loader rejects oversized files, logs via USB CDC.
- **Loading Strategy:** Pre-loaded into PSRAM at boot or bank-switch. Per-file allocation.
- **Naming:** `/bankXX/YY.wav` — XX = zero-padded bank (01–99), YY = button index (01–08).
- **Button index mapping:** 01–04 = L1–L4, 05–08 = R1–R4.

> **PSRAM budget:** 8 × 2.88 MB theoretical max ≈ 23 MB. Hard cap enforced at load time. Typical usage well under 5 MB.

### 2.7 Concurrency & Transfer Mode

#### SD Mutex Strategy
All SD access gated by `sd_card_mutex`. No task reads or writes SD without holding this mutex.

#### SD Card-Detect Handler
On removal (GPIO 26 falling edge): `AudioMixerTask` drains and suspends, mutex released, filesystem unmounted. Display shows `NO SD CARD`. On insertion: filesystem re-mounted, Bank 01 loaded, `AudioMixerTask` resumed.

#### Transfer Mode State Machine
- **Entry:** 2-second Shift + Enc2 Switch hold. `AudioMixerTask` drains DMA buffer, suspends via `vTaskSuspend()`. Mutex released, filesystem unmounted, SD handed to USB MSC. Display shows `TRANSFER MODE`. MSC drive appears as `STINGER` on host.
- **Exit — Combo:** Same Shift + Enc2 Switch hold. Filesystem re-mounted, `AudioMixerTask` resumed.
- **Exit — USB Disconnect:** VBUS loss triggers same re-mount and resume. Do not rely on host enumeration signals.

#### Known Limitation — FAT32 Power Loss
FAT32 is not journaled. Power loss during Transfer Mode SD writes may cause filesystem corruption. Acceptable for prototype. Production mitigation: ESP-IDF FatFS wear-leveling layer.

---

## 3. Architectural Principles (Boilerplate vs. Application)

To ensure long-term maintainability and clear development flow, the Stinger firmware follows a strict **3-Layer Architectural Model**. All development (manual or AI-assisted) must adhere to these boundaries.

### 3.1 Layer 1: Hardware Abstraction & Drivers (Boilerplate)
**Location:** `firmware/components/{es8388, display, input_handler, ...}`
- **Responsibility:** Low-level register access, bus initialization (I2C/SPI/I2S), and GPIO management.
- **Constraint:** These modules must remain hardware-agnostic regarding the "Stinger" business logic. They provide the "tools" but do not know "why" they are being used.

### 3.2 Layer 2: Subsystem Services (Middleware)
**Location:** `firmware/components/{audio_mixer, sd_manager, usb_audio}`
- **Responsibility:** Managing complex stateful objects (audio buffers, file handles, USB descriptor tables).
- **Constraint:** These modules encapsulate FreeRTOS tasks and DMA complexity. They expose high-level APIs (e.g., `mixer_play_file()`) to the application layer.

### 3.3 Layer 3: Application Logic (The "Brain")
**Location:** `firmware/main/stinger_app.{h,c}`
- **Responsibility:** High-level soundboard behavior, state machine coordination, and user-interaction mapping.
- **Constraint:** This layer contains no low-level hardware code. It coordinates Layer 2 services to fulfill user requests (e.g., "If Key 1 is pressed in Bank 2 -> tell SD Manager to fetch file -> tell Mixer to play").
