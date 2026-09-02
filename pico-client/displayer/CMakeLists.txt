cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

if (NOT TARGET pico_stdlib)
    include(pico_sdk_import.cmake)
    project(watering LANGUAGES C CXX ASM VERSION 0.0.1)
    pico_sdk_init()
endif()

set(target_name "displayer")

pico_sdk_init()

add_executable(displayer
    main.c
    hal/src/hal_display_ssd1680.c
    hal/src/hal_server.c
    text_renderer.c
    disp_task.c
    comm_task.c
    dispatch_table.c
    ../common/dispatcher.c
    ../common/sched.c
    ../common/wd_task.c
    ../common/hal/src/hal_network.c
    ../common/hal/src/hal_wlan.c
    ../common/hal/src/hal_system.c
)

pico_enable_stdio_usb(displayer 1)
pico_enable_stdio_uart(displayer 0)

target_include_directories(displayer PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/../common/
    ${CMAKE_CURRENT_LIST_DIR}/hal/include/
    ${CMAKE_CURRENT_LIST_DIR}/../common/hal/include/
)

target_link_libraries(displayer
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip
    hardware_spi
    hardware_watchdog
    hardware_timer
    hardware_irq
    hardware_sync
)

target_compile_definitions(${target_name} PUBLIC
    WIFI_SSID="${WIFI_SSID}"
    WIFI_PASS="${WIFI_PASS}"
    TEST_TCP_SERVER_IP="${TEST_TCP_SERVER_IP}"
)

pico_add_extra_outputs(displayer)