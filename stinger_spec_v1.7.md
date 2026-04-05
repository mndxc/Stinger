# Stinger
## USB Audio Soundboard — Comprehensive Design & Manufacturing Specification
**Revision 1.7 (V37 Consolidated)**
**Project:** Stinger | **Manufacturer:** JP+

> This document serves as the production-ready specification and design guide for the Stinger USB Audio Soundboard by JP+. This revision applies the project name throughout, defines USB device descriptor strings, and adds the boot splash screen specification.

---

# PART I: MASTER SYSTEM SPECIFICATION

---

## 1. Architecture: The "Long-Strap" Console

The Stinger uses a modular Two-Board Hinge design to optimize signal integrity and mechanical stability.

- **Brain PCB (Base):** Houses the ESP32-S3, ES8388 Codec, MicroSD slot, Power Section, and Rear I/O.
- **Plate PCB (Interface):** Houses the 16-button matrix and the 2.9" E-paper display.
- **Interconnect:** Two standard 10-pin IDC ribbon cables (2.54mm pitch) connect the boards.

---

## 2. Hardware Specification

### 2.1 MCU Module

- **Model:** ESP32-S3-WROOM-1-N8R8 (Octal SPI).
- **Forbidden Pins:** GPIO 33–37 (internal PSRAM/Flash).
- **Strapping:** GPIO 0 HIGH (10kΩ); GPIO 45 (VDD_SPI) LOW for 3.3V flash operation.

### 2.2 Audio Subsystem

- **Codec:** ES8388 (QFN-28, 4×4mm) by Everest Semiconductor. Actively manufactured.
- **Configuration:** 48kHz / 16-bit PCM, I2S slave mode.
- **I2C Address:** 0x10 (CE pin LOW) or 0x11 (CE pin HIGH). CE pin state must be defined in schematic and matched in `board_config.h`.
- **Output:** Board-mount 3.5mm TRRS jack (CTIA) on the front panel.
- **Power Pins:**

| ES8388 Supply Pin | Connected Rail | Notes |
|---|---|---|
| DVDD | 3.3V_D | Digital core supply. |
| AVDD | 3.3V_A | ADC/DAC analog supply. Isolated via ferrite bead + 10µF cap. |
| PVDD | 3.3V_D | PLL supply. Shares with DVDD. |
| HPVDD | 3.3V_D | Headphone amp supply. Decouple with 10µF + 100nF locally. |

### 2.3 TRRS Signal Path

| TRRS Pin (CTIA) | Signal | ES8388 Connection |
|---|---|---|
| Tip | Left Audio Out | LOUT1 via 1µF DC blocking cap, then PRTR5V0U2X TVS to GND |
| Ring 1 | Right Audio Out | ROUT1 via 1µF DC blocking cap, then PRTR5V0U2X TVS to GND |
| Ring 2 | Microphone In | LIN1 via 2.2kΩ bias resistor from MICBIAS, then PRTR5V0U2X TVS to GND |
| Sleeve | Ground | AGND |

> **TVS placement:** PRTR5V0U2X devices placed between TRRS jack pins and DC blocking caps / bias resistors. One package covers two channels — use two packages for all three active lines. Fourth channel is spare.

---

## 3. Protection & Glue Circuitry

### 3.1 USB Protection

- **VBUS PTC Fuse:** MF-MSMF050-2 (500mA hold, 1A trip, 1812) in series with VBUS before any other circuitry.
- **USB ESD:** USBLC6-2SC6 (SOT-23-6) placed immediately after USB-C connector pins, before PTC fuse. Rated 8kV contact discharge per IEC 61000-4-2.

### 3.2 TRRS Protection

- **TVS:** PRTR5V0U2X,215 (SOT-143, dual-channel). Two packages. Placed between jack pins and signal conditioning.

### 3.3 Matrix & Encoder Protection

- **Series Resistors:** 100Ω (0402) on every matrix Row and Column GPIO line (8 lines total).
- **Encoder Phase Lines:** 100Ω series resistors on all four encoder phase GPIOs (47, 14, 22, 23).

### 3.4 Programming & Reset Logic

