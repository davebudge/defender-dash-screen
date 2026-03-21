/**
 * @file icon_defs.h
 * @brief Warning light icon definitions
 *
 * This is the central configuration for all dashboard warning lights.
 * To add a new icon:
 *   1. Add an entry to icon_id_t enum
 *   2. Create img_xxx.c image file (LVGL C array)
 *   3. Add LV_IMG_DECLARE in icons.h
 *   4. Add a row to icon_defs[] in icon_defs.c
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Icon IDs ──────────────────────────────────────────────────────── */

typedef enum {
    ICON_HVIL = 0,
    ICON_CUP_HEAT,
    ICON_PARKING_BRAKE_SLASH,
    ICON_SEATBELT,
    ICON_PARKING_LIGHT,
    ICON_HAZARD,
    ICON_PARKING_BRAKE,
    ICON_EV_CHARGER,
    ICON_HIGH_BEAM,
    ICON_BRAKE_WARNING,
    ICON_ABS,
    ICON_ENGINE,
    ICON_BATTERY_12V,
    ICON_CHARGE_CCS2,
    ICON_TRACTION_CONTROL,
    ICON_THERMO_HOT,
    ICON_THERMO_COLD,
    ICON_BOOK_WRENCH,
    ICON_HV_PRIMARY_PACK,
    ICON_HV_SECONDARY_PACK,
    ICON_ARROW_LEFT,
    ICON_ARROW_RIGHT,
    ICON_COUNT
} icon_id_t;

/* ── Icon color categories ─────────────────────────────────────────── */

typedef enum {
    ICON_COLOR_RED = 0,
    ICON_COLOR_AMBER,
    ICON_COLOR_BLUE,
    ICON_COLOR_GREEN,
    ICON_COLOR_WHITE,
} icon_color_t;

/* ── Icon definition structure ─────────────────────────────────────── */

typedef struct {
    icon_id_t           id;           /**< Unique icon identifier */
    const char         *name;         /**< Human-readable name (for debug logging) */
    const lv_img_dsc_t *img;          /**< LVGL image descriptor (NULL = placeholder) */
    icon_color_t        color;        /**< Icon color when active */
    uint8_t             grid_row;     /**< Row position in grid (0-based) */
    uint8_t             grid_col;     /**< Column position in grid (0-based) */
    uint32_t            can_msg_id;   /**< CAN message ID (0 = not yet mapped) */
    uint8_t             can_byte;     /**< Byte index within CAN data (0-7) */
    uint8_t             can_bit_mask; /**< Bit mask within that byte */
    bool                adr_self_test;/**< Illuminate during ADR startup self-test */
} icon_def_t;

/* ── Grid layout constants ─────────────────────────────────────────── */

#define ICON_GRID_COLS      4     /**< Number of columns in the icon grid */
#define ICON_GRID_TOP_ROWS  3     /**< Rows above the gear display */
#define ICON_GRID_BOT_ROWS  3     /**< Rows below the gear display */
#define ICON_GRID_EXTRA_ROW 6     /**< Bottom-most row (charge arrows) */

/* ── Access to the icon definitions table ──────────────────────────── */

extern const icon_def_t icon_defs[ICON_COUNT];

/**
 * @brief Get the LVGL recolor value for an icon color category
 */
lv_color_t icon_color_to_lv(icon_color_t color);

#ifdef __cplusplus
}
#endif
