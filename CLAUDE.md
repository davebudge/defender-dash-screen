# Defender Dashboard Screen

## Project Overview

Dashboard display software for the **Defender Puma EV conversion** by **Jaunt Motors**.
Runs on a **Waveshare ESP32-S3-Touch-LCD-4.3B-BOX** (800x480 display, used in portrait 480x800).
Shows ~22 warning light icons in a grid layout with a central gear selector (P/R/N/D/S).

**Owner:** Dave Budge, Jaunt Motors (embedded systems / hardware background)
**GitHub:** https://github.com/davebudge/defender-dash-screen

## Notion

The build log and documentation lives on the Jaunt Notion workspace:

- **Build Log:** "Puma Dash by Claude" — page ID `32a02a4300898070bc52e9a5eee6568c`
  - URL: https://www.notion.so/jaunt/Puma-Dash-by-Claude-32a02a4300898070bc52e9a5eee6568c
  - This is the page to update with development progress notes
- **Display Requirements:** "Defender Instrument Cluster Display Requirements [KARPIEL]" — page ID `1bc02a43008980d98543f46e611fe0af`
  - Contains Fellten dual-pack CAN specs, DBC info, fault codes
- **Parent Task:** "ESP32 S3 Dash Screen Investigation DEF23" — page ID `27302a4300898010a646f20f303e9e9d`
- **Project:** "LR Defender: PUMA 2012 - 2016" — page ID `1a702a43008980d98543f46e611fe0af`

## Figma Design

https://www.figma.com/design/iSKJzCdIgV8DlrL4khMYkA/Defender-Puma-Central-Screen?node-id=9-378

## Tech Stack

- **Framework:** ESP-IDF 5.2+ (matches existing Jaunt team code)
- **UI Library:** LVGL 8.3.x (hand-coded, NOT SquareLine Studio)
- **CAN Bus:** TWAI at 500 kbps (TX=GPIO15, RX=GPIO16)
- **Font:** Eurostile Extended #2 Bold (converted to LVGL C array for gear letters)
- **Simulator:** `simulator/index.html` — run with `python3 -m http.server 8090 -d simulator`

## Architecture

### Data-driven icon system
All icon config (image, color, grid position, CAN mapping, ADR self-test flag) lives in a single table in `main/icons/icon_defs.c`. Adding a new warning light = add one row + one image file. No other code changes needed.

### CAN-to-UI pipeline
1. CAN RX task on Core 0 receives messages via TWAI
2. Dispatches by message ID to registered handlers
3. `vehicle_state` module updates state (FreeRTOS mutex-protected)
4. LVGL timer (100ms, Core 1) polls state and updates UI

### Key Modules

| Module | Location | Purpose |
|--------|----------|---------|
| CAN bus | `main/can/can_bus.c` | CAN RX task + message dispatch |
| Vehicle state | `main/data/vehicle_state.c` | Thread-safe CAN-to-UI bridge, key barrel state machine, ADR self-test |
| Warning grid | `main/ui/ui_warning_grid.c` | Data-driven icon grid layout |
| Gear display | `main/ui/ui_gear_display.c` | Central gear letter display (P/R/N/D/S) |
| Icon definitions | `main/icons/icon_defs.c` | Master icon configuration table |
| Status screen | `main/ui/ui_status_screen.c` | Vehicle telemetry display (swipe right) |
| UI styles | `main/ui/ui_styles.h` | Color constants (SAE J578) |

## Vehicle Operating States (Key Barrel)

The vehicle has four operating states managed via CAN or the key barrel:

| State | Description | Icon Behavior |
|-------|-------------|---------------|
| **OFF** | No power to screen | All off, screen blank |
| **ACC** | Accessory — screen boots | Splash screen → ADR self-test (3s tell-tale illumination) |
| **ON** | Running/Ready | CAN-driven icons ONLY. Self-test cancelled if still active |
| **CHARGE** | Charge cable connected | CAN-driven icons ONLY. Self-test cancelled if still active |

### ADR Self-Test Rules
- Self-test ONLY triggers on **OFF→ACC** transition (cold boot)
- Going directly to ON or CHARGE does NOT trigger self-test
- Moving from ACC to ON/CHARGE cancels any in-progress self-test
- Battery pack icons (primary/secondary) and balance arrows are NOT part of self-test
- Self-test duration: 3 seconds per ADR requirements
- After self-test, icons turn off with a staggered waterfall animation

### Icons included in ADR self-test
`HVIL`, `Seatbelt`, `Parking Brake`, `Brake Warning`, `ABS`, `Engine`, `12V Battery`, `Traction Control`

## Icon Colors (SAE J578)

**IMPORTANT:** Use bright, vivid colors. Dave has specifically requested full-brightness colors — NOT muted/dim versions.

| Color | ESP32 (LVGL) | Simulator SVG Filter | Notes |
|-------|-------------|---------------------|-------|
| Red | `0xFF0000` | Flat `1,0,0` matrix | SAE J578 signal red |
| Amber | `0xFF8C00` | `1, 0.647, 0` matrix | SAE J578 amber — it's ORANGE, not yellow |
| Blue | `0x0066FF` | `0, 0.533, 1` matrix | SAE J578 signal blue |
| Green | `0x00CC00` | `0, 1, 0` matrix | SAE J578 signal green |

### Color tinting approach
- ESP32: LVGL `lv_img_set_style_img_recolor()` on white PNG icons
- Simulator: SVG `<feColorMatrix>` filters applied via `data-color` attribute on `<img>` tags
- CSS `mask-image` does NOT work in Safari — do not use it

## Simulator

The web simulator (`simulator/index.html`) mirrors the ESP32 dashboard in a browser:

- **Key barrel emulator:** OFF/ACC/ON/CHARGE buttons control the boot sequence and state
- **OFF state:** Screen goes completely black (visibility hidden)
- **OFF→ACC:** Shows Jaunt logo splash (1.5s), then ADR self-test (3s)
- **Battery balancing arrows:** Animated CSS chevrons between the two HV pack icons. Right row animates at 1.8s, left at 2.3s — different durations create organic, non-mechanical feel
- **Presets:** All On, All Off, Drive Mode, Charge Mode, Fault Mode
- **Icon exports:** White-on-transparent PNGs from Figma at @2x, displayed at half size

## CAN Bus (Fellten Dual-Pack System)

CAN message 0x36A contains most warning flags. Key signals from Shane/Fellten:

- **Conditioning Packs:** Packs shutting down with current flowing, or unbalanced but coming online
- **Link HV Present:** 0 = only one pack online
- **Link CC Fault:** Contactor controller fault in link pack
- **Link CC Present:** Comms confirmation between packs (0 = no comms)

### Master Pack Faults
HVIL Broken, BMS Comms errors, BMS DTC errors, Precharge Fault, Lock Motor Fault, DC Temp sensors out of range, Persistent fault

### Link Pack Faults
Comms error, BMS Comms errors, BMS DTC errors, Precharge Fault

## Build Notes

- Icons are Figma exports at @2x resolution — display at half size in both ESP32 and simulator
- Gear letter P is RED, all other gears (R/N/D/S) are WHITE
- The Waveshare display hardware drivers are reused from an existing Jaunt prototype
- sdkconfig is cleaned — unused demo apps removed, watchdog timeout configured
