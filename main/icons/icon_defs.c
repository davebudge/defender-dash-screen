/**
 * @file icon_defs.c
 * @brief Master icon definitions table
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │  HOW TO ADD A NEW WARNING LIGHT:                            │
 * │                                                             │
 * │  1. Export icon from Figma as white-on-transparent PNG      │
 * │  2. Convert to C array using LVGL image converter           │
 * │  3. Save as main/icons/img_xxx.c                            │
 * │  4. Add LV_IMG_DECLARE(img_xxx) to icons.h                  │
 * │  5. Add enum value to icon_id_t in icon_defs.h              │
 * │  6. Add one row to icon_defs[] below                        │
 * │                                                             │
 * │  HOW TO MAP A CAN MESSAGE:                                  │
 * │  Set can_msg_id, can_byte, and can_bit_mask in the row.     │
 * │  The vehicle_state module auto-registers handlers.          │
 * └──────────────────────────────────────────────────────────────┘
 *
 * Grid layout (matches Figma design, 480x800 portrait):
 *
 *   Row 0: HVIL          [empty]       [empty]       Cup/Heat
 *   Row 1: PBrake Slash  Seatbelt      Park Light    Hazard
 *   Row 2: PBrake        EV Charger    High Beam     Brake Warn
 *          ──────────── GEAR DISPLAY ────────────
 *   Row 3: ABS           Engine        Battery 12V   Coolant
 *   Row 4: Traction Ctrl Thermo Hot    Thermo Cold   Book/Wrench
 *   Row 5: HV Batt Warn  HV Batt Warn  [empty]      [empty]
 *   Row 6: Charge Left   [arrow]       Charge Right  [empty]
 */

#include "icon_defs.h"
#include "icons.h"
#include "ui_styles.h"

/* ── Icon definitions table ────────────────────────────────────────── */
/*                                                                       */
/* CAN mappings: set can_msg_id to 0x000 as placeholder until the team   */
/* maps the actual message IDs from VCU / BMS CAN specs.                 */

const icon_def_t icon_defs[ICON_COUNT] = {
    /*  id                       name               img              color              row  col  can_id  byte mask */
    { ICON_HVIL,                "HVIL",             NULL,            ICON_COLOR_RED,     0,   0,  0x000,  0,   0x00 },
    { ICON_CUP_HEAT,            "Cup Heat",         NULL,            ICON_COLOR_AMBER,   0,   3,  0x000,  0,   0x00 },
    { ICON_PARKING_BRAKE_SLASH, "PBrake Slash",     NULL,            ICON_COLOR_RED,     1,   0,  0x000,  0,   0x00 },
    { ICON_SEATBELT,            "Seatbelt",         NULL,            ICON_COLOR_RED,     1,   1,  0x000,  0,   0x00 },
    { ICON_PARKING_LIGHT,       "Parking Light",    NULL,            ICON_COLOR_BLUE,    1,   2,  0x000,  0,   0x00 },
    { ICON_HAZARD,              "Hazard",           NULL,            ICON_COLOR_RED,     1,   3,  0x000,  0,   0x00 },
    { ICON_PARKING_BRAKE,       "Parking Brake",    NULL,            ICON_COLOR_RED,     2,   0,  0x000,  0,   0x00 },
    { ICON_EV_CHARGER,          "EV Charger",       NULL,            ICON_COLOR_AMBER,   2,   1,  0x000,  0,   0x00 },
    { ICON_HIGH_BEAM,           "High Beam",        NULL,            ICON_COLOR_GREEN,   2,   2,  0x000,  0,   0x00 },
    { ICON_BRAKE_WARNING,       "Brake Warning",    NULL,            ICON_COLOR_RED,     2,   3,  0x000,  0,   0x00 },
    { ICON_ABS,                 "ABS",              NULL,            ICON_COLOR_AMBER,   3,   0,  0x000,  0,   0x00 },
    { ICON_ENGINE,              "Engine",           NULL,            ICON_COLOR_RED,     3,   1,  0x000,  0,   0x00 },
    { ICON_BATTERY_12V,         "12V Battery",      NULL,            ICON_COLOR_RED,     3,   2,  0x000,  0,   0x00 },
    { ICON_COOLANT,             "Coolant",          NULL,            ICON_COLOR_BLUE,    3,   3,  0x000,  0,   0x00 },
    { ICON_TRACTION_CONTROL,    "Traction Ctrl",    NULL,            ICON_COLOR_AMBER,   4,   0,  0x000,  0,   0x00 },
    { ICON_THERMO_HOT,          "Thermo Hot",       NULL,            ICON_COLOR_RED,     4,   1,  0x000,  0,   0x00 },
    { ICON_THERMO_COLD,         "Thermo Cold",      NULL,            ICON_COLOR_BLUE,    4,   2,  0x000,  0,   0x00 },
    { ICON_BOOK_WRENCH,         "Book & Wrench",    NULL,            ICON_COLOR_AMBER,   4,   3,  0x000,  0,   0x00 },
    { ICON_HV_BATTERY_WARN_1,   "HV Batt Warn 1",  NULL,            ICON_COLOR_AMBER,   5,   0,  0x000,  0,   0x00 },
    { ICON_HV_BATTERY_WARN_2,   "HV Batt Warn 2",  NULL,            ICON_COLOR_AMBER,   5,   1,  0x000,  0,   0x00 },
    { ICON_CHARGE_ARROW_LEFT,   "Charge Left",      NULL,            ICON_COLOR_AMBER,   6,   1,  0x000,  0,   0x00 },
    { ICON_CHARGE_ARROW_RIGHT,  "Charge Right",     NULL,            ICON_COLOR_AMBER,   6,   2,  0x000,  0,   0x00 },
};

/* ── Color lookup ──────────────────────────────────────────────────── */

lv_color_t icon_color_to_lv(icon_color_t color)
{
    switch (color) {
        case ICON_COLOR_RED:    return UI_COLOR_RED;
        case ICON_COLOR_AMBER:  return UI_COLOR_AMBER;
        case ICON_COLOR_BLUE:   return UI_COLOR_BLUE;
        case ICON_COLOR_GREEN:  return UI_COLOR_GREEN;
        case ICON_COLOR_WHITE:  return UI_COLOR_WHITE;
        default:                return UI_COLOR_WHITE;
    }
}
