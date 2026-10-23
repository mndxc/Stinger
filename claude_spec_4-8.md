# Stinger — USB Audio Soundboard
## Design & Manufacturing Specification
**Revision: 1.6**
**Date: 2026-04-08**

---

## Part I: System Overview

### 1.1 Project Vision

Stinger is a standalone, driverless USB audio soundboard designed for real-time audio clip injection into a headset microphone stream. It operates as a standard USB Audio Class device requiring no drivers on the host PC.

**Core capabilities:**
- Trigger audio clips from MicroSD storage at the press of a button
- Mix injected clips into both the upstream microphone stream (heard by the PC) and the downstream headphone output (heard by the user)
- Pass-through of PC playback audio to headphones
- Full-duplex headset support via TRRS jack (earbud headphones + electret mic)

**Design philosophy:**
- Zero-software: driverless operation via USB UAC and HID classes
- Hardened latency: DMA-driven I2S audio processing with FreeRTOS task isolation
- Always-on UI: E-paper display for persistent clip labeling
- Modular two-board construction for signal integrity and mechanical flexibility

---

## Part II: Hardware Architecture

### 2.1 MCU

| Parameter | Value |
|---|---|
| Module | ESP32-S3-WROOM-1-N8R8 |
| Flash | 8 MB Octal SPI |
| PSRAM | 8 MB Octal SPI |
| Forbidden GPIOs | GPIO 33–37 (Octal PSRAM/Flash) |
| Non-existent GPIOs | GPIO 22–25 (do not exist on ESP32-S3) |
| Strapping pins | GPIO 0 (boot, pull HIGH 10kΩ), GPIO 3 (JTAG, floating), GPIO 45 (VDD_SPI, pull LOW for 3.3V), GPIO 46 (boot mode) |
| USB | GPIO 19 (D−), GPIO 20 (D+) |

**Boot notes:**
- GPIO 0 doubles as I2S MCLK output. The MCLK signal is only driven by the I2S peripheral after the boot strapping window has closed. Do not enable MCLK output during early boot initialization.
- GPIO 26–32 are nominally used for SPI flash/PSRAM. Avoid for general use.

---

### 2.2 Audio Subsystem

| Parameter | Value |
|---|---|
| Codec | ES8388 (QFN-28-EP, JLCPCB C365736) |
| Sample rate | 48 kHz |
| Bit depth | 16-bit PCM |
| Configuration bus | I2C (slave mode, address 0x20 CE=GND / 0x22 CE=PVDD) |
| Audio data bus | I2S slave (clocks driven by ESP32-S3) |
| MCLK | GPIO 0, CLK_OUT1, 256×Fs = 12.288 MHz |
| Headphone output | LOUT1/ROUT1 → TRRS TIP/RING1 via 1µF DC blocking caps |
| Mic input | Single-ended, LINPUT1/RINPUT1 from TRRS RING2 |
| Jack | 3.5mm TRRS, CTIA standard |

#### ES8388 Supply Pins

| Pin | Voltage | Function | Filtering |
|---|---|---|---|
| AVDD | 3.3V | Analog ADC/DAC core, PGA, reference | Series 10Ω resistor + 10µF + 100nF to AGND |
| HPVDD | 3.3V | Headphone output amplifier | 10Ω resistor from AVDD (per datasheet) + 10µF + 100nF |
| DVDD | 1.8–3.3V | Digital logic core | 100nF + 10µF local decoupling |
| PVDD | 1.8–3.3V | Digital IO buffer supply | 100nF + 10µF local decoupling; CE pin referenced to PVDD |

**LDO:** One dedicated low-noise LDO (LP2985-33 or equivalent) for all ES8388 supply pins, separate from the ESP32-S3 supply. LP2985 enable pin connected to a free ESP32-S3 GPIO for Bluetooth revision power gating. AMS1117 is not suitable for the codec supply due to insufficient noise performance.

**Datasheet notes (ES8388 User Guide):**
- One LDO recommended for the ES8388 supply due to analog noise sensitivity
- 10Ω resistor recommended between AVDD and HPVDD when sharing the same supply
- Decoupling capacitors should be placed as close to the package as possible
- 1kΩ pull-up resistors recommended on I2C CCLK and CDATA

#### Single-Ended Mic Input Circuit

