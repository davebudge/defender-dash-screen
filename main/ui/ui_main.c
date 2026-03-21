/**
 * @file ui_main.c
 * @brief Top-level UI initialization with swipe navigation
 *
 * Uses LVGL tileview for horizontal swipe between screens:
 *   - Tile 0 (default): Warning lights + gear selector
 *   - Tile 1: Vehicle status telemetry
 *
 * Screen layout per tile (480 x 800 portrait):
 *
 *   ┌──────────────────────┐  y=0
 *   │   Top Warning Grid   │  rows 0-2 (270px)
 *   ├──────────────────────┤  y=270
 *   │    GEAR DISPLAY (D)  │  ~200px
 *   ├──────────────────────┤  y=470
 *   │  Bottom Warning Grid │  rows 3-6 (~330px)
 *   └──────────────────────┘  y=800
 *
 * Always starts on Tile 0 (main dashboard).
 */

#include "ui_main.h"
#include "ui_styles.h"
#include "ui_warning_grid.h"
#include "ui_gear_display.h"
#include "ui_status_screen.h"
#include "vehicle_state.h"
#include "fonts.h"

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

/* ── Swipe navigation ─────────────────────────────────────────────── */

#define NAV_DOT_COUNT    2
#define NAV_DOT_SIZE     8
#define NAV_DOT_SPACING  12
#define NAV_DOT_Y        780   /* Near bottom of screen */

static lv_obj_t *s_tileview = NULL;
static lv_obj_t *s_nav_dots[NAV_DOT_COUNT] = {0};
static int s_current_tile = 0;

/* ── Update timer ──────────────────────────────────────────────────── */

#define UI_UPDATE_PERIOD_MS  100

static void ui_update_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if (vehicle_state_is_dirty()) {
        ui_warning_grid_update();
        ui_gear_display_update();
        ui_status_screen_update();
    }
}

/* ── Navigation dots ───────────────────────────────────────────────── */

static void update_nav_dots(int active_idx)
{
    for (int i = 0; i < NAV_DOT_COUNT; i++) {
        if (s_nav_dots[i] == NULL) continue;

        if (i == active_idx) {
            /* Active dot: white, wider pill shape */
            lv_obj_set_size(s_nav_dots[i], NAV_DOT_SIZE * 3, NAV_DOT_SIZE);
            lv_obj_set_style_bg_color(s_nav_dots[i], UI_COLOR_WHITE, 0);
            lv_obj_set_style_bg_opa(s_nav_dots[i], LV_OPA_COVER, 0);
        } else {
            /* Inactive dot: dim circle */
            lv_obj_set_size(s_nav_dots[i], NAV_DOT_SIZE, NAV_DOT_SIZE);
            lv_obj_set_style_bg_color(s_nav_dots[i], UI_COLOR_DIM, 0);
            lv_obj_set_style_bg_opa(s_nav_dots[i], LV_OPA_COVER, 0);
        }
    }
}

static void tileview_event_cb(lv_event_t *e)
{
    lv_obj_t *tv = lv_event_get_target(e);
    lv_obj_t *active_tile = lv_tileview_get_tile_act(tv);

    /* Get tile index from column position */
    lv_coord_t x = lv_obj_get_x(active_tile);
    int idx = x / SCREEN_W;

    if (idx != s_current_tile) {
        s_current_tile = idx;
        update_nav_dots(idx);
        ESP_LOGD(TAG, "Switched to tile %d", idx);
    }
}

static void create_nav_dots(lv_obj_t *parent)
{
    /* Calculate total width of dots */
    int total_w = (NAV_DOT_SIZE * 3) + ((NAV_DOT_COUNT - 1) * NAV_DOT_SPACING) + NAV_DOT_SIZE;
    int start_x = (SCREEN_W - total_w) / 2;

    for (int i = 0; i < NAV_DOT_COUNT; i++) {
        s_nav_dots[i] = lv_obj_create(parent);
        lv_obj_set_style_border_width(s_nav_dots[i], 0, 0);
        lv_obj_set_style_radius(s_nav_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_pad_all(s_nav_dots[i], 0, 0);
        lv_obj_clear_flag(s_nav_dots[i], LV_OBJ_FLAG_SCROLLABLE);

        int x = start_x + i * (NAV_DOT_SIZE + NAV_DOT_SPACING);
        lv_obj_set_pos(s_nav_dots[i], x, NAV_DOT_Y);
    }

    update_nav_dots(0);
}

/* ── Helper: create a transparent container ────────────────────────── */

static lv_obj_t *create_container(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, w, h);
    lv_obj_set_pos(cont, x, y);

    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    return cont;
}

/* ── Public API ────────────────────────────────────────────────────── */

void ui_init(void)
{
    ESP_LOGI(TAG, "Initializing dashboard UI (%dx%d, %d screens)",
             SCREEN_W, SCREEN_H, NAV_DOT_COUNT);

    ui_styles_init();

    /* Configure root screen */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, UI_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* Create tileview for horizontal swipe navigation */
    s_tileview = lv_tileview_create(scr);
    lv_obj_set_size(s_tileview, SCREEN_W, SCREEN_H);
    lv_obj_set_pos(s_tileview, 0, 0);
    lv_obj_set_style_bg_color(s_tileview, UI_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(s_tileview, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(s_tileview, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* ── Tile 0: Main dashboard (warning lights + gear) ────────────── */
    lv_obj_t *tile_main = lv_tileview_add_tile(s_tileview, 0, 0, LV_DIR_RIGHT);
    lv_obj_set_style_bg_color(tile_main, UI_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(tile_main, LV_OPA_COVER, 0);

    lv_obj_t *top_grid = create_container(tile_main, 0, TOP_GRID_Y, SCREEN_W, TOP_GRID_H);
    lv_obj_t *gear_area = create_container(tile_main, 0, GEAR_AREA_Y, SCREEN_W, GEAR_AREA_H);
    lv_obj_t *bot_grid = create_container(tile_main, 0, BOT_GRID_Y, SCREEN_W, BOT_GRID_H);

    ui_warning_grid_create(top_grid, bot_grid);
    ui_gear_display_create(gear_area);

    /* ── Tile 1: Vehicle status screen ─────────────────────────────── */
    lv_obj_t *tile_status = lv_tileview_add_tile(s_tileview, 1, 0, LV_DIR_LEFT);

    ui_status_screen_create(tile_status);

    /* ── Navigation dots (on root screen, above tileview) ──────────── */
    create_nav_dots(scr);

    /* ── Start on tile 0 ───────────────────────────────────────────── */
    lv_obj_set_tile_id(s_tileview, 0, 0, LV_ANIM_OFF);

    /* Start periodic update timer */
    lv_timer_create(ui_update_timer_cb, UI_UPDATE_PERIOD_MS, NULL);

    ESP_LOGI(TAG, "Dashboard UI initialized (swipe enabled)");
}
