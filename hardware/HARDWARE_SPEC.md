# Stinger: Hardware Specification
**Revision 1.9 — Hybrid Assembly Model**

> [!IMPORTANT]
> **Navigation & Source of Truth**  
> *   **Pin Mapping:** Refer to the root [SPEC.md](file:///Users/jonathanpool/Projects/Stinger/SPEC.md) (Source of Truth).
> *   **Detailed Design Notes:** Refer to [KICAD_DESIGN_NOTES.md](file:///Users/jonathanpool/Projects/Stinger/hardware/KICAD_DESIGN_NOTES.md) for layout rules.
> *   **BOM & Sourcing:** The table in **Section 5** of *this document* is the absolute Source of Truth for parts.

---

## 1. Assembly Strategy

The Stinger uses a **Hybrid Assembly Model** to balance complexity and ease of construction:

| Board | Assembly Method | Description |
|---|---|---|
| **Brain PCB** | **Mixed Mode** | **Professional SMT** for fine-pitch ICs/passives. **Manual THT** for IDC headers. |
| **Plate PCB** | **Hand-Soldered** | Contains buttons, diodes, and through-hole headers. Uses larger pads (0805 or THT) for manual assembly. |

---

## 2. Hardware Specification

### 2.1 MCU Module
- **Model:** ESP32-S3-WROOM-1-N8R8.
- **Strapping:** GPIO 0 HIGH (10kΩ); GPIO 45 LOW (10kΩ).

### 2.2 Audio Subsystem
- **Codec:** ES8388 (QFN-28). 48kHz / 16-bit PCM.
- **I2C Address:** 0x10 (CE pin LOW).

### 2.3 TRRS signal Path
- **Protection:** PRTR5V0U2X TVS devices on all active lines. 1µF DC-blocking caps on L/R outputs.

### 2.4 Matrix & Input Protection
- **Series Resistors:** **100Ω (0402)** on all matrix lines and encoder phases. **Located on Brain PCB** for professional placement.
- **Ghosting Protection:** **25× 1N4148 diodes** (SOD-123 or THT). **Located on Plate PCB** at each switch node.
- **Orientation:** Cathode to Matrix Row (for active-low scan).

### 2.5 E-Paper Display Module
- **Model:** 2.9" E-Paper Module (e.g., Waveshare).
- **Interconnect:** **8-pin through-hole header (2.54mm)** on Plate PCB. Hand-wired to the display module.

### 2.6 SD Card & Power
- **SD Slot:** Includes card-detect switch on GPIO 26.
- **Auto-Reset:** Dual S8050 NPN circuit for RTS/DTR.

---

## 3. Board Layout Strategy

- **Brain (SMT Zone):** Concentrated fine-pitch routing for audio/USB. IDC headers are THT and should be soldered manually after SMT assembly to save on fixture fees.
- **Plate (Manual Zone):** Spacious layout for MX-style keys, 0805 resistors (if any), and easily accessible THT pads for the display module and IDC headers.

---

## 4. Maintenance & Change Management

To keep the hardware and software in sync, follow these rules for every change:
1.  **Pins:** Update `firmware/stinger/master_pins.json` first. Run `idf.py build` to propagate changes to `SPEC.md`.
2.  **BOM:** Update Section 5 of this document.
3.  **Footprints:** If switching from SMD to THT on the Plate, update the KiCad project and this spec simultaneously.

---

## 5. Bill of Materials (Master Source of Truth)

### 5.1 Brain PCB — Professional SMT Items (LCSC)

| Component | Part / Value | LCSC | Note |
|---|---|---|---|
| MCU | ESP32-S3-WROOM-1-N8R8 | C2913148 | |
| Audio Codec | ES8388 | C365736 | QFN-28 |
| LDO | SGM2019-3.3 | C16193 | 3.3V, 300mA |
| USB ESD | USBLC6-2SC6 | C7519 | Within 5mm of USB-C |
| TRRS TVS | PRTR5V0U2X,215 | C12333 | Qty: 2 |
| PTC Fuse | MF-MSMF050-2 | C17313 | 1812 package |
| USB-C | RA Top-Mount | C165948 | |
| MicroSD | With Card Detect | C91145 | |
| Series Resists | 100Ω (0402) | | Qty: 14 |
| Passives | 100nF, 1uF, 10uF | | Refer to KiCad Project |

### 5.2 Brain PCB — Hand-Soldered / User-Side Items
*These items are through-hole and should be installed manually to avoid JLCPCB's THT assembly and fixture surcharges.*

| Component | Part / Value | LCSC | Note |
|---|---|---|---|
| IDC Headers | 2×7 Box Header | C2922129 | 2.54mm THT. Placed on top side. |

### 5.3 Plate PCB — Hand-Soldered / Manual Items
*This entire board is intended for manual assembly.*

| Component | Part / Value | Sourcing | Note |
|---|---|---|---|
| Keys | MX-Style or Tactile | Generic | 25 required |
| Diodes | 1N4148W (SOD-123) | C1661 | or THT 1N4148 (C467) |
| IDC Headers | 2×7 Box Header | C2922129 | 2.54mm THT. Placed on bottom side. |
| E-Paper | 2.9" Module | Waveshare | With 8-pin breakout |
| Status LED | WS2812B (5050) | C114586 | 1 required |
| Ribbon Cable | 14-pin IDC Assembly | Generic | Qty: 2 |

