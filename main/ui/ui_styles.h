/**
 * @file ui_styles.h
 * @brief Shared UI styles and color definitions for the dashboard
 */

#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Dashboard colors ──────────────────────────────────────────────── */

#define UI_COLOR_BG         lv_color_black()
#define UI_COLOR_RED         lv_color_make(0xFF, 0x00, 0x00)    /* SAE J578 signal red */
#define UI_COLOR_AMBER       lv_color_make(0xFF, 0x8C, 0x00)    /* SAE J578 amber (orange) */
#define UI_COLOR_BLUE        lv_color_make(0x00, 0x66, 0xFF)    /* SAE J578 signal blue */
#define UI_COLOR_GREEN       lv_color_make(0x00, 0xCC, 0x00)    /* SAE J578 signal green */
#define UI_COLOR_WHITE       lv_color_white()
#define UI_COLOR_DIM         lv_color_make(0x30, 0x30, 0x30)

/* ── Opacity values ────────────────────────────────────────────────── */

#define UI_OPA_ICON_ON       LV_OPA_COVER    /* 100% - icon active */
#define UI_OPA_ICON_OFF      LV_OPA_20       /* 20%  - icon dim/silhouette */

/**
 * @brief Initialize shared styles
 *
 * Must be called once before creating any UI elements.
 */
void ui_styles_init(void);

#ifdef __cplusplus
}
#endif