- **Auto-Reset Circuit:** Dual NPN transistors (S8050) on RTS/DTR for automatic bootloader entry.
- **EN (Reset) Pin:** 10kΩ pull-up to 3.3V + 100nF cap to GND.
- **Boot Pin (GPIO 0):** 10kΩ pull-up to 3.3V + tactile button for manual Download Mode.
- **EN Button:** Tactile button for manual hardware reset.

### 3.5 Power & Status

- **Power Indicator:** Red LED on 3.3V rail via 1kΩ resistor.
- **Activity Indicator:** RGB LED on GPIO 48.
- **Ferrite Bead:** BLM18 series 0402 between 3.3V_D and 3.3V_A. 10µF + 100nF decoupling on 3.3V_A.

### 3.6 SD Card Detect

- MicroSD slot card-detect switch pin wired to GPIO 26.
- Firmware monitors via interrupt. Falling edge (removal) triggers clean unmount. Rising edge (insertion) triggers re-mount and Bank 01 reload.

---

## 4. Master Pin Mapping & Matrix

### 4.1 Pin Assignment Table (Rev V37)

| Bus / Function    | Pin Assignment                                      | Note                                                          |
|-------------------|-----------------------------------------------------|---------------------------------------------------------------|
| USB D− / D+       | GPIO 19 / 20                                        |                                                               |
| I2S (Audio Data)  | BCLK: 38, LRCK: 39, DOUT: 40, DIN: 41              |                                                               |
| I2S MCLK          | GPIO 0                                              | CLK_OUT1. 256×Fs = 12.288 MHz. Enable after codec I2C init.  |
| I2C (Control)     | SDA: 4, SCL: 5                                      |                                                               |
| SPI2 (MicroSD)    | CS: 10, MOSI: 11, SCK: 12, MISO: 13                |                                                               |
| SPI3 (E-Paper)    | CS: 21, DIN: 16, CLK: 17, DC: 18, RST: 7, BUSY: 6  | CS on non-strapping pin.                                      |
| Matrix Rows       | GPIO 1, 2, 42, 43                                   | Rows 0–3. 100Ω series on all row lines.                      |
| Matrix Cols       | GPIO 8, 3, 44, 9                                    | Cols 0–3. 100Ω series on all col lines.                      |
| Encoder 1 A/B     | GPIO 47, 14                                         | Dedicated interrupts. Bank switching. 100Ω series.           |
| Encoder 2 A/B     | GPIO 22, 23                                         | Dedicated interrupts. SD Gain. 100Ω series.                  |
| SD Card Detect    | GPIO 26                                             | Active low. Card-detect switch on MicroSD slot.              |
| Status RGB LED    | GPIO 48                                             |                                                               |

### 4.2 Logical Matrix Map (4×4)

| Row \ Col       | Col 0 (GPIO 8) | Col 1 (GPIO 3) | Col 2 (GPIO 44) | Col 3 (GPIO 9) |
|-----------------|----------------|----------------|-----------------|----------------|
| Row 0 (GPIO 1)  | Trigger L1     | Trigger L2     | Trigger L3      | Trigger L4     |
| Row 1 (GPIO 2)  | Trigger R1     | Trigger R2     | Trigger R3      | Trigger R4     |
| Row 2 (GPIO 42) | Fav 1          | Fav 2          | Fav 3           | Fav 4          |
| Row 3 (GPIO 43) | Fav 5          | Shift Key      | Enc 1 Switch    | Enc 2 Switch   |

---

## 5. Display Specification

### 5.1 Hardware

- **Module:** Waveshare 2.9" E-paper, 296×128 pixels, portrait orientation (296px tall, 128px wide).
- **Interface:** SPI3 (dedicated bus).

### 5.2 Boot Splash Screen

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
- After 2 seconds, transition to the main Bank view (Zone layout, Section 5.3).
- If SD card is absent at boot, splash displays normally, then transitions to `NO SD CARD` state.

### 5.3 Main Screen Layout

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

### 5.4 Bank Switching

- **Control:** Encoder 1. CW = next bank, CCW = previous bank. Wraps at boundaries.
- **At bank switch:** Full E-paper refresh. Active playback not interrupted.
- **Boot default:** Bank 01. Displays `NO BANK LOADED` if `/bank01/` absent.
- **On SD removal:** Displays `NO SD CARD`.
- **On SD insertion:** Reloads Bank 01 and refreshes display.