```
AVDD ─── R_BIAS (2.2kΩ) ─── MIC_NODE ─── C_BLOCK (1µF) ─── LINPUT1 → ES8388
                                  │                               RINPUT1 (tied to LINPUT1, mono)
                             C_BYPASS (100nF)
                                  │
                                AGND
LINPUT2, RINPUT2 → AGND
```

- R_BIAS supplies DC bias to power the electret capsule in the headset mic
- C_BLOCK removes DC bias from the signal before the ES8388 input
- C_BYPASS filters AVDD noise from the bias node
- Both LINPUT1 and RINPUT1 receive the same mono mic signal for a centered recording stream
- Register 0x0A selects INPUT1; Register 0x09 sets PGA gain

#### ES8388 Relevant Register Sequence (startup)

Per the ES8388 User Guide, the initialization sequence is:
1. Chip Control (Reg 0–2): enable bias current and internal reference
2. ADC Power Management (Reg 3): power up ADC and line inputs
3. DAC Power Management (Reg 4): power up DAC and line outputs
4. Master Mode Control (Reg 8): set slave mode (Bit7=0)
5. ADC Control (Reg 9–10): select INPUT1, set PGA gain
6. DAC Control (Reg 23+): configure output routing and volume

---

### 2.3 ESD Protection

| Location | Part | Notes |
|---|---|---|
| TRRS jack | TPRCLAMP0524S (SOT-23-6L) | P2→TIP(L), P4→RING1(R), P5→RING2(MIC), P6=NC, P1+P3→AGND. Place within 2–3mm of jack. |
| USB-C data lines | USBLC6-2SC6 | Protects D+/D− and VBUS ESD. 1pF on data lines. |

**TRRS sleeve** connects directly to AGND — does not route through the ESD clamp.

---

### 2.4 Ground Strategy

Single unified ground plane (no split AGND/DGND). Physical layout separation replaces electrical separation:

- ES8388 and associated analog components clustered away from ESP32-S3 and switching regulators
- Analog decoupling caps return directly to ground plane via short, low-impedance vias
- Multiple parallel vias under ES8388 exposed pad to minimize return inductance
- Switching regulator and RF circuitry physically distant from the ES8388 input signal path

---

### 2.5 Storage

| Parameter | Value |
|---|---|
| Interface | SPI2 (dedicated) |
| Format | FAT32, WAV files (16-bit PCM, 48kHz, mono) |
| File naming | `/bankXX/YY.wav` (e.g. `/bank01/01.wav` for button L1) |

WAV format preferred over compressed formats to avoid real-time decode complexity and latency. Raw PCM after 44-byte header strip.

---

### 2.6 Display

| Parameter | Value |
|---|---|
| Type | 2.9" E-paper |
| Interface | SPI3 (dedicated) |
| Purpose | Persistent clip name labels, bank indicator, encoder mode indicator |

---

### 2.7 User Interface

#### Input Matrix — 4×4

| Row \ Col | Col 0 (GPIO 8) | Col 1 (GPIO 3) | Col 2 (GPIO 44) | Col 3 (GPIO 9) |
|---|---|---|---|---|
| Row 0 (GPIO 1) | Trigger L1 | Trigger L2 | Trigger L3 | Trigger L4 |
| Row 1 (GPIO 2) | Trigger R1 | Trigger R2 | Trigger R3 | Trigger R4 |
| Row 2 (GPIO 42) | Fav 1 | Fav 2 | Fav 3 | Fav 4 |
| Row 3 (GPIO 43) | Fav 5 | **STOP** | Enc Switch | *(spare)* |

**Notes:**
- Shift key removed — mode toggling handled by encoder switch
- STOP key (Row 3, Col 1): interrupts a currently playing clip mid-playback
- One spare cell available for future function assignment

#### Encoder

One rotary encoder with integrated push switch.

| Action | Function |
|---|---|
| Rotate (default mode) | System volume (USB HID Consumer Control) |
| Rotate (gain mode) | SD clip output gain (internal mixer scaling) |
| Short press | Toggle between volume mode and gain mode |

Current mode and value displayed on e-paper. Mode state persists until next press.

---

## Part III: Pin Assignment

### 3.1 Master Pin Table (Rev 1.6)

