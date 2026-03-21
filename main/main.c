/**
 * @file main.c
 * @brief Defender Dashboard - Application Entry Point
 *
 * Boot sequence:
 * 1. Initialize LCD hardware + backlight
 * 2. Show Jaunt Motors splash screen immediately
 * 3. Initialize I2C, CAN bus, vehicle state in background
 * 4. Transition to main dashboard UI
 *
 * The LVGL task runs on Core 1, CAN RX task on Core 0.
 */

#include "waveshare_rgb_lcd_port.h"
#include "lvgl_port.h"
#include "CH422G.h"
#include "DEV_Config.h"
#include "can_bus.h"
#include "ui_main.h"
#include "vehicle_state.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

/* ── Splash screen ─────────────────────────────────────────────────── */

LV_IMG_DECLARE(img_jaunt_logo);

#define SPLASH_MIN_DISPLAY_MS   1500   /* Minimum time splash is shown */

static void show_splash_screen(void)
{
    if (!lvgl_port_lock(-1)) {
        ESP_LOGE(TAG, "Failed to lock LVGL for splash screen");
        return;
    }

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Jaunt logo centered on screen */
    lv_obj_t *logo = lv_img_create(scr);
    lv_img_set_src(logo, &img_jaunt_logo);
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();
}

static void clear_splash_and_init_ui(void)
{
    if (!lvgl_port_lock(-1)) {
        ESP_LOGE(TAG, "Failed to lock LVGL for UI init");
        return;
    }

    /* Clear splash objects */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);

    /* Initialize the main dashboard UI */
    ui_init();

    lvgl_port_unlock();

    ESP_LOGI(TAG, "Dashboard UI ready");
}

/* ── Main ──────────────────────────────────────────────────────────── */

void app_main(void)
{
    ESP_LOGI(TAG, "Defender Dashboard starting...");

    /* 1. Initialize display hardware (LCD + touch + backlight) */
    waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();  /* Turn on backlight immediately */

    /* 2. Initialize I2C (needed for CH422G GPIO expander) */
    DEV_Module_Init();

    /* 3. Show splash screen ASAP so display isn't blank */
    show_splash_screen();
    TickType_t splash_start = xTaskGetTickCount();

    ESP_LOGI(TAG, "Splash screen displayed");

    /* 4. Initialize subsystems while splash is visible */
    vehicle_state_init();

    esp_err_t ret = can_bus_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN bus init failed: %s (dashboard will run without CAN)",
                 esp_err_to_name(ret));
        /* Continue - display should still work without CAN */
    }

    /* 5. Ensure splash is shown for minimum time */
    TickType_t elapsed = xTaskGetTickCount() - splash_start;
    TickType_t min_ticks = pdMS_TO_TICKS(SPLASH_MIN_DISPLAY_MS);
    if (elapsed < min_ticks) {
        vTaskDelay(min_ticks - elapsed);
    }

    /* 6. Transition from splash to main dashboard */
    clear_splash_and_init_ui();

    /* 7. Start CAN bus receive task (runs on Core 0) */
    if (ret == ESP_OK) {
        ret = can_bus_start_task();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "CAN RX task start failed: %s", esp_err_to_name(ret));
        }
    }

    ESP_LOGI(TAG, "Defender Dashboard initialized successfully");
}
