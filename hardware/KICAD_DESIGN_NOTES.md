# Stinger — KiCad Design Notes
**Reference Rev:** SPEC.md 1.8 / HARDWARE_SPEC.md / master_pins.json (Rev V37)

> Working notes for schematic capture and PCB layout in KiCad. Synthesised from all project spec documents. Always cross-reference `master_pins.json` as the pin source of truth.

---

## ⚠️ Critical Highlights — Read Before Opening KiCad

These are the highest-risk mistakes for this design. Each point is expanded in the relevant section below.

### Strapping Pins — Will Break Boot if Wrong
- **GPIO 45 (VDD_SPI)** must be tied LOW (10kΩ to GND) for 3.3V flash. If floating, flash voltage is undefined.
- **GPIO 0** must have a 10kΩ pull-up to 3.3V. It doubles as I2S MCLK — firmware must not enable MCLK until after ES8388 I2C init completes. Annotate this in the schematic.
- **GPIO 15 (MTX_C1)** replaces the original GPIO 3 assignment to eliminate bootstrapping conflicts during power-on. See §3.1.
- **GPIO 1 (MTX_R0)** is a strapping pin sampled at boot for ROM log printing.
  - **Care Needed:** Do not add external pull-up/down resistors. Ensure the row remains high-impedance at boot (buttons open). Keep traces short to minimize parasitic capacitance that could delay the strapping voltage ramp.

### Forbidden GPIOs
- **GPIO 33–37** are internal to the N8R8 module (Octal PSRAM). Do **not** break these out or connect to any net.

### ES8388 CE Pin (I2C Address)
- CE pin sets the I2C address: **0x10 if LOW**, 0x11 if HIGH. Tie it definitively in the schematic and match the value in `board_config.h`. Never leave floating.

### ESD Placement Order Matters
- **USB:** USBLC6-2SC6 must appear **before** the PTC fuse in the circuit, and within **5mm** of the USB-C D+/D− pads.
- **TRRS:** PRTR5V0U2X must sit **between** the jack pads and the first downstream passive (DC-blocking cap or bias resistor) on each signal line.

### Analog Ground Island
- ES8388 AGND needs an isolated copper pour connected to main GND via a **single 0Ω star-ground resistor** near the LDO. Do not merge with the main GND pour — this will corrupt the analog supply.

### Series Resistors — All 12 of Them
- 100Ω on every matrix row (×4), every matrix col (×4), and every encoder phase line (×4). Easy to miss during schematic capture.

---

## 1. Board Architecture

The Stinger is a **two-board design** connected by two **14-pin IDC ribbon cables** (2.54mm pitch):

| Board | Contents |
|---|---|
| **Brain PCB (Base)** | ESP32-S3, ES8388 Codec, MicroSD slot, Power Section, Rear I/O (USB-C, TRRS) |
| **Plate PCB (Interface)** | 4×4 button matrix, 2.9" E-paper display |

**IDC Interconnect reminder:** Route SHIFT_KEY (Header B, Pin 9) away from SPI_CLK (Header B, Pin 4) to prevent crosstalk on the ribbon cable.

---

## 2. MCU Module — ESP32-S3-WROOM-1-N8R8

### 2.1 Forbidden Pins — Do NOT route to pads
| GPIO Range | Reason |
|---|---|
| GPIO 33–37 | Internal PSRAM / Octal Flash. These lines are internal to the module and must not be exposed. |

### 2.2 Strapping Pins — Critical at Power-On
These pins are sampled by the ESP32-S3 hardware **during the boot window** before firmware runs. Their state at power-on determines boot mode and flash voltage.

| GPIO | Required Level | Implementation |
|---|---|---|
| **GPIO 0** (Boot) | HIGH for normal boot | 10kΩ pull-up to 3.3V. Tactile button to GND for Download Mode. |
| **GPIO 45** (VDD_SPI) | LOW for 3.3V flash | Tie directly to GND or through 10kΩ pull-down. **Never float.** |
| **GPIO 46** | Not used in pin map — treat with care | Leave floating or tie LOW. Do not use as output. |

> **GPIO 1 boot window:** This pin is assigned to Matrix Row 0. It will be tri-stated during the strapping window — ensure Row 0 is driven LOW by default or add a weak pull-down. Matrix Col 1 was moved to **GPIO 15** specifically to avoid the GPIO 3 strapping concern.

