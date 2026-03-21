/**
 * @file ui_main.c
 * @brief Top-level UI initialization with swipe navigation
 *
 * Uses LVGL tileview for horizontal swipe between screens:
 *   - Tile 0 (default): Warning lights + gear selector
 *   - Tile 1: Vehicle status telemetry
 *
 * Always starts on Tile 0 (main dashboard).
 * Nav dots are inside the tileview tiles so they move with swipe.
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

#define NUM_TILES        2
#define NAV_DOT_SIZE     8
#define NAV_DOT_SPACING  12
#define NAV_DOT_Y        778   /* Near bottom of screen */

static lv_obj_t *s_tileview = NULL;
static lv_obj_t *s_tiles[NUM_TILES] = {0};
static int s_current_tile = 0;

/* Nav dots - one set per tile so they're always visible */
static lv_obj_t *s_nav_dots[NUM_TILES][NUM_TILES];

/* ── Update timer ──────────────────────────────────────────────────── */

#define UI_UPDATE_PERIOD_MS  100

static void ui_update_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if (vehicle_state_is_dirty()) {
        /* Only update the visible screen for efficiency */
        if (s_current_tile == 0) {
            ui_warning_grid_update();
            ui_gear_display_update();
        } else {
            ui_status_screen_update();
        }
    }
}

/* ── Navigation dots ───────────────────────────────────────────────── */

static void update_nav_dots(int active_idx)
{
    for (int tile = 0; tile < NUM_TILES; tile++) {
        for (int dot = 0; dot < NUM_TILES; dot++) {
            if (s_nav_dots[tile][dot] == NULL) continue;

            if (dot == active_idx) {
                lv_obj_set_size(s_nav_dots[tile][dot], NAV_DOT_SIZE * 3, NAV_DOT_SIZE);
                lv_obj_set_style_bg_color(s_nav_dots[tile][dot], UI_COLOR_WHITE, 0);
                lv_obj_set_style_bg_opa(s_nav_dots[tile][dot], LV_OPA_COVER, 0);
            } else {
                lv_obj_set_size(s_nav_dots[tile][dot], NAV_DOT_SIZE, NAV_DOT_SIZE);
                lv_obj_set_style_bg_color(s_nav_dots[tile][dot], UI_COLOR_DIM, 0);
                lv_obj_set_style_bg_opa(s_nav_dots[tile][dot], LV_OPA_COVER, 0);
            }
        }
    }
}

static void tileview_event_cb(lv_event_t *e)
{
    lv_obj_t *tv = lv_event_get_target(e);
    lv_obj_t *active_tile = lv_tileview_get_tile_act(tv);

    /* Find which tile is active */
    for (int i = 0; i < NUM_TILES; i++) {
        if (active_tile == s_tiles[i] && i != s_current_tile) {
            s_current_tile = i;
            update_nav_dots(i);
            ESP_LOGD(TAG, "Switched to tile %d", i);
            break;
        }
    }
}

static void create_nav_dots(lv_obj_t *tile, int tile_idx)
{
    int total_w = (NAV_DOT_SIZE * 3) + ((NUM_TILES - 1) * NAV_DOT_SPACING) + NAV_DOT_SIZE;
    int start_x = (SCREEN_W - total_w) / 2;

    for (int i = 0; i < NUM_TILES; i++) {
        lv_obj_t *dot = lv_obj_create(tile);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_pad_all(dot, 0, 0);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

        int x = start_x + i * (NAV_DOT_SIZE + NAV_DOT_SPACING);
        lv_obj_set_pos(dot, x, NAV_DOT_Y);

        s_nav_dots[tile_idx][i] = dot;
    }
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
             SCREEN_W, SCREEN_H, NUM_TILES);

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
    s_tiles[0] = lv_tileview_add_tile(s_tileview, 0, 0, LV_DIR_RIGHT);
    lv_obj_set_style_bg_color(s_tiles[0], UI_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(s_tiles[0], LV_OPA_COVER, 0);

    lv_obj_t *top_grid = create_container(s_tiles[0], 0, TOP_GRID_Y, SCREEN_W, TOP_GRID_H);
    lv_obj_t *gear_area = create_container(s_tiles[0], 0, GEAR_AREA_Y, SCREEN_W, GEAR_AREA_H);
    lv_obj_t *bot_grid = create_container(s_tiles[0], 0, BOT_GRID_Y, SCREEN_W, BOT_GRID_H);

    ui_warning_grid_create(top_grid, bot_grid);
    ui_gear_display_create(gear_area);
    create_nav_dots(s_tiles[0], 0);

    /* ── Tile 1: Vehicle status screen ─────────────────────────────── */
    s_tiles[1] = lv_tileview_add_tile(s_tileview, 1, 0, LV_DIR_LEFT);

    ui_status_screen_create(s_tiles[1]);
    create_nav_dots(s_tiles[1], 1);

    /* ── Start on tile 0 ───────────────────────────────────────────── */
    lv_obj_set_tile_id(s_tileview, 0, 0, LV_ANIM_OFF);
    s_current_tile = 0;
    update_nav_dots(0);

    /* Start periodic update timer */
    lv_timer_create(ui_update_timer_cb, UI_UPDATE_PERIOD_MS, NULL);

    ESP_LOGI(TAG, "Dashboard UI initialized (swipe enabled)");
}
