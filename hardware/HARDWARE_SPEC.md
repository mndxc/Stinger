# Stinger: Hardware Specification

> This document outlines the physical architecture, component selection, schematics logic, and printed circuit board (PCB) details of the Stinger. For the master pin mapping, refer to the root `SPEC.md`.

## 1. Hardware Specification

### 1.1 MCU Module

- **Model:** ESP32-S3-WROOM-1-N8R8 (Octal SPI).
- **Forbidden Pins:** GPIO 33–37 (internal PSRAM/Flash).
- **Strapping:** GPIO 0 HIGH (10kΩ); GPIO 45 (VDD_SPI) LOW for 3.3V flash operation. GPIO 1 is a ROM-printing bootstrapping pin (keep trace short, ensure high-impedance at boot). Matrix Col 1 moved to GPIO 15 to avoid GPIO 3 bootstrapping conflicts.

### 1.2 Audio Subsystem

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

### 1.3 TRRS Signal Path

| TRRS Pin (CTIA) | Signal | ES8388 Connection |
|---|---|---|
| Tip | Left Audio Out | LOUT1 via 1µF DC blocking cap, then PRTR5V0U2X TVS to GND |
| Ring 1 | Right Audio Out | ROUT1 via 1µF DC blocking cap, then PRTR5V0U2X TVS to GND |
| Ring 2 | Microphone In | LIN1 via 2.2kΩ bias resistor from MICBIAS, then PRTR5V0U2X TVS to GND |
| Sleeve | Ground | AGND |

> **TVS placement:** PRTR5V0U2X devices placed between TRRS jack pins and DC blocking caps / bias resistors. One package covers two channels — use two packages for all three active lines. Fourth channel is spare.

---

## 2. Protection & Glue Circuitry

### 2.1 USB Protection

- **VBUS PTC Fuse:** MF-MSMF050-2 (500mA hold, 1A trip, 1812) in series with VBUS before any other circuitry.
- **USB ESD:** USBLC6-2SC6 (SOT-23-6) placed immediately after USB-C connector pins, before PTC fuse. Rated 8kV contact discharge per IEC 61000-4-2.

### 2.2 TRRS Protection

- **TVS:** PRTR5V0U2X,215 (SOT-143, dual-channel). Two packages. Placed between jack pins and signal conditioning.

### 2.3 Matrix & Encoder Protection

- **Series Resistors:** 100Ω (0402) on every matrix Row and Column GPIO line (10 lines total).
- **Encoder Phase Lines:** 100Ω series resistors on all four encoder phase GPIOs (47, 14, 22, 23).

### 2.4 Programming & Reset Logic

- **Auto-Reset Circuit:** Dual NPN transistors (S8050) on RTS/DTR for automatic bootloader entry.
- **EN (Reset) Pin:** 10kΩ pull-up to 3.3V + 100nF cap to GND.
- **Boot Pin (GPIO 0):** 10kΩ pull-up to 3.3V + tactile button for manual Download Mode.
- **EN Button:** Tactile button for manual hardware reset.

### 2.5 Power & Status

- **Power Indicator:** Red LED on 3.3V rail via 1kΩ resistor.
- **Activity Indicator:** RGB LED on GPIO 48.
- **Ferrite Bead:** BLM18 series 0402 between 3.3V_D and 3.3V_A. 10µF + 100nF decoupling on 3.3V_A.

### 2.6 SD Card Detect

- MicroSD slot card-detect switch pin wired to GPIO 26.
- Firmware monitors via interrupt. Falling edge (removal) triggers clean unmount. Rising edge (insertion) triggers re-mount and Bank 01 reload.

---

## 3. Board Layout Strategy

- **Zone A (Rear):** USB-C connector, USBLC6-2SC6, PTC fuse, Power Switch, LDO.
- **Zone B (Center):** ESP32-S3, IDC headers for ribbon cables (Two 14-pin 2×7 headers).
- **Zone C (Front):** ES8388 Codec, TRRS jack, PRTR5V0U2X TVS devices.

> **ESD placement rule:** USBLC6-2SC6 within 5mm of USB-C pins, before PTC fuse. PRTR5V0U2X between TRRS jack pads and first passive on each signal line.

---

## 4. Signal Integrity & Safety

- **Analog Ground Island:** ES8388 AVDD and AGND on isolated copper island. Star-ground via 0Ω resistor near LDO.
- **Encoder Interrupts:** Dedicated GPIOs with 100Ω series resistors. Not matrix-scanned.
- **Ribbon Isolation:** SHIFT_KEY (Header B, Pin 9) kept distant from SPI_CLK (Header B, Pin 4).
- **Boot Window:** GPIO 1 tri-stated during power-on strapping window. Matrix Col 1 moved to GPIO 15 (non-strapping) to eliminate boot interference.
- **MCLK Boot Safety:** GPIO 0 MCLK output enabled only after ES8388 I2C init completes.
- **Debug:** All logs via USB CDC. UART0 unavailable in production.

---

## 5. Bill of Materials (Consolidated)

### 5.1 Primary ICs

| Component | Part | LCSC | Notes |
|---|---|---|---|
| MCU Module | ESP32-S3-WROOM-1-N8R8 | C2913148 | |
| Audio Codec | ES8388 | C365736 | QFN-28, 4×4mm |
| LDO Regulator | SGM2019-3.3 | C16193 | 3.3V, 300mA |

### 5.2 Protection

| Component | Part | LCSC | Notes |
|---|---|---|---|
| USB ESD | USBLC6-2SC6 | C7519 | SOT-23-6. ST. Within 5mm of USB-C pins. |
| TRRS TVS | PRTR5V0U2X,215 | C12333 | SOT-143, dual-channel. Nexperia. Qty: 2. |
| VBUS PTC Fuse | MF-MSMF050-2 | C17313 | 500mA hold / 1A trip. 1812. |

### 5.3 Passives (Consolidated)

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

### 5.4 Miscellaneous

| Component | Notes |
|---|---|
| USB-C Connector | Mid-mount or right-angle top-mount (e.g. LCSC C165948). |
| MicroSD Slot | Must include card-detect switch pin. Wire CD to GPIO 26. |
| TRRS Jack | PJ-320A or equivalent. 3.5mm board-mount. CTIA. |
| Tactile Buttons ×2 | EN reset + Boot mode. Standard 6×6mm. |
| Ferrite Bead | BLM18 series 0402. 3.3V_D → 3.3V_A isolation. |
| NPN Transistors ×2 | S8050. Auto-reset RTS/DTR circuit. |
| Red LED | Power indicator on 3.3V rail. |
| RGB LED | Activity/status on GPIO 48. **Located on Plate PCB.** |

### 5.5 Board-to-Board Interconnect

| Component | Spec | Qty | Notes |
|---|---|---|---|
| IDC Box Header (PCB) | 2×7, 2.54mm pitch, shrouded, THT | 4 | 2 per board (Brain + Plate). |
| IDC Crimp Socket (Cable) | 2×7, 2.54mm pitch, female | 4 | 2 per cable. |
| Flat Ribbon Cable | 14-conductor, 28AWG | 2 | 1.27mm wire pitch. |

