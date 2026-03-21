/**
 * @file ui_gear_display.c
 * @brief Central gear selector display implementation
 */

#include "ui_gear_display.h"
#include "vehicle_state.h"
#include "ui_styles.h"
#include "fonts.h"

/* ── Gear letter lookup ────────────────────────────────────────────── */

static const char *GEAR_LETTERS[] = { "P", "R", "N", "D", "S" };

/* ── Internal state ────────────────────────────────────────────────── */

static lv_obj_t *s_gear_label = NULL;
static gear_t s_last_gear = GEAR_P;

/* ── Implementation ────────────────────────────────────────────────── */

void ui_gear_display_create(lv_obj_t *parent)
{
    s_gear_label = lv_label_create(parent);

    /* Eurostile Extended #2 Bold at 96px for gear letters */
    lv_obj_set_style_text_font(s_gear_label, &font_eurostile_96, 0);
    lv_obj_set_style_text_color(s_gear_label, UI_COLOR_WHITE, 0);
    lv_obj_set_style_text_align(s_gear_label, LV_TEXT_ALIGN_CENTER, 0);

    /* Center in parent */
    lv_obj_align(s_gear_label, LV_ALIGN_CENTER, 0, 0);

    /* Initial text */
    lv_label_set_text(s_gear_label, "P");
    s_last_gear = GEAR_P;
}

void ui_gear_display_update(void)
{
    if (s_gear_label == NULL) return;

    gear_t gear = vehicle_state_get_gear();

    /* Only update label if gear has changed (avoids unnecessary redraws) */
    if (gear != s_last_gear && gear < GEAR_COUNT) {
        lv_label_set_text(s_gear_label, GEAR_LETTERS[gear]);
        s_last_gear = gear;
    }
}