### 2.3 MCLK Boot Safety (GPIO 0)
- GPIO 0 doubles as **I2S MCLK (CLK_OUT1)**, outputting 12.288 MHz to the ES8388.
- The MCU firmware must **not enable MCLK until after ES8388 I2C initialization completes**.
- In KiCad: the physical connection from GPIO 0 → ES8388 MCLK pin is permanent. The timing safety is a firmware concern, but it is worth annotating in the schematic notes.

### 2.4 VDD_SPI Note (GPIO 45)
Setting GPIO 45 LOW selects **3.3V Flash operation** (correct for this module). Confirm the `ESP32-S3-WROOM-1-N8R8` module's datasheet confirms 3.3V SPI flash internally — this strapping controls internal routing within the module, not an external pin per se, but it still must be defined in your schematic if breaking out the pin.

---

## 3. Complete Pin Assignment (Master Reference)

> **Source of Truth:** `firmware/stinger/master_pins.json`. Do NOT deviate.

| Function | GPIO | Net Name Suggestion | Notes |
|---|---|---|---|
| USB D− | 19 | `USB_DM` | Via USBLC6-2SC6 |
| USB D+ | 20 | `USB_DP` | Via USBLC6-2SC6 |
| I2S BCLK | 38 | `I2S_BCLK` | To ES8388 SCLK |
| I2S LRCK | 39 | `I2S_LRCK` | To ES8388 LRCK |
| I2S DOUT | 40 | `I2S_DOUT` | ESP → ES8388 (Tx) |
| I2S DIN | 41 | `I2S_DIN` | ES8388 → ESP (Rx) |
| I2S MCLK | 0 | `I2S_MCLK` | 12.288 MHz CLK_OUT1. Also boot/download button. |
| I2C SDA | 4 | `I2C_SDA` | ES8388 control. 4.7kΩ pull-up. |
| I2C SCL | 5 | `I2C_SCL` | ES8388 control. 4.7kΩ pull-up. |
| SPI2 CS | 10 | `SD_CS` | MicroSD chip select |
| SPI2 MOSI | 11 | `SD_MOSI` | MicroSD |
| SPI2 SCK | 12 | `SD_SCK` | MicroSD |
| SPI2 MISO | 13 | `SD_MISO` | MicroSD |
| SPI3 CS | 21 | `EPD_CS` | E-paper — non-strapping pin |
| SPI3 DIN | 16 | `EPD_DIN` | E-paper data in |
| SPI3 CLK | 17 | `EPD_CLK` | E-paper clock |
| SPI3 DC | 18 | `EPD_DC` | E-paper data/command |
| SPI3 RST | 7 | `EPD_RST` | E-paper reset |
| SPI3 BUSY | 6 | `EPD_BUSY` | E-paper busy signal |
| Matrix Row 0 | 1 | `MTX_R0` | Boot window — see §2.2 |
| Matrix Row 1 | 2 | `MTX_R1` | |
| Matrix Row 2 | 42 | `MTX_R2` | Also MTMS (JTAG) — fine for production |
| Matrix Row 3 | 43 | `MTX_R3` | Also MTDI (JTAG) |
| Matrix Row 4 | 25 | `MTX_R4` | Non-strapping. |
| Matrix Col 0 | 8 | `MTX_C0` | |
| Matrix Col 1 | 15 | `MTX_C1` | Non-strapping. Moved from GPIO 3. |
| Matrix Col 2 | 44 | `MTX_C2` | Also MTCK (JTAG) |
| Matrix Col 3 | 9 | `MTX_C3` | |
| Matrix Col 4 | 24 | `MTX_C4` | Non-strapping. |
| Encoder 1 A | 47 | `ENC1_A` | Bank switching — interrupt driven |
| Encoder 1 B | 14 | `ENC1_B` | Bank switching |
| Encoder 2 A | 22 | `ENC2_A` | SD Gain — interrupt driven |
| Encoder 2 B | 23 | `ENC2_B` | SD Gain |
| SD Card Detect | 26 | `SD_DETECT` | Active LOW. Falling edge = removal. |
| Status RGB LED | 48 | `STATUS_LED` | Addressable LED (e.g. WS2812). GPIO 48. |

---

## 3.1 GPIO Availability Audit & Expansion Options

### Available Module GPIO Range
The ESP32-S3-WROOM-1-N8R8 exposes GPIOs **0–26** and **38–48** on its castellated pads.
- **Internal / Forbidden:** GPIO 27–32 (flash), GPIO 33–37 (octal PSRAM) — do not connect.

