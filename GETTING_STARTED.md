# Getting Started with Stinger Development

This guide will help you initialize your development environment and build the Stinger firmware for the ESP32-S3.

## 📋 Prerequisites (macOS)

You'll need a few system tools. If you don't have Homebrew installed, get it from [brew.sh](https://brew.sh).

```bash
brew install cmake ninja dfu-util python3 git
```

## 🛠️ ESP-IDF v5.3 Framework Setup

The Stinger project requires **ESP-IDF v5.3**. We recommend installing it in a dedicated directory:

1.  **Create the directory:**
    ```bash
    mkdir -p ~/esp/v5.3
    cd ~/esp/v5.3
    ```

2.  **Clone the framework:**
    ```bash
    git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git
    ```

3.  **Install the ESP32-S3 toolchain:**
    ```bash
    cd ~/esp/v5.3/esp-idf
    ./install.sh esp32s3
    ```

## 🏗️ Build & Flash Firmware

1.  **Activate the environment:**
    Open a new terminal and run:
    ```bash
    . ~/esp/v5.3/esp-idf/export.sh
    ```
    *(Tip: You can add this to your `.zshrc` or `.bash_profile` for easier access.)*

2.  **Navigate to the firmware directory:**
    ```bash
    cd firmware
    ```

3.  **Initialize the target:**
    (Only needed once for the initial setup)
    ```bash
    idf.py set-target esp32s3
    ```

4.  **Build the project:**
    ```bash
    idf.py build
    ```

5.  **Flash and Monitor:**
    Plug in your Stinger board via USB and run:
    ```bash
    idf.py flash monitor
    ```

## 📂 Project Structure

- `firmware/` — ESP-IDF v5.3 source code.
    - `main/pin_map.h`: Hardware Rev V37 GPIO assignments.
    - `main/board_config.h`: Master constants (Audio/USB/Display).
- `hardware/` — KiCad 8.0 design files (Brain and Plate boards).
- `tools/` — Helper scripts for WAV preparation and flashing.
- `assets/` — Bitmap fonts and sample audio banks.

## 🔗 Technical Specification

For detailed hardware requirements, pin maps, and firmware logic, refer to the [SPEC.md](SPEC.md).
