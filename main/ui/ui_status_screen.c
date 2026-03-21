/**
 * @file ui_status_screen.c
 * @brief Vehicle status telemetry screen implementation
 *
 * Shows a vertical list of label-value pairs for battery
 * and vehicle telemetry data. Uses Andale Mono font.
 */

#include "ui_status_screen.h"
#include "vehicle_state.h"
#include "ui_styles.h"
#include "fonts.h"

#include <stdio.h>

/* ── Layout constants ──────────────────────────────────────────────── */

#define STATUS_PAD_X     24     /* Left/right padding */
#define STATUS_PAD_TOP   16     /* Top padding below title */
#define ROW_HEIGHT       44     /* Height of each data row */
#define TITLE_HEIGHT     50     /* Height of title area */
#define LABEL_COLOR      lv_color_make(0xAA, 0xAA, 0xAA)  /* Light gray labels */
#define VALUE_COLOR      lv_color_white()
#define DIVIDER_COLOR    lv_color_make(0x40, 0x40, 0x40)

/* ── Status row definitions ────────────────────────────────────────── */

#define STATUS_ROW_COUNT  15

typedef enum {
    SROW_SOC = 0,
    SROW_VOLTAGE,
    SROW_CURRENT,
    SROW_TEMP_FRONT,
    SROW_TEMP_REAR,
    SROW_VOLTAGE_DELTA,
    SROW_CELL_BALANCE,
    SROW_CHARGE_CYCLES,
    SROW_BATTERY_HEALTH,
    SROW_MAX_DISCHARGE,
    SROW_RANGE,
    SROW_LAST_CHARGED,
    SROW_NEXT_MAINT,
    SROW_FIRMWARE,
    SROW_COUNT
} status_row_id_t;

static const char *ROW_LABELS[] = {
    "STATE OF CHARGE",
    "BATTERY VOLTAGE",
    "BATTERY CURRENT",
    "BATTERY TEMP FRONT",
    "BATTERY TEMP REAR",
    "BATTERY PACK VOLTAGE DELTA",
    "CELL BALANCE",
    "CHARGE CYCLES",
    "BATTERY HEALTH",
    "MAX DISCHARGE CURRENT",
    "ESTIMATED REMAINING RANGE",
    "LAST CHARGED DATE",
    "NEXT MAINTENANCE DUE",
    "FIRMWARE VERSION",
};

/* ── Internal state ────────────────────────────────────────────────── */

static lv_obj_t *s_value_labels[SROW_COUNT] = {0};

/* ── Implementation ────────────────────────────────────────────────── */

static lv_obj_t *create_row(lv_obj_t *parent, const char *label_text, int y_pos)
{
    /* Label (left side) */
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_font(label, &font_andale_mono_16, 0);
    lv_obj_set_style_text_color(label, LABEL_COLOR, 0);
    lv_obj_set_pos(label, STATUS_PAD_X, y_pos + 12);

    /* Value (right side) */
    lv_obj_t *value = lv_label_create(parent);
    lv_label_set_text(value, "---");
    lv_obj_set_style_text_font(value, &font_andale_mono_20, 0);
    lv_obj_set_style_text_color(value, VALUE_COLOR, 0);
    lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(value, 200);
    lv_obj_align(value, LV_ALIGN_TOP_RIGHT, -STATUS_PAD_X, y_pos + 10);

    /* Divider line below (subtle) */
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_set_size(line, 480 - (STATUS_PAD_X * 2), 1);
    lv_obj_set_pos(line, STATUS_PAD_X, y_pos + ROW_HEIGHT - 1);
    lv_obj_set_style_bg_color(line, DIVIDER_COLOR, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);

    return value;
}

void ui_status_screen_create(lv_obj_t *parent)
{
    /* Make parent scrollable for the long list */
    lv_obj_set_style_bg_color(parent, UI_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);

    /* Title */
    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "VEHICLE STATUS");
    lv_obj_set_style_text_font(title, &font_andale_mono_20, 0);
    lv_obj_set_style_text_color(title, VALUE_COLOR, 0);
    lv_obj_set_pos(title, STATUS_PAD_X, 16);

    /* Title divider */
    lv_obj_t *title_div = lv_obj_create(parent);
    lv_obj_set_size(title_div, 480 - (STATUS_PAD_X * 2), 2);
    lv_obj_set_pos(title_div, STATUS_PAD_X, TITLE_HEIGHT);
    lv_obj_set_style_bg_color(title_div, DIVIDER_COLOR, 0);
    lv_obj_set_style_bg_opa(title_div, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(title_div, 0, 0);
    lv_obj_set_style_pad_all(title_div, 0, 0);

    /* Create all data rows */
    int y = TITLE_HEIGHT + STATUS_PAD_TOP;
    for (int i = 0; i < SROW_COUNT; i++) {
        s_value_labels[i] = create_row(parent, ROW_LABELS[i], y);
        y += ROW_HEIGHT;
    }
}

void ui_status_screen_update(void)
{
    vehicle_status_t st = vehicle_state_get_status();
    char buf[32];

    snprintf(buf, sizeof(buf), "%.1f%%", st.soc);
    lv_label_set_text(s_value_labels[SROW_SOC], buf);

    snprintf(buf, sizeof(buf), "%.1fV", st.battery_voltage);
    lv_label_set_text(s_value_labels[SROW_VOLTAGE], buf);

    snprintf(buf, sizeof(buf), "%.1fA", st.battery_current);
    lv_label_set_text(s_value_labels[SROW_CURRENT], buf);

    snprintf(buf, sizeof(buf), "%d\xC2\xB0""C", (int)st.battery_temp_front);
    lv_label_set_text(s_value_labels[SROW_TEMP_FRONT], buf);

    snprintf(buf, sizeof(buf), "%d\xC2\xB0""C", (int)st.battery_temp_rear);
    lv_label_set_text(s_value_labels[SROW_TEMP_REAR], buf);

    snprintf(buf, sizeof(buf), "%.2fV", st.pack_voltage_delta);
    lv_label_set_text(s_value_labels[SROW_VOLTAGE_DELTA], buf);

    lv_label_set_text(s_value_labels[SROW_CELL_BALANCE], st.cell_balance);

    snprintf(buf, sizeof(buf), "%u", st.charge_cycles);
    lv_label_set_text(s_value_labels[SROW_CHARGE_CYCLES], buf);

    snprintf(buf, sizeof(buf), "%u%%", st.battery_health);
    lv_label_set_text(s_value_labels[SROW_BATTERY_HEALTH], buf);

    snprintf(buf, sizeof(buf), "%.0fA", st.max_discharge_current);
    lv_label_set_text(s_value_labels[SROW_MAX_DISCHARGE], buf);

    snprintf(buf, sizeof(buf), "%u km", st.estimated_range_km);
    lv_label_set_text(s_value_labels[SROW_RANGE], buf);

    lv_label_set_text(s_value_labels[SROW_LAST_CHARGED], st.last_charged);
    lv_label_set_text(s_value_labels[SROW_NEXT_MAINT], st.next_maintenance);
    lv_label_set_text(s_value_labels[SROW_FIRMWARE], st.firmware_version);
}
