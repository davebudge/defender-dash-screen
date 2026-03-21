# Defender Dashboard Screen

Central dashboard display for the Land Rover Defender Puma EV conversion. Runs on a **Waveshare ESP32-S3-Touch-LCD-4.3B-BOX** (800x480 IPS, portrait orientation).

## What It Does

Displays a grid of ~23 warning light icons with a central gear selector (P/R/N/D/S). Warning lights are driven by CAN bus messages from the VCU and BMS.

## Hardware

| Component | Details |
|-----------|---------|
| Display | Waveshare ESP32-S3-Touch-LCD-4.3B-BOX (SKU 28141) |
| MCU | ESP32-S3 (240MHz, 16MB Flash, 8MB PSRAM) |
| Screen | 4.3" IPS, 800x480, capacitive touch (GT911) |
| CAN Bus | TWAI at 500 kbps (TX=GPIO15, RX=GPIO16) |
| Power | 12V via terminal block |

## Tech Stack

- **ESP-IDF 5.2+** (Espressif IoT Development Framework)
- **LVGL 8.3.x** (hand-coded, not SquareLine Studio)
- **FreeRTOS** (CAN on Core 0, LVGL on Core 1)

## Architecture

```
CAN Bus (TWAI) -> can_bus.c RX Task (Core 0)
    -> Dispatch by msg_id to registered handlers
    -> vehicle_state.c updates state (mutex-protected)
    -> ui_main.c LVGL timer (100ms, Core 1) polls state
    -> ui_warning_grid / ui_gear_display update widgets
```

**Data-driven icon system:** All icon configuration lives in a single table (`main/icons/icon_defs.c`). Adding a new warning light requires only:
1. Add an image file (`main/icons/img_xxx.c`)
2. Add one row to the `icon_defs[]` table
3. Add `LV_IMG_DECLARE` in `icons.h`

No other code changes needed.

## Project Structure

```
main/
  main.c                     # App entry point
  lvgl_port.c/h              # LVGL display driver (with 90deg rotation)
  waveshare_rgb_lcd_port.c/h # LCD hardware init
  can/
    can_bus.c/h              # CAN RX task + message dispatch
  ui/
    ui_main.c/h              # Screen layout, update timer
    ui_warning_grid.c/h      # Data-driven icon grid
    ui_gear_display.c/h      # Central gear letter (Eurostile font)
    ui_styles.c/h            # Shared colors and styles
  data/
    vehicle_state.c/h        # Thread-safe CAN-to-UI bridge
  icons/
    icon_defs.c/h            # Master icon configuration table
    icons.h                  # Image declarations
    img_*.c                  # LVGL image C arrays (one per icon)
  fonts/
    font_eurostile_96.c      # Eurostile Extended #2 Bold (P/R/N/D/S)
lib/
  CH422G/                    # GPIO expander driver
  Config/                    # I2C master init
```

## Building

### Prerequisites

- [ESP-IDF v5.2+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)

### Build & Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## CAN Bus Mapping

CAN message IDs are configured in `main/icons/icon_defs.c`. Each icon has three CAN fields:

| Field | Description |
|-------|-------------|
| `can_msg_id` | CAN message identifier (0 = not mapped) |
| `can_byte` | Byte index in the CAN data (0-7) |
| `can_bit_mask` | Bit mask within that byte |

All mappings are currently placeholder (`0x000`). Update them once the VCU/BMS CAN specs are finalised.

## Adding a New Warning Light

1. Export the icon from Figma as a white-on-transparent PNG
2. Convert to LVGL C array:
   ```bash
   python3 scripts/convert_icon.py icon_name.png
   ```
   Or use the [LVGL Online Image Converter](https://lvgl.io/tools/imageconverter) (CF_TRUE_COLOR_ALPHA, C array)
3. Save the `.c` file as `main/icons/img_youricon.c`
4. Add `LV_IMG_DECLARE(img_youricon);` to `main/icons/icons.h`
5. Add enum entry to `icon_id_t` in `main/icons/icon_defs.h`
6. Add a row to `icon_defs[]` in `main/icons/icon_defs.c`

## Design

[Figma Design](https://www.figma.com/design/iSKJzCdIgV8DlrL4khMYkA/Defender-Puma-Central-Screen?node-id=9-378)

## Team

Built by [Jaunt Motors](https://jauntmotors.com) for the Defender Puma EV conversion project.
