file(GLOB ICON_SRCS "${CMAKE_CURRENT_LIST_DIR}/icons/img_*.c")
file(GLOB FONT_SRCS "${CMAKE_CURRENT_LIST_DIR}/fonts/*.c")

idf_component_register(
    SRCS
        "main.c"
        "lvgl_port.c"
        "waveshare_rgb_lcd_port.c"
        "can/can_bus.c"
        "ui/ui_main.c"
        "ui/ui_warning_grid.c"
        "ui/ui_gear_display.c"
        "ui/ui_status_screen.c"
        "ui/ui_styles.c"
        "data/vehicle_state.c"
        "icons/icon_defs.c"
        ${ICON_SRCS}
        ${FONT_SRCS}
    INCLUDE_DIRS
        "."
        "can"
        "ui"
        "data"
        "icons"
        "fonts"
    REQUIRES
        driver
)