### 5.5 Label Files

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

## 6. Firmware & Concurrency

### 6.1 Development Environment

- **Framework:** ESP-IDF v5.3 standalone.
- **ES8388 Driver:** Adapted from ESP-ADF `components/audio_hal/driver/es8388/`. Local component — no ESP-ADF pipeline dependency.
- **USB Stack:** TinyUSB (bundled with ESP-IDF v5.x). Composite device via `tusb_config.h`.
- **Target:** `esp32s3`. Dual-core task pinning via `xTaskCreatePinnedToCore`.

### 6.2 USB Device Descriptor Strings

| Descriptor Field   | Value     | Notes |
|--------------------|-----------|-------|
| Manufacturer       | `JP+`     | Appears in host OS device manager. |
| Product            | `Stinger` | Appears as audio device and MSC drive name. |
| Serial Number      | TBD       | Can use ESP32-S3 unique MAC-derived ID at runtime. |
| UAC1 Interface     | `Stinger Audio` | Audio device name shown in host audio settings. |
| MSC Volume Label   | `STINGER` | Drive label shown in host file manager during Transfer Mode. |

### 6.3 FreeRTOS Task Map

| Core   | Task Name      | Priority | Responsibility                                          |
|--------|----------------|----------|---------------------------------------------------------|
| Core 1 | AudioMixerTask | 25 (Max) | SD data fetch, 3-stream mixing, I2S DMA.                |
| Core 0 | UsbStackTask   | 22       | UAC1, HID, and MSC events.                              |
| Core 0 | InputScanTask  | 20       | 4×4 matrix scan (1ms). Encoder rotation via interrupts. |
| Core 1 | SdManagerTask  | 15       | File handles, MSC data piping, card-detect handler.     |
| Core 0 | DisplayTask    | 2        | E-paper SPI refreshes.                                  |

### 6.4 Playback Logic

- **Mode:** Retrigger — new key press restarts sample from beginning.
- **Sample Format:** 16-bit PCM, 48kHz Mono WAV.
- **Routing:** Samples sum into mono mic capture path → UAC1 capture stream to host.
- **Stream Summary:**
  - **UAC1 Capture (to PC):** Mono mic + mono SD sample (summed).
  - **UAC1 Playback (from PC):** Stereo USB audio → headphone output.
  - **Headphone Output:** Stereo USB playback + sidetone.
- **Sample Completion:** Sample channel zeroed on buffer exhaustion. Clean return to mic-only.

### 6.5 Encoder Logic

| Encoder   | Function       | Notes                                         |
|-----------|----------------|-----------------------------------------------|
| Encoder 1 | Bank Switching | CW = next bank, CCW = previous bank. Wraps.   |
| Encoder 2 | SD Gain        | Internal mixer scaling. No HID event emitted. |

### 6.6 Sample File Specification

- **Format:** 16-bit PCM, 48kHz Mono WAV.
- **Typical Length:** Under 6 seconds (~576 KB).
- **Maximum Length:** 30 seconds hard cap (~2.88 MB). Boot loader rejects oversized files, logs via USB CDC.
- **Loading Strategy:** Pre-loaded into PSRAM at boot or bank-switch. Per-file allocation.
- **Naming:** `/bankXX/YY.wav` — XX = zero-padded bank (01–99), YY = button index (01–08).
- **Button index mapping:** 01–04 = L1–L4, 05–08 = R1–R4.

> **PSRAM budget:** 8 × 2.88 MB theoretical max ≈ 23 MB. Hard cap enforced at load time. Typical usage well under 5 MB.

### 6.7 Concurrency & Transfer Mode

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

# PART II: KICAD HARDWARE DESIGN

---

## 7. Board Layout Strategy

- **Zone A (Rear):** USB-C connector, USBLC6-2SC6, PTC fuse, Power Switch, LDO.
- **Zone B (Center):** ESP32-S3, IDC headers for ribbon cables.
- **Zone C (Front):** ES8388 Codec, TRRS jack, PRTR5V0U2X TVS devices.

> **ESD placement rule:** USBLC6-2SC6 within 5mm of USB-C pins, before PTC fuse. PRTR5V0U2X between TRRS jack pads and first passive on each signal line.

## 8. Signal Integrity & Safety

