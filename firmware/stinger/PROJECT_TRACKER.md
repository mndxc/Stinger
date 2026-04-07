# Stinger Firmware - AI Project Tracker

This document is the authoritative task-tracking resource for the Stinger USB Audio Soundboard firmware project. It serves as a continuous record of progress, breaking the project down from its current initialized state to the finished product as defined in `FIRMWARE_SPEC.md`.

## AI Instructions & Guidelines

When working on this project in an autonomous or pair-programming session, AI coding assistants must adhere to the following rules for managing this document:

### 1. Adding New Items
* Always add new tasks into the appropriate "Logical Phase" section.
* If a task is too large, break it down as nested bullet points under the main task.
* Format tasks consistently using standard markdown checklist syntax.

### 2. Status Indicators
Use the following strict indicators to denote the state of an item:
* `[ ]` **Unstarted**: Work has not begun.
* `[-]` **In Progress**: Work is currently underway or partially complete.
* `[x]` **Completed**: Implementation is complete, tested, and verified.
* `[!]` **Blocked**: Work cannot proceed until an external dependency or question is resolved.

### 3. Documenting Progress (Partially Complete Items)
* When updating an item to `[-]` (In Progress) or `[!]` (Blocked), you **must** add a sub-bullet starting with `> Update (YYYY-MM-DD):` directly below the task.
* Briefly detail what has been completed, what remains, or the exact nature of a blockage.
* *Example:*
  - [-] Implement SD Card read/write operations
    - > Update (2026-04-06): FAT32 mounting works, but DMA reading is currently throwing an underflow error. Debugging mixer priority.

### 4. Checking Off Items
* When a task is fully resolved, change its status to `[x]`. 
* You may leave the `> Update` progression history intact as an audit trail of how the task was solved, or summarise the final merged resolution.

---

## Current State Summary
* **Environment**: ESP-IDF v6.0 IDE configured.
* **Build System**: CMake configured. Project builds successfully.
* **Hardware Config**: Setup of `master_pins.yaml` and auto-generated `pin_map.h`.
* **Sub-module scaffolding**: `main.c` calls `stinger_app_init()`.

---

## Task Backlog (Logical Steps to Completion)

### Phase 0: Hardware Design Freeze (CRITICAL)
- [-] **Schematic & Pin Audit**: Review `master_pins.json` against ESP32-S3 strapping requirements.
  - > Update (2026-04-06): Hardware checklist created. Focus on GPIO 0 (MCLK) and GPIO 3 (Matrix) conflicts.
- [ ] **BOM Verification**: Confirm ES8388, E-paper, and SD card components meet electrical specs.
- [ ] **Circuit Protection**: Confirm anti-ghosting diodes and I2C pull-ups are in the layout.

### Phase 1: Hardware Abstraction & Drivers (Layer 1)
- [x] Initialize I2C Bus for Codec Communication
- [x] Integrate ES8388 Audio Codec Driver (Local Component)
  - > Update (2026-04-06): I2C Master initialized and ES8388 driver implemented. Verified via successful build and component registration.
- [ ] Initialize SPI3 Bus for Display Comm
- [ ] Integrate Waveshare 2.9" E-paper Display Driver
- [x] Initialize FreeRTOS `InputScanTask` (Core 0, Priority 20) for 4x4 Matrix
- [x] Integrate Rotary Encoders (Interrupt-driven updates)

### Phase 2: Core Subsystem Services (Layer 2)
- [ ] Initialize `SdManagerTask` (Core 1, Priority 15)
  - [ ] Implement FAT32 filesystem mounting via SPI/SDIO
  - [ ] Implement `sd_card_mutex` strategy
  - [ ] Implement Card-Detect GPIO callback (falling edge on GPIO 26)
- [ ] Initialize `UsbStackTask` (Core 0, Priority 22)
  - [ ] Setup TinyUSB descriptors (JP+, Stinger, etc.)
  - [ ] Implement UAC1 Audio interfaces (Capture & Playback)
  - [ ] Implement MSC interface for USB Mass Storage (Transfer Mode)
- [ ] Initialize `AudioMixerTask` (Core 1, Priority 25)
  - [ ] Setup I2S DMA for sending data to ES8388
  - [ ] Implement SD WAV file fetching (pre-load into PSRAM)
  - [ ] Implement 3-stream mono mixing logic 
  - [ ] Implement Mic + SD Audio mixdown for UAC1 Capture

### Phase 3: Application Logic (Layer 3)
- [ ] Scaffold `stinger_app.c` State Machine logic
- [ ] Implement Bank Switching logic via Encoder 1
  - [ ] Validate `/bankXX/` folder format and boundary wrapping (01–99)
- [ ] Implement Sub-Mixer gain scaling logic via Encoder 2
- [ ] Implement sample trigger/retrigger logic on Keypress (L1-L4, R1-R4)

### Phase 4: UI & Display Rendering
- [ ] Initialize `DisplayTask` (Core 0, Priority 2)
- [ ] Implement Boot Splash Screen logic (STINGER JP+)
  - [ ] Enforce 2-second timeout and SD-Card-absent fallback.
- [ ] Implement Main Layout Rendering
  - [ ] Zone 1: Bank Indicator logic
  - [ ] Zone 2: Button Labels (parsing `labels.cfg` or filename)
  - [ ] Zone 3: Status bar (Volume, Gain, Connection indicators)
- [ ] Implement dynamic display refresh on Bank Switch

### Phase 5: Concurrency & Transfer Mode
- [ ] Implement Transfer Mode Entry/Exit sequence (Shift + Enc2 Switch)
  - [ ] Drain DMA buffers and suspend `AudioMixerTask`
  - [ ] Release SD Mutex and unmount FAT32
  - [ ] Hand over SD control to USB MSC and update display to "TRANSFER MODE"
- [ ] Implement USB Disconnect state recovery logic
- [ ] Perform Audio stress testing (simultaneous sample overlaps)
- [ ] Final bug-fixing and memory-leak verification
