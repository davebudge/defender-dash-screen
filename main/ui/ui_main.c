/**
 * @file ui_main.c
 * @brief Top-level UI initialization and layout
 *
 * Screen layout (480 x 800 portrait):
 *
 *   ┌──────────────────────┐  y=0
 *   │   Top Warning Grid   │  rows 0-2 (270px)
 *   │                      │
 *   ├──────────────────────┤  y=270
 *   │                      │
 *   │    GEAR DISPLAY      │  ~200px
 *   │        (D)           │
 *   │                      │
 *   ├──────────────────────┤  y=470
 *   │  Bottom Warning Grid │  rows 3-6 (~330px)
 *   │                      │
 *   └──────────────────────┘  y=800
 */

#include "ui_main.h"
#include "ui_styles.h"
#include "ui_warning_grid.h"
#include "ui_gear_display.h"
#include "vehicle_state.h"

#include "lvgl.h"
#include "esp_log.h"

static const char *TAG = "UI_MAIN";

/* ── Layout constants ──────────────────────────────────────────────── */

#define SCREEN_W        480
#define SCREEN_H        800

#define TOP_GRID_Y      10
#define TOP_GRID_H      270
#define GEAR_AREA_Y     280
#define GEAR_AREA_H     190
#define BOT_GRID_Y      470
#define BOT_GRID_H      330

/* ── Update timer ──────────────────────────────────────────────────── */

#define UI_UPDATE_PERIOD_MS  100

static void ui_update_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    /* Only update UI when vehicle state has changed */
    if (vehicle_state_is_dirty()) {
        ui_warning_grid_update();
        ui_gear_display_update();
    }
}

/* ── Helper: create a transparent container ────────────────────────── */

static lv_obj_t *create_container(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, w, h);
    lv_obj_set_pos(cont, x, y);

    /* Make container invisible (no border, no background) */
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    return cont;
}

/* ── Public API ────────────────────────────────────────────────────── */

void ui_init(void)
{
    ESP_LOGI(TAG, "Initializing dashboard UI (%dx%d)", SCREEN_W, SCREEN_H);

    /* Initialize shared styles */
    ui_styles_init();

    /* Configure root screen */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, UI_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* Create layout containers */
    lv_obj_t *top_grid = create_container(scr, 0, TOP_GRID_Y, SCREEN_W, TOP_GRID_H);
    lv_obj_t *gear_area = create_container(scr, 0, GEAR_AREA_Y, SCREEN_W, GEAR_AREA_H);
    lv_obj_t *bot_grid = create_container(scr, 0, BOT_GRID_Y, SCREEN_W, BOT_GRID_H);

    /* Create UI components */
    ui_warning_grid_create(top_grid, bot_grid);
    ui_gear_display_create(gear_area);

    /* Start periodic update timer */
    lv_timer_create(ui_update_timer_cb, UI_UPDATE_PERIOD_MS, NULL);

    ESP_LOGI(TAG, "Dashboard UI initialized");
}
