# Stinger
## USB Audio Soundboard — Overview & Master Specification
**Revision 1.8 (Architectural Alignment) – Split Edition**
**Project:** Stinger | **Manufacturer:** JP+

> This document serves as the top-level project overview and the **Master Contract** between the Hardware and Firmware domains. For detailed subsystem specifications, refer to the linked documents below.

---

## 1. Project Specifications

The specification has been cleanly divided into hardware and firmware domains for ease of development:

- **[Hardware Specification](hardware/HARDWARE_SPEC.md)**: Schematics, layout guidelines, BOM, and protection circuitry.
- **[Firmware Specification](firmware/stinger/FIRMWARE_SPEC.md)**: RTOS tasks, display layouts, sound engine concurrency, and architecture guidelines.

---

## 2. Architecture: The "Long-Strap" Console

The Stinger uses a modular Two-Board Hinge design to optimize signal integrity and mechanical stability.

- **Brain PCB (Base):** Houses the ESP32-S3, ES8388 Codec, MicroSD slot, Power Section, and Rear I/O. Uses **Professional SMT** for fine-pitch ICs and **Manual THT** for IDC interconnects.
- **Plate PCB (Interface):** Houses the 25-button matrix and the 2.9" E-paper display. Fully **Hand-Soldered**.
- **Interconnect:** Two standard 14-pin IDC ribbon cables (2.54mm pitch) connect the boards via THT shrouded headers.

---

## 3. Master Pin Mapping & Matrix (The Contract)

**Note on Single Source of Truth:** This section is the *absolute contract* between the hardware design and the firmware implementation. Neither domain dictates the pins independently. 
- **Hardware:** Routes the schematics matching this table.
- **Firmware:** Reads from this table to define `firmware/main/pin_map.h`, which is the *code's* source of truth.

### 3.1 Pin Alignment Automation
To prevent "split-brain" discrepancies between PCB schematics and firmware constants, this project employs a centralized code generation strategy.
- **Source of Truth:** The pins are exclusively defined in `firmware/stinger/master_pins.json`.
- **Developer Workflow:** Setting your IDE workspace to `firmware/stinger/` is fully supported and recommended. You edit the JSON file directly. 
- **Under the Hood:** During CMake generation (`idf.py build` or `ninja`), a Python script (`tools/generate_pins.py`) is automatically executed. It builds `pin_map.h` for the firmware compiler and dynamically updates the Markdown table below. The script resolves paths natively relative to itself, meaning you can trigger builds and code-generation from any valid workspace context or terminal folder without filepath errors.

### 3.2 Pin Assignment Table (Rev V37)