- **Analog Ground Island:** ES8388 AVDD and AGND on isolated copper island. Star-ground via 0Ω resistor near LDO.
- **Encoder Interrupts:** Dedicated GPIOs with 100Ω series resistors. Not matrix-scanned.
- **Ribbon Isolation:** SHIFT_KEY (Header B, Pin 9) kept distant from SPI_CLK (Header B, Pin 4).
- **Boot Window:** GPIO 1 and GPIO 3 tri-stated during power-on strapping window.
- **MCLK Boot Safety:** GPIO 0 MCLK output enabled only after ES8388 I2C init completes.
- **Debug:** All logs via USB CDC. UART0 unavailable in production.

---

## 9. Bill of Materials (Consolidated)

### 9.1 Primary ICs

| Component | Part | LCSC | Notes |
|---|---|---|---|
| MCU Module | ESP32-S3-WROOM-1-N8R8 | C2913148 | |
| Audio Codec | ES8388 | C365736 | QFN-28, 4×4mm |
| LDO Regulator | SGM2019-3.3 | C16193 | 3.3V, 300mA |

### 9.2 Protection

| Component | Part | LCSC | Notes |
|---|---|---|---|
| USB ESD | USBLC6-2SC6 | C7519 | SOT-23-6. ST. Within 5mm of USB-C pins. |
| TRRS TVS | PRTR5V0U2X,215 | C12333 | SOT-143, dual-channel. Nexperia. Qty: 2. |
| VBUS PTC Fuse | MF-MSMF050-2 | C17313 | 500mA hold / 1A trip. 1812. |

### 9.3 Passives (Consolidated)

| Value | Package | Role |
|---|---|---|
| 100nF | 0402 | Bypass / decoupling on all IC supply pins. |
| 1µF | 0402 | DC blocking on TRRS audio lines. |
| 10µF | 0805 | Bulk decoupling: 3.3V_A, HPVDD, LDO output. |
| 100Ω | 0402 | Series ESD protection: matrix rows, cols, encoder phases. |
| 1kΩ | 0402 | Power LED current limit. |
| 2.2kΩ | 0402 | MICBIAS to TRRS Ring 2. |
| 4.7kΩ | 0402 | I2C SDA/SCL pull-ups. |
| 10kΩ | 0402 | GPIO pull-ups: EN, Boot, strapping. |

### 9.4 Miscellaneous

| Component | Notes |
|---|---|
| USB-C Connector | Mid-mount or right-angle top-mount (e.g. LCSC C165948). |
| MicroSD Slot | Must include card-detect switch pin. Wire CD to GPIO 26. |
| TRRS Jack | PJ-320A or equivalent. 3.5mm board-mount. CTIA. |
| Tactile Buttons ×2 | EN reset + Boot mode. Standard 6×6mm. |
| Ferrite Bead | BLM18 series 0402. 3.3V_D → 3.3V_A isolation. |
| NPN Transistors ×2 | S8050. Auto-reset RTS/DTR circuit. |
| Red LED | Power indicator on 3.3V rail. |
| RGB LED | Activity/status on GPIO 48. |

---

# PART III: PROJECT STRUCTURE

---

## 10. Repository Layout

