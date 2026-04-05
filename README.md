# Stinger — USB Audio Soundboard

Stinger is a high-performance USB Audio Soundboard powered by the ESP32-S3 and the ES8388 Audio Codec. This repository contains the firmware, hardware design files, and tools for the Stinger project.

## 🚀 Getting Started

To initialize your development environment and build the firmware, follow the step-by-step instructions in:
👉 **[GETTING_STARTED.md](GETTING_STARTED.md)**

## 🛠️ Project Structure

- `firmware/` — ESP-IDF v5.3 source code.
    - `main/pin_map.h`: Hardware Rev V37 GPIO assignments.
    - `main/board_config.h`: Master constants (Audio/USB/Display).
- `hardware/` — KiCad 8.0 design files (Brain and Plate boards).
- `tools/` — Helper scripts for WAV preparation and flashing.
- `assets/` — Bitmap fonts and sample audio banks.

## 🔗 Technical Specification

For detailed hardware requirements, pin maps, and firmware logic, refer to the [SPEC.md](SPEC.md).