| Bus / Function | GPIO Assignment | Notes |
|---|---|---|
| USB D− / D+ | 19 / 20 | Fixed USB PHY pins |
| I2S BCLK / LRCK / DOUT / DIN | 38 / 39 / 40 / 41 | |
| I2S MCLK | 0 | CLK_OUT1. Strapping pin — MCLK only driven post-boot |
| I2C SDA / SCL | 4 / 5 | ES8388 control. 1kΩ pull-ups to PVDD |
| SPI2 MicroSD CS / MOSI / SCK / MISO | 10 / 11 / 12 / 13 | |
| SPI3 E-Paper CS / DIN / CLK / DC / RST / BUSY | 21 / 16 / 17 / 18 / 7 / 6 | |
| Matrix Row 0–3 | 1 / 2 / 42 / 43 | |
| Matrix Col 0–3 | 8 / 3 / 44 / 9 | GPIO 3 is strapping (floating) — confirm boot behavior |
| Encoder A / B | 47 / 14 | Dedicated edge-triggered interrupts |
| Encoder Switch | via matrix Row 3 Col 2 | GPIO 43 row, GPIO 44 col |
| Status RGB LED | 48 | |
| LP2985 LDO Enable | 15 | Active HIGH. Drive LOW to power down ES8388 |

### 3.2 Free / Unassigned GPIOs

GPIO 26 is at the edge of the SPI flash range — avoid. The following are available:

**Available:** GPIO 26 (caution), GPIO 27, GPIO 28, GPIO 29, GPIO 30, GPIO 31, GPIO 32 (all SPI flash range — avoid unless needed), GPIO 46 (strapping)

**Clean free pins:** None remaining after LP2985 enable assigned to GPIO 15. GPIO budget is essentially fully committed on this revision. Plan additional peripherals for next revision.

---

## Part IV: Power Architecture

### 4.1 Supply Rails

| Rail | Source | Consumers |
|---|---|---|
| VBUS (5V) | USB-C input | Input to regulators |
| 3.3V digital | AMS1117-3.3 or equivalent | ESP32-S3, SPI peripherals, E-paper, LEDs |
| 3.3V analog | LP2985-33 (low-noise LDO) | ES8388 AVDD, HPVDD, DVDD, PVDD |

### 4.2 Input Protection

| Component | Location | Purpose |
|---|---|---|
| 500mA PTC resettable fuse | VBUS input | Overcurrent protection |
| USBLC6-2SC6 | USB-C D+/D− | ESD and cable discharge on data lines |

### 4.3 LP2985 Enable Pin Behavior

- Enable HIGH (default via pull-up): ES8388 powered, full USB audio operation
- Enable LOW (driven by ESP32-S3 GPIO 15): ES8388 powered down, Bluetooth-only mode (future revision)
- On re-enable: firmware must re-execute full ES8388 I2C initialization sequence — register state does not persist through power cycle
- I2C bus pull-ups are to PVDD (from LP2985). When LP2985 is disabled, confirm other I2C devices are not present on the same bus or handle bus isolation in firmware

---

## Part V: Firmware Architecture

### 5.1 USB Audio Clock Reconciliation

The ESP32-S3 bridges two clock domains: the USB host clock (PC) and the ES8388 I2S clock. Implementation plan:

| Phase | Approach | Notes |
|---|---|---|
| Rev 1.6 bring-up | Free-running (Option 1) | ES8388 clocked independently of USB. Accept drift, identify real-world impact before optimizing. |
| Post-bring-up | USB feedback endpoint (Option 2) | UAC explicit feedback endpoint reports sample consumption rate to PC host. ESP-IDF has infrastructure for this. |
| If needed | Software PLL (Option 3) | Monitor USB SOF signal via ESP32-S3 USB peripheral registers. Count I2S samples per SOF interval. Correct via sample stuffing/slipping with integrating controller. |

No board respin required to progress through Options 1–3. All are firmware-only changes. Option 4 (hardware VCXO/PLL) would require a respin and is not planned.

### 5.2 Audio Mixing Model

Three simultaneous audio streams managed in firmware:

| Stream | Direction | Path |
|---|---|---|
| PC playback | USB → I2S → ES8388 DAC → headphones | Heard by user only |
| Headset mic | ES8388 ADC → I2S → USB | Sent to PC |
| SD clip | MicroSD → RAM buffer → mixed into both above | Heard by user AND sent to PC |