### Free GPIOs After Current Assignments

| GPIO | Strapping? | Recommended Use |
|---|---|---|
| **GPIO 15** | No | **MTX_C1 relocated here from GPIO 3** (eliminates strapping risk) |
| **GPIO 24** | No | **MTX_C4** (Matrix 5th Column) |
| **GPIO 25** | No | **MTX_R4** (Matrix 5th Row) |
| **GPIO 46** | Yes (ROM msgs) | Last-resort reserve only. Usable post-boot, but avoid if possible. |

### Strapping Pin Recommendation
- **GPIO 0 (MCLK):** Keep on GPIO 0. It is CLK_OUT1, which provides the lowest-jitter 12.288 MHz clock for the ES8388. Routing MCLK through a generic GPIO via the internal GPIO matrix introduces measurable jitter — an unacceptable trade-off for an audio codec. The 10kΩ pull-up handles the boot concern adequately.
- **GPIO 3:** Now **FREE**. Relocated MTX_C1 to **GPIO 15** to eliminate strapping window concerns.
- **GPIO 45:** Tied LOW — correctly handled as a circuit tie, not a functional IO.

### Matrix Configuration (5×5)
The matrix has been expanded to 5×5 (25 buttons) to support future growth. All matrix lines are now assigned and accounted for in Section 3.

| Configuration | Pins Used | Matrix Size | Button Count | Status |
|---|---|---|---|---|
| **Current Design** | GPIO 24 + 25 | 5×5 | 25 | **ACTIVE** |

> **Note:** Even if you do not physically populate all 25 buttons on your first prototype, the 5×5 routing should be implemented in KiCad to allow for "drop-in" physical expansion later.

---

## 4. Power & Supply Rails

### 4.1 Rail Definitions
| Rail | Voltage | Notes |
|---|---|---|
| `3.3V_D` | 3.3V | Digital rail: ESP32-S3, codec DVDD/PVDD/HPVDD |
| `3.3V_A` | 3.3V (filtered) | Analog-only: ES8388 AVDD/AGND. Isolated via ferrite bead. |
| `VBUS` | 5V | From USB-C, through PTC fuse first. |

### 4.2 LDO
- **Part:** SGM2019-3.3 (LCSC C16193), 300mA output.
- Input: VBUS (after fuse). Output: `3.3V_D`.
- Decoupling: 10µF on output, 100nF on both input and output.

### 4.3 Analog/Digital Power Separation (ES8388)
| ES8388 Pin | Rail | Required Decoupling |
|---|---|---|
| DVDD | 3.3V_D | 100nF local |
| PVDD | 3.3V_D | 100nF local |
| HPVDD | 3.3V_D | 10µF + 100nF locally |
| AVDD | 3.3V_A | 10µF + 100nF on the filtered side of the ferrite bead |

- **Ferrite Bead:** BLM18 series, 0402. Between `3.3V_D` and `3.3V_A`.
- **Analog Ground Island:** ES8388 AGND on an isolated copper pour. Connect to main GND via a star-ground point (0Ω resistor) near the LDO GND pin.

### 4.4 I2C Pull-Ups
- **4.7kΩ** on both SDA (GPIO 4) and SCL (GPIO 5) to `3.3V_D`.

---

## 5. USB Section

### 5.1 Connector
- USB-C connector — mid-mount or right-angle top-mount (e.g. LCSC C165948).
- VBUS → PTC fuse → LDO input and downstream protection.

### 5.2 ESD Protection — USBLC6-2SC6 (LCSC C7519)
- **Placement:** Immediately after USB-C connector pads, **before** the PTC fuse.
- SOT-23-6 package. Within **5mm** of USB-C D+/D− pins. This is a hard placement rule.
- Rated 8kV contact discharge (IEC 61000-4-2).

### 5.3 PTC Fuse — MF-MSMF050-2 (LCSC C17313)
- 500mA hold / 1A trip, 1812 package.
- In series with VBUS, **after** USBLC6-2SC6, before LDO.

### 5.4 Auto-Reset Circuit
- Two S8050 NPN transistors on CH340/UART RTS and DTR lines.
- Standard esptool-compatible wiring for automatic bootloader entry.

---

## 6. Audio Subsystem — ES8388 (LCSC C365736)

