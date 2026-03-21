/**
 * @file ui_warning_grid.c
 * @brief Data-driven warning light icon grid implementation
 */

#include "ui_warning_grid.h"
#include "icon_defs.h"
#include "vehicle_state.h"
#include "ui_styles.h"

#include "esp_log.h"

static const char *TAG = "UI_GRID";

/* ── Grid layout constants (pixels) ────────────────────────────────── */

#define CELL_W          120   /* Width of each grid cell */
#define CELL_H          90    /* Height of each grid cell */
#define ICON_SIZE       64    /* Icon image size (square) */
#define GRID_PAD_X      0     /* Horizontal padding from edge */
#define GRID_PAD_Y      0     /* Vertical padding from edge */

/* ── Internal state ────────────────────────────────────────────────── */

static lv_obj_t *s_icon_objs[ICON_COUNT] = {0};
static bool s_blink_state = false;  /* Toggles for BLINK animation */

/* ── Implementation ────────────────────────────────────────────────── */

void ui_warning_grid_create(lv_obj_t *top_parent, lv_obj_t *bot_parent)
{
    for (int i = 0; i < ICON_COUNT; i++) {
        const icon_def_t *def = &icon_defs[i];

        /* Determine parent based on row (rows 0-2 top, 3+ bottom) */
        lv_obj_t *parent;
        int local_row;

        if (def->grid_row < ICON_GRID_TOP_ROWS) {
            parent = top_parent;
            local_row = def->grid_row;
        } else {
            parent = bot_parent;
            local_row = def->grid_row - ICON_GRID_TOP_ROWS;
        }

        /* Create image object */
        lv_obj_t *img = lv_img_create(parent);

        /* Set image source if available, otherwise use a placeholder */
        if (def->img != NULL) {
            lv_img_set_src(img, def->img);
        }
        /* If img is NULL, the object will be empty until real icons are added */

        /* Set size and position */
        lv_obj_set_size(img, ICON_SIZE, ICON_SIZE);

        int x = GRID_PAD_X + def->grid_col * CELL_W + (CELL_W - ICON_SIZE) / 2;
        int y = GRID_PAD_Y + local_row * CELL_H + (CELL_H - ICON_SIZE) / 2;
        lv_obj_set_pos(img, x, y);

        /* Enable image recoloring - icons are white, we tint them */
        lv_obj_set_style_img_recolor(img, icon_color_to_lv(def->color), 0);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);

        /* Start in OFF state (dimmed) */
        lv_obj_set_style_img_opa(img, UI_OPA_ICON_OFF, 0);

        s_icon_objs[i] = img;

        ESP_LOGD(TAG, "Created icon: %s at row=%d col=%d", def->name, def->grid_row, def->grid_col);
    }

    ESP_LOGI(TAG, "Warning grid created (%d icons)", ICON_COUNT);
}

void ui_warning_grid_update(void)
{
    /* Toggle blink state each call (called at 100ms interval = 5Hz blink) */
    s_blink_state = !s_blink_state;

    for (int i = 0; i < ICON_COUNT; i++) {
        if (s_icon_objs[i] == NULL) continue;

        icon_state_t state = vehicle_state_get_icon((icon_id_t)i);

        lv_opa_t opa;
        switch (state) {
            case ICON_STATE_ON:
                opa = UI_OPA_ICON_ON;
                break;
            case ICON_STATE_BLINK:
                opa = s_blink_state ? UI_OPA_ICON_ON : UI_OPA_ICON_OFF;
                break;
            case ICON_STATE_OFF:
            default:
                opa = UI_OPA_ICON_OFF;
                break;
        }

        lv_obj_set_style_img_opa(s_icon_objs[i], opa, 0);
    }
}