```
stinger/
│
├── SPEC.md                        # This document (canonical project reference)
├── README.md                      # Project overview and build instructions
├── .gitignore
│
├── hardware/
│   ├── brain/
│   │   ├── brain.kicad_pro
│   │   ├── brain.kicad_sch
│   │   ├── brain.kicad_pcb
│   │   └── brain_bom.csv
│   ├── plate/
│   │   ├── plate.kicad_pro
│   │   ├── plate.kicad_sch
│   │   └── plate.kicad_pcb
│   ├── symbols/
│   │   └── ES8388.kicad_sym
│   ├── footprints/
│   │   └── ES8388_QFN28.kicad_mod
│   └── fabrication/
│       ├── brain/
│       └── plate/
│
├── firmware/
│   ├── CMakeLists.txt
│   ├── sdkconfig
│   ├── sdkconfig.defaults
│   │
│   ├── main/
│   │   ├── CMakeLists.txt
│   │   ├── main.c
│   │   ├── pin_map.h              # Single source of truth for all GPIO assignments
│   │   └── board_config.h         # Constants: sample rate, bank count, I2C address, etc.
│   │
│   └── components/
│       ├── es8388/                # ES8388 driver (adapted from ESP-ADF, no ADF dependency)
│       │   ├── CMakeLists.txt
│       │   ├── es8388.c
│       │   └── es8388.h
│       ├── audio_mixer/
│       │   ├── CMakeLists.txt
│       │   ├── audio_mixer.c
│       │   └── audio_mixer.h
│       ├── usb_audio/             # TinyUSB composite: UAC1 + HID + MSC
│       │   ├── CMakeLists.txt
│       │   ├── usb_descriptors.c  # Descriptor tables. Strings: "JP+", "Stinger".
│       │   ├── usb_audio.c
│       │   └── usb_audio.h
│       ├── sd_manager/            # SD mount, file loading, card-detect, mutex
│       │   ├── CMakeLists.txt
│       │   ├── sd_manager.c
│       │   └── sd_manager.h
│       ├── input_handler/         # Matrix scan, encoder ISRs, bank switching
│       │   ├── CMakeLists.txt
│       │   ├── input_handler.c
│       │   └── input_handler.h
│       └── display/               # E-paper driver + zone renderer + boot splash
│           ├── CMakeLists.txt
│           ├── display.c          # SPI driver, Waveshare 2.9" init, splash sequence
│           ├── display.h
│           ├── layout.c           # Zone rendering: splash, bank view, error states
│           └── layout.h
│
├── tools/
│   ├── flash.sh                   # Wrapper for idf.py flash monitor
│   └── wav_prep.py                # Validates WAV files to spec before SD copy
│
└── assets/
    ├── fonts/                     # Bitmap fonts for E-paper rendering
    └── sample_banks/
        └── bank01/
            ├── 01.wav
            └── labels.cfg
```

### 10.1 Key File Conventions

- **`pin_map.h`** — single source of truth for all GPIO numbers. No magic numbers elsewhere.
- **`board_config.h`** — all non-GPIO constants: sample rate (48000), bit depth (16), max banks (99), max sample frames, ES8388 I2C address, MCLK frequency (12288000), USB strings (`"JP+"`, `"Stinger"`), splash duration (2000ms).
- **`sdkconfig`** — committed to repo. All developers and AI tools build against identical ESP-IDF config.
- **`wav_prep.py`** — validates WAV files for spec compliance (48kHz, 16-bit, mono) and checks duration against 30-second cap before SD copy.

---

## Revision History

| Revision | Summary of Changes |
|----------|--------------------|
| 1.0 | Initial consolidated release. |
| 1.1 | GPIO 15 → 21 for SPI3 CS. Matrix map added. FreeRTOS task map added. |
| 1.2 | Encoder phases moved off matrix to dedicated interrupt GPIOs. Priority order corrected. PSRAM loading strategy defined. SD mutex and Transfer Mode state machine specified. |
| 1.3 | GPIO 15 → 22 and GPIO 46 → 23 for Encoder 2 A/B (strapping-pin conflicts). Maximum sample length added. Transfer Mode VBUS exit clarified. Citation artifact removed. |
| 1.4 | Sample format changed to mono. Mixer routing clarified. Sample completion behavior defined. Max sample length extended to 30s. Variable PSRAM allocation. |
| 1.5 | Codec replaced: WM8960 (EOL) → ES8388 (C365736). MCLK added to pin map. Encoder 1 assigned to bank switching. Display specification added. Label file format defined. 99-bank system defined. Project directory added. ESP-IDF v5.3 target defined. |
| 1.6 | Full protection circuitry restored: VBUS PTC fuse (C17313), USB ESD (C7519), TRRS TVS (C12333 ×2), matrix 100Ω series resistors. SD card-detect added (GPIO 26). BOM rationalised to 8 passive values. FAT32 power-loss limitation documented. |
| 1.7 | Project renamed to **Stinger** by JP+. Name applied throughout all sections and directory structure. USB descriptor strings defined: Manufacturer `JP+`, Product `Stinger`, UAC1 `Stinger Audio`, MSC volume `STINGER`. Serial number deferred to runtime MAC-derived ID. Boot splash screen specified (2s, STINGER / JP+ centered). USB strings added to `board_config.h` conventions. |