### 6.1 General
- QFN-28, 4×4mm. LCC land pattern — double-check thermal pad in KiCad footprint.
- I2C address: **0x10** (CE pin LOW) or **0x11** (CE pin HIGH).
  - **Set CE pin state in schematic** (tie to GND or 3.3V_D) and make sure it matches `board_config.h` in firmware.
- I2S slave mode. Signals: MCLK (GPIO 0), BCLK (38), LRCK (39), DOUT (40), DIN (41).

### 6.2 TRRS Jack Signal Path
- **Jack:** PJ-320A (or equivalent), 3.5mm board-mount, CTIA wiring.
- **Output path (Tip = L, Ring 1 = R):** ES8388 LOUT1/ROUT1 → 1µF DC-blocking cap → TVS (PRTR5V0U2X) → TRRS pin.
- **Input path (Ring 2 = Mic):** TRRS Ring 2 → TVS → 2.2kΩ bias resistor from MICBIAS → ES8388 LIN1.
- **Sleeve:** AGND.

### 6.3 TRRS Protection — PRTR5V0U2X,215 (LCSC C12333)
- SOT-143, dual-channel TVS from Nexperia.
- **2 packages required** (covers 3 active lines + 1 spare).
- Placed **between the TRRS jack pads and the first passive** (DC-blocking cap or bias resistor) on each signal line.

---

## 7. MicroSD Section

### 7.1 Slot
- Must have a **card-detect switch pin**. Wire CD switch to GPIO 26 (`SD_DETECT`). Active LOW.
- SPI mode: CS=10, MOSI=11, SCK=12, MISO=13.

### 7.2 Decoupling
- 100nF on VDD of SD slot. Pull MOSI and SCK low or float properly per Espressif SPI SD docs if slot has no pull-ups.

---

## 8. Protection Circuitry — Matrix & Encoders

### 8.1 Series Resistors (100Ω, 0402)
All GPIO lines below **must have a 100Ω series resistor** on every line before the connector/button:

| Lines | Count |
|---|---|
| Matrix Rows (GPIO 1, 2, 42, 43, 25) | 5 × 100Ω |
| Matrix Cols (GPIO 8, 15, 44, 9, 24) | 5 × 100Ω |
| Encoder 1 phases (GPIO 47, 14) | 2 × 100Ω |
| Encoder 2 phases (GPIO 22, 23) | 2 × 100Ω |

**Total: 14 × 100Ω 0402 resistors** for ESD protection on user-accessible inputs.

### 8.2 Encoder Switches (Part of Matrix)
- Encoder 1 Switch → Matrix Row 3, Col 2 (GPIO 42 / GPIO 9)
- Encoder 2 Switch → Matrix Row 3, Col 3 (GPIO 42 / GPIO 9)
- *(Encoder **rotation** phases are interrupt-driven, separate from matrix.)*

---

## 9. Reset & Boot Logic

| Component | Spec |
|---|---|
| EN (Reset) pin | 10kΩ pull-up to 3.3V + 100nF cap to GND. Tactile button to GND. |
| GPIO 0 (Boot) | 10kΩ pull-up to 3.3V + 100nF cap to GND. Tactile button to GND. |
| Auto-reset | RTS/DTR via dual S8050 NPN transistors |

---

## 10. Status Indicators

| Indicator | Implementation |
|---|---|
| Power LED | Red LED on `3.3V_D` via 1kΩ series resistor |
| Activity/Status | Addressable RGB LED on GPIO 48 (e.g. WS2812). |

---

## 11. Two-Board Interconnect (IDC Headers)

- Two **14-pin IDC ribbon headers** (2.54mm pitch). Total of **28 pins** (23 signals + 5 Ground/Shield).
- **Ribbon Isolation Task:** Use the additional pins to interleave **Ground (GND)** between high-speed signals (`SPI_CLK`) and sensitive inputs (`SHIFT_KEY`).
- Signals crossing the interconnect: SPI3 (E-paper), Matrix (5×5), Encoders (4), RGB LED Data (1), 3.3V, and GND.
- Recommend placing decoupling caps close to IDC pin-outs on both boards for any VCC lines carried over.

---

## 12. Board Layout Zones (Brain PCB)

| Zone | Location | Contents |
|---|---|---|
| **Zone A** | Rear | USB-C connector, USBLC6-2SC6 (within 5mm of D+/D−), PTC fuse, Power Switch, LDO |
| **Zone B** | Center | ESP32-S3-WROOM-1, two 14-pin (2×7) IDC headers |
| **Zone C** | Front | ES8388, TRRS jack, PRTR5V0U2X TVS devices |

