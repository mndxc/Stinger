# Stinger Firmware Development Notes

## Input System (Phase 1)
- **Matrix Scanning**: Initial implementation uses a 1ms scan interval with a 10us settling delay between row activation and column reading. If ghosting occurs with larger matrices or longer traces, consider increasing the settling delay or adding software debouncing logic beyond simple state change detection.
- **Encoder ISR**: Current implementation triggers on the falling edge of the A pin and reads the B pin state to determine direction. This is a lightweight approach. If resolution issues or "skipping" is observed during fast rotation, consider moving to a full 4-state quadrature state machine on both edges.
- **Layer 3 Routing**: All input events are funneled through a FreeRTOS queue (`s_app_queue`) to `stinger_app_task`. This decouples the low-level hardware timing from the application logic.

## Build System & Dependencies
- **USB Audio Component**: Temporarily disabled (`usb_audio/CMakeLists.txt.disabled`) due to missing `tinyusb` component dependency in the current environment. This needs to be resolved during Phase 2.
- **Pin Map**: `pin_map.h` is auto-generated. Manual edits will be overwritten. Always update `master_pins.json` (or `.yaml` as referenced in docs) and rebuild.
- **Component Requirements**: ESP-IDF v6.0 requires explicit component requirements in `idf_component_register`. For example, `esp_driver_gpio` must be explicitly listed in `REQUIRES`.

## Future Transitions
- **Transfer Mode**: Logic for Shift + Enc2 Switch hold is planned but not yet implemented. The scan task will provide the raw button events; the timing logic should reside in the Layer 3 state machine.
- **FreeRTOS Task Pinning**: Matrix scanning is pinned to Core 0 (Priority 20). Audio Mixer (Phase 2) should be pinned to Core 1 (Priority 25) to avoid starvation from UI/USB tasks.
