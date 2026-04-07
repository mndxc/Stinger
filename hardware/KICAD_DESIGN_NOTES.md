# Stinger — KiCad Design Notes
**Revision 2.0 — Hybrid Assembly Edition**

> [!IMPORTANT]
> **Navigation & Source of Truth**  
> *   **Pin Mapping:** Refer to the root [SPEC.md](file:///Users/jonathanpool/Projects/Stinger/SPEC.md) (Source of Truth).
> *   **Master BOM:** All part numbers and LCSC codes are in [HARDWARE_SPEC.md §5](file:///Users/jonathanpool/Projects/Stinger/hardware/HARDWARE_SPEC.md#L98).
> *   **Sync:** Update `master_pins.json` before changing any schematic nets.

---

## 1. Assembly & Sourcing Strategy

The Stinger is a **two-board design** optimized for a mix of professional and manual construction:

| Board | Directory | Assembly | Key Contents |
|---|---|---|---|
| **Brain PCB** | `hardware/brain/` | **Mixed Mode** | **Professional SMT** for fine-pitch ICs/passives. **Manual THT** for IDC headers. |
| **Plate PCB** | `hardware/plate/` | **Hand-Soldered** | Buttons, Ghosting Diodes, WS2812B, and IDC Headers. |

---

## 2. MCU Module — ESP32-S3-WROOM-1-N8R8

### 2.1 Strapping Pins — Critical at Power-On
These pins determine the boot mode. Ensure the Brain PCB includes these 10kΩ pull-up/down resistors for the SMT machine to place.

| GPIO | Required Level | Implementation |
|---|---|---|
| **GPIO 0** (Boot) | HIGH | 10kΩ pull-up. Tactile button to GND for Download Mode. |
| **GPIO 45** (VDD_SPI) | LOW | 10kΩ pull-down. Selects 3.3V internal flash. |

### 2.2 MCLK Boot Safety (GPIO 0)
- GPIO 0 outputs the 12.288 MHz MCLK to the ES8388.
- **Schematic Note:** Ensure MCLK is enabled in firmware *only* after ES8388 I2C initialization to avoid interference with the boot strapping window.

---

## 3. Complete Pin Assignment (Master Reference)

> **Auto-Updated From:** `firmware/stinger/master_pins.json`.

| Function | GPIO | Net Name | Location |
|---|---|---|---|
| I2S MCLK | 0 | `I2S_MCLK` | Brain |
| I2C SDA | 4 | `I2C_SDA` | Brain |
| I2C SCL | 5 | `I2C_SCL` | Brain |
| SD CS | 10 | `SD_CS` | Brain |
| Matrix Rows | 1, 2, 42, 43, 25 | `MTX_R[0:4]` | Crossing Interconnect |
| Matrix Cols | 8, 15, 44, 9, 24 | `MTX_C[0:4]` | Crossing Interconnect |
| EPD SPI | 21, 16, 17, 18, 7, 6 | `EPD_*` | Crossing Interconnect |
| RGB LED | 48 | `STATUS_LED` | Crossing Interconnect |

---

## 4. Matrix & Display Implementation (Plate PCB)

### 4.1 5×5 Matrix Diodes
- **Ghosting Fix:** Place a **1N4148** diode at every switch node.
- **Orientation:** **Cathode to Row**. This allows the scanner (Rows as outputs, Columns as pull-up inputs) to operate correctly.
- **Footprint:** Use **SOD-123** for a compact hand-solder or **DO-35 (THT)** for a classic look.

### 4.2 E-Paper Display Module
- **Strategy:** Do not use a raw FPC connector.
- **Physical:** Use an **8-pin through-hole header (2.54mm)** on the Plate PCB.
- **Wiring:** Manually wire the Waveshare (or equivalent) E-paper module to these pads.
- **Pins:** VCC, GND, DIN, CLK, CS, DC, RST, BUSY.

---

## 5. Power & Supply Rails

- **LDO:** SGM2019-3.3 (Brain). Output `3.3V_D`.
- **Analog Island:** ES8388 `AVDD/AGND` isolated via ferrite bead and 10µF cap on the Brain.
- **Star-Ground:** Connect AGND to main GND at a single point near the LDO on the Brain.

---

## 6. Protection Circuitry Layout (Brain PCB)

- **USB ESD:** `USBLC6-2SC6` must be within **5mm** of USB-C pads.
- **TRRS TVS:** `PRTR5V0U2X` between jack pads and DC-blocking caps.
- **Series Resistors:** All **14 × 100Ω 0402** resistors should be placed near the ESP32-S3 pins on the Brain.

---

## 7. Board-to-Board Interconnect (IDC)

- **Headers:** Two **14-pin (2×7)** 2.54mm pitch shrouded headers.
- **Cable:** Standard 14-way flat ribbon.
- **Signal Integrity:** Use the 5 spare/GND pins on the headers to interleave ground between `EPD_CLK` and sensitive data lines like `SD_CS`.

---

## 8. Checklist Before Finalizing Design

- [ ] All 25 matrix buttons have a ghosting protection diode (Cathode to Row).
- [ ] Brain PCB contains all 0402 passives and fine-pitch ICs for SMT.
- [ ] IDC headers on Brain PCB are excluded from SMT pick-and-place to save on THT fees.
- [ ] Plate PCB pads for E-paper and IDC are through-hole for easy wiring.
- [ ] Pin map matches the root `SPEC.md` and `master_pins.json`.
- [ ] Analog ground island on Brain is isolated and star-grounded.

---

## 9. Sourcing Quick Links

> Refer to [HARDWARE_SPEC.md §5](file:///Users/jonathanpool/Projects/Stinger/hardware/HARDWARE_SPEC.md#L98) for exact LCSC parts and counts. Don't mirror the BOM here to prevent version drift.