<!-- PINS_START -->
<!-- The table below is auto-generated from firmware/stinger/master_pins.yaml during build. Do not edit. -->
| Pin Name | GPIO | Description/Note |
|---|---|---|
| `PIN_USB_D_MINUS` | GPIO 19 | USB D- |
| `PIN_USB_D_PLUS` | GPIO 20 | USB D+ |
| `PIN_I2S_BCLK` | GPIO 38 | I2S BCLK (Audio Data) |
| `PIN_I2S_LRCK` | GPIO 39 | I2S LRCK (Audio Data) |
| `PIN_I2S_DOUT` | GPIO 40 | I2S DOUT (Audio Data) |
| `PIN_I2S_DIN` | GPIO 41 | I2S DIN (Audio Data) |
| `PIN_I2S_MCLK` | GPIO 0 | I2S MCLK (CLK_OUT1. 256xFs = 12.288 MHz) |
| `PIN_I2C_SDA` | GPIO 4 | I2C SDA (Control) |
| `PIN_I2C_SCL` | GPIO 5 | I2C SCL (Control) |
| `PIN_SPI2_CS` | GPIO 10 | SPI2 CS (MicroSD) |
| `PIN_SPI2_MOSI` | GPIO 11 | SPI2 MOSI (MicroSD) |
| `PIN_SPI2_SCK` | GPIO 12 | SPI2 SCK (MicroSD) |
| `PIN_SPI2_MISO` | GPIO 13 | SPI2 MISO (MicroSD) |
| `PIN_SPI3_CS` | GPIO 21 | SPI3 CS (E-Paper) on non-strapping pin |
| `PIN_SPI3_DIN` | GPIO 16 | SPI3 DIN (E-Paper) |
| `PIN_SPI3_CLK` | GPIO 17 | SPI3 CLK (E-Paper) |
| `PIN_SPI3_DC` | GPIO 18 | SPI3 DC (E-Paper) |
| `PIN_SPI3_RST` | GPIO 7 | SPI3 RST (E-Paper) |
| `PIN_SPI3_BUSY` | GPIO 6 | SPI3 BUSY (E-Paper) |
| `PIN_MATRIX_ROW_0` | GPIO 1 | Matrix Row 0 |
| `PIN_MATRIX_ROW_1` | GPIO 2 | Matrix Row 1 |
| `PIN_MATRIX_ROW_2` | GPIO 42 | Matrix Row 2 |
| `PIN_MATRIX_ROW_3` | GPIO 43 | Matrix Row 3 |
| `PIN_MATRIX_ROW_4` | GPIO 25 | Matrix Row 4 |
| `PIN_MATRIX_COL_0` | GPIO 8 | Matrix Col 0 |
| `PIN_MATRIX_COL_1` | GPIO 15 | Matrix Col 1 (moved from GPIO 3 strapping pin) |
| `PIN_MATRIX_COL_2` | GPIO 44 | Matrix Col 2 |
| `PIN_MATRIX_COL_3` | GPIO 9 | Matrix Col 3 |
| `PIN_MATRIX_COL_4` | GPIO 24 | Matrix Col 4 |
| `PIN_ENC1_A` | GPIO 47 | Encoder 1 A (Bank switching) |
| `PIN_ENC1_B` | GPIO 14 | Encoder 1 B (Bank switching) |
| `PIN_ENC2_A` | GPIO 22 | Encoder 2 A (SD Gain) |
| `PIN_ENC2_B` | GPIO 23 | Encoder 2 B (SD Gain) |
| `PIN_SD_DETECT` | GPIO 26 | SD Card Detect (Active low) |
| `PIN_STATUS_LED` | GPIO 48 | Status RGB LED |
<!-- PINS_END -->

### 3.2 Logical Matrix Map (5×5)

| Row \ Col       | Col 0 (GPIO 8) | Col 1 (GPIO 15) | Col 2 (GPIO 44) | Col 3 (GPIO 9) | Col 4 (GPIO 24) |
|-----------------|----------------|-----------------|-----------------|----------------|-----------------|
| Row 0 (GPIO 1)  | Trigger L1     | Trigger L2      | Trigger L3      | Trigger L4     | Reserved        |
| Row 1 (GPIO 2)  | Trigger R1     | Trigger R2      | Trigger R3      | Trigger R4     | Reserved        |
| Row 2 (GPIO 42) | Fav 1          | Fav 2           | Fav 3           | Fav 4          | Reserved        |
| Row 3 (GPIO 43) | Fav 5          | Shift Key       | Enc 1 Switch    | Enc 2 Switch   | Reserved        |
| Row 4 (GPIO 25) | Reserved       | Reserved        | Reserved        | Reserved       | Reserved        |

---

## 4. Repository Layout

```
stinger/
│
├── SPEC.md                        # This document (Project Overview & Master Pin Contract)
├── hardware/
│   ├── HARDWARE_SPEC.md           # Master hardware requirements
│   ├── brain/ ...
│   ├── plate/ ...
│   └── fabrication/ ...
│
├── firmware/
│   ├── stinger/
│   │   ├── master_pins.json       # Source of Truth for Pins
│   │   ├── FIRMWARE_SPEC.md       # Master firmware requirements
│   │   ├── main/
│   │   │   ├── pin_map.h          # Auto-generated code implementation
│   │   │   ├── board_config.h
│   │   │   └── stinger_app.c
│   │   ├── components/ ...
│   │   └── tools/
│   │       └── generate_pins.py   # Generator script
└── assets/ ...
```

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
| 1.6 | Full protection circuitry restored. SD card-detect added (GPIO 26). BOM rationalised. |
| 1.7 | Project renamed to **Stinger** by JP+. USB descriptor strings defined. Boot splash screen specified. |
| 1.8 | Formalized 3-Layer Architectural model. Document split into Hardware/Firmware specs, leaving Master Pin Contract in root `SPEC.md`. |
| 1.9 | Updated framework requirement to ESP-IDF v6.0 to align with current IDE extension defaults. |