**Mix equations (per sample):**
```
upstream_to_PC   = ADC_sample + clip_sample (scaled)
headphone_out    = PC_playback_sample + clip_sample (scaled)
```

**SD card buffering:** SD reads are decoupled from the audio processing loop via a RAM buffer. A lower-priority task fills the buffer from SD; the audio task reads from the buffer. Buffer depth sufficient to absorb worst-case SD read latency (target: 200–500ms of audio ≈ 20–50KB mono 16-bit 48kHz).

**Clip trigger behavior:** Retrigger (new button press restarts the clip from the beginning).

**STOP button:** Sets clip state to idle immediately, zeroing the clip sample contribution on the next mix iteration.

### 5.3 FreeRTOS Task Map

| Core | Task | Priority | Responsibility |
|---|---|---|---|
| Core 0 | UsbStackTask | 24 | UAC, HID Consumer Control, USB feedback endpoint |
| Core 0 | InputScanTask | 20 | 4×4 matrix scan (1ms), encoder interrupt handling |
| Core 0 | DisplayTask | 2 | E-paper SPI refresh, mode/gain display updates |
| Core 1 | AudioMixerTask | 22 | I2S DMA callbacks, 3-stream mixing, clip state machine |
| Core 1 | SdManagerTask | 15 | File I/O, clip buffer fill, bank management |

### 5.4 Encoder Firmware Behavior

- Encoder A/B on GPIO 47/14 as edge-triggered interrupts
- Encoder switch state read from matrix scan (Row 3, Col 2)
- Mode state: `VOLUME_MODE` (default) or `GAIN_MODE`
- In `VOLUME_MODE`: encoder rotation sends HID Consumer Control volume up/down to PC
- In `GAIN_MODE`: encoder rotation adjusts internal `clip_gain` scalar (0.0–1.0 or dB scale)
- Short press of encoder switch toggles mode; e-paper updates to show current mode and value
- Debounce encoder switch in firmware (5–10ms)

---

## Part VI: Physical Design

### 6.1 Board Construction

Two-board hinge design:

| Board | Contents |
|---|---|
| Brain PCB | ESP32-S3, ES8388, MicroSD, power section, TRRS jack, USB-C, rear I/O |
| Plate PCB | 4×4 button matrix, rotary encoder, E-paper display |

Interconnect: Two 10-pin IDC ribbon cables (2.54mm pitch).

### 6.2 Layout Guidelines

- ES8388 and analog components in a dedicated cluster, physically distant from ESP32-S3 and switching regulators
- TRRS jack and TPRCLAMP0524S within 2–3mm of each other — clamp placed between jack and ES8388 in the signal path
- All ES8388 decoupling caps within 2mm of their respective supply pins
- I2S traces (GPIO 38–41) routed with consistent length matching
- MCLK (GPIO 0) treated as a clock trace — keep short, avoid coupling to analog area
- I2C pull-up resistors (1kΩ) placed close to ES8388 CCLK/CDATA pins
- MIC_INP signal trace kept short and away from digital switching traces

### 6.3 Key JLCPCB Part Numbers

| Component | JLCPCB Part # |
|---|---|
| ES8388 codec | C365736 |
| ESP32-S3-WROOM-1-N8R8 | C2913148 |
| TPRCLAMP0524S | (verify current stock) |
| USBLC6-2SC6 | (verify current stock) |

---

## Part VII: Future Revision Notes (Rev 2.0)

- **Bluetooth audio:** Power down ES8388 via LP2985 enable pin (GPIO 15). ESP32-S3 handles audio via BT Classic A2DP or BLE Audio. No hardware changes required — firmware only.
- **GPIO budget:** Rev 1.6 is essentially fully committed. Any additional peripherals require a respin.
- **Clock quality:** If audio quality testing reveals audible clock drift artifacts, evaluate Option 3 (software PLL) before considering Option 4 (hardware VCXO — requires respin).
- **Differential mic input:** The ES8388 supports pseudo-differential and fully-differential mic input modes for improved noise rejection. A future TRRS-alternative connector (e.g. XLR or balanced TRS) could take advantage of this.