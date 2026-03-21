/**
 * @file fonts.h
 * @brief Custom font declarations for the dashboard
 */

#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Eurostile Extended #2 Bold at 96px - glyphs: D, N, P, R, S */
LV_FONT_DECLARE(font_eurostile_96);

/** Andale Mono 16px - status screen labels */
LV_FONT_DECLARE(font_andale_mono_16);

/** Andale Mono 20px - status screen values */
LV_FONT_DECLARE(font_andale_mono_20);

#ifdef __cplusplus
}
#endif