> The ESD devices should be placed so signals do not run any appreciable length on the PCB before reaching them.

---

## 13. Debug / Programming

- All debug output via **USB CDC** (no UART0 in production).
- Expose physical pads for EN and Boot tactile buttons on the Brain PCB.
- Auto-reset circuit (RTS/DTR) is preferred for developer workflow.

---

## 14. Quick Checklist Before Routing

- [ ] GPIO 33–37 not broken out to any schematic net
- [ ] GPIO 45 (VDD_SPI) tied LOW (10kΩ to GND or direct)
- [ ] GPIO 0 has 10kΩ pull-up + boot button to GND
- [ ] ES8388 CE pin state defined and matches `board_config.h`
- [ ] USBLC6-2SC6 placed within 5mm of USB-C D+/D− before PTC fuse in circuit
- [ ] PRTR5V0U2X placed between TRRS jack pads and first passive on each signal line
- [ ] Ferrite bead isolates `3.3V_D` from `3.3V_A`; star-ground 0Ω near LDO
- [ ] ES8388 AGND on isolated copper island
- [ ] 4.7kΩ pull-ups on SDA (GPIO 4) and SCL (GPIO 5)
- [ ] 100Ω series resistors on all 10 matrix lines and all 4 encoder phase lines
- [ ] MicroSD slot has card-detect pin wired to GPIO 26
- [ ] SPI3 CS on GPIO 21 (non-strapping pin — correct, do not move)
- [ ] SHIFT_KEY and SPI_CLK separated on IDC header pin assignments
- [ ] Power LED (red) on `3.3V_D` via 1kΩ
- [ ] RGB Status LED on GPIO 48
- [ ] EN reset + boot tactile buttons accessible (with caps to GND)
- [ ] S8050 auto-reset transistors wired to RTS/DTR

---

## 15. BOM Quick Reference (LCSC Part Numbers)

### Primary ICs

| Component | Part | LCSC |
|---|---|---|
| MCU | ESP32-S3-WROOM-1-N8R8 | C2913148 |
| Audio Codec | ES8388 (QFN-28) | C365736 |
| LDO | SGM2019-3.3 | C16193 |
| USB ESD | USBLC6-2SC6 (SOT-23-6) | C7519 |
| TRRS TVS | PRTR5V0U2X,215 (SOT-143) | C12333 |
| PTC Fuse | MF-MSMF050-2 (1812) | C17313 |
| USB-C Connector | (mid-mount / RA) | C165948 |

### Board-to-Board Interconnect

| Component | Spec | Qty | Notes |
|---|---|---|---|
| IDC Box Header (PCB) | 2×7, 2.54mm pitch, shrouded, through-hole | 4 | 2 per board (Brain + Plate). |
| IDC Crimp Socket (Cable) | 2×7, 2.54mm pitch, IDC crimp-on female | 4 | 2 per cable × 2 cables. Matched to above header. |
| Flat Ribbon Cable | 14-conductor, 28AWG, 1.27mm pitch | 2 | Wire count must match connector (14-way). |

> **Sourcing tip:** Pre-assembled ribbon cable kits (header + socket + cable) are sold in sets on LCSC/AliExpress. Search "2×5 IDC ribbon cable assembly" or use LCSC's ribbon cable category. Verify the pitch (1.27mm flat ribbon for 2.54mm connectors is correct for standard IDC).

### Passives
- 100nF / 0402 — bypass on all IC supply pins
- 1µF / 0402 — DC blocking on TRRS audio outputs
- 10µF / 0805 — bulk decoupling (3.3V_A, HPVDD, LDO output)
- 100Ω / 0402 — series ESD (matrix rows, cols, encoder phases) × 14
- 1kΩ / 0402 — power LED
- 2.2kΩ / 0402 — MICBIAS to TRRS Ring 2 (mic bias)
- 4.7kΩ / 0402 — I2C SDA/SCL pull-ups × 2
- 10kΩ / 0402 — EN, Boot, strapping pull-ups × (at minimum 3)
- 0Ω / 0402 — star-ground jumper (AGND island to main GND)
- BLM18 series / 0402 — ferrite bead (3.3V_D → 3.3V_A)
- S8050 NPN × 2 — auto-reset transistors
