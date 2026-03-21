/**
 * @file ui_gear_display.c
 * @brief Central gear selector display with directional animation
 *
 * When the gear changes, the current letter slides and fades out in
 * the direction matching the physical selector movement (P R N D S
 * left-to-right), while the new letter slides in from the opposite
 * side. Animation is short and subtle (~200ms).
 */

#include "ui_gear_display.h"
#include "vehicle_state.h"
#include "ui_styles.h"
#include "fonts.h"

#include "esp_log.h"

static const char *TAG = "UI_GEAR";

/* ── Gear letter lookup ────────────────────────────────────────────── */

static const char * const GEAR_LETTERS[GEAR_COUNT] = { "P", "R", "N", "D", "S" };

/* ── Animation parameters ──────────────────────────────────────────── */

#define ANIM_DURATION_MS    200    /* Animation length */
#define ANIM_SLIDE_PX       40     /* Pixels to slide */

/* ── Internal state ────────────────────────────────────────────────── */

static lv_obj_t *s_gear_label = NULL;      /* Current visible label */
static lv_obj_t *s_gear_label_out = NULL;   /* Outgoing label (during animation) */
static lv_obj_t *s_gear_parent = NULL;      /* Parent container */
static gear_t s_last_gear = GEAR_P;
static bool s_animating = false;

/* ── Animation callbacks ───────────────────────────────────────────── */

static void anim_x_cb(void *obj, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)obj, v);
}

static void anim_opa_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

static void anim_out_ready_cb(lv_anim_t *a)
{
    /* Delete the outgoing label once animation finishes */
    if (s_gear_label_out != NULL) {
        lv_obj_del(s_gear_label_out);
        s_gear_label_out = NULL;
    }
    s_animating = false;
}

static lv_obj_t *create_gear_label(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &font_eurostile_96, 0);
    lv_obj_set_style_text_color(label, UI_COLOR_WHITE, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, text);
    return label;
}

/* ── Implementation ────────────────────────────────────────────────── */

void ui_gear_display_create(lv_obj_t *parent)
{
    s_gear_parent = parent;

    /* Clip children so sliding labels don't show outside */
    lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    s_gear_label = create_gear_label(parent, "P");
    s_last_gear = GEAR_P;
    s_animating = false;

    ESP_LOGI(TAG, "Gear display created");
}

void ui_gear_display_update(void)
{
    if (s_gear_label == NULL || s_gear_parent == NULL) return;
    if (s_animating) return;  /* Don't interrupt a running animation */

    gear_t gear = vehicle_state_get_gear();

    if (gear == s_last_gear || gear >= GEAR_COUNT) return;

    ESP_LOGD(TAG, "Gear %c -> %c", "PRNDS"[s_last_gear], "PRNDS"[gear]);

    /*
     * Direction: P(0) R(1) N(2) D(3) S(4)
     * If new > old: physical dial moved right → letter exits LEFT
     * If new < old: physical dial moved left  → letter exits RIGHT
     */
    int direction = (gear > s_last_gear) ? -1 : 1;
    int slide_out = direction * ANIM_SLIDE_PX;   /* Outgoing slides this way */
    int slide_in  = -direction * ANIM_SLIDE_PX;  /* Incoming starts opposite */

    s_animating = true;

    /* ── Outgoing animation: current label slides out + fades ────── */
    s_gear_label_out = s_gear_label;

    lv_anim_t a_out_x;
    lv_anim_init(&a_out_x);
    lv_anim_set_var(&a_out_x, s_gear_label_out);
    lv_anim_set_exec_cb(&a_out_x, anim_x_cb);
    lv_anim_set_values(&a_out_x, lv_obj_get_x(s_gear_label_out), lv_obj_get_x(s_gear_label_out) + slide_out);
    lv_anim_set_time(&a_out_x, ANIM_DURATION_MS);
    lv_anim_set_path_cb(&a_out_x, lv_anim_path_ease_in);
    lv_anim_set_ready_cb(&a_out_x, anim_out_ready_cb);
    lv_anim_start(&a_out_x);

    lv_anim_t a_out_opa;
    lv_anim_init(&a_out_opa);
    lv_anim_set_var(&a_out_opa, s_gear_label_out);
    lv_anim_set_exec_cb(&a_out_opa, anim_opa_cb);
    lv_anim_set_values(&a_out_opa, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&a_out_opa, ANIM_DURATION_MS);
    lv_anim_set_path_cb(&a_out_opa, lv_anim_path_ease_in);
    lv_anim_start(&a_out_opa);

    /* ── Incoming animation: new label slides in + fades in ──────── */
    s_gear_label = create_gear_label(s_gear_parent, GEAR_LETTERS[gear]);
    lv_obj_set_style_opa(s_gear_label, LV_OPA_TRANSP, 0);

    /* Get the center X position for alignment */
    lv_obj_update_layout(s_gear_parent);
    lv_coord_t center_x = (lv_obj_get_width(s_gear_parent) - lv_obj_get_width(s_gear_label)) / 2;

    lv_anim_t a_in_x;
    lv_anim_init(&a_in_x);
    lv_anim_set_var(&a_in_x, s_gear_label);
    lv_anim_set_exec_cb(&a_in_x, anim_x_cb);
    lv_anim_set_values(&a_in_x, center_x + slide_in, center_x);
    lv_anim_set_time(&a_in_x, ANIM_DURATION_MS);
    lv_anim_set_path_cb(&a_in_x, lv_anim_path_ease_out);
    lv_anim_start(&a_in_x);

    lv_anim_t a_in_opa;
    lv_anim_init(&a_in_opa);
    lv_anim_set_var(&a_in_opa, s_gear_label);
    lv_anim_set_exec_cb(&a_in_opa, anim_opa_cb);
    lv_anim_set_values(&a_in_opa, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&a_in_opa, ANIM_DURATION_MS);
    lv_anim_set_path_cb(&a_in_opa, lv_anim_path_ease_out);
    lv_anim_start(&a_in_opa);

    s_last_gear = gear;
}
