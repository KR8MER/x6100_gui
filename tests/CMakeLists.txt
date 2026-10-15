cmake_minimum_required(VERSION 3.23)

# add_executable(tests test.cpp)
# target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

add_compile_options(-fsanitize=address -fsanitize=undefined  -fno-omit-frame-pointer -fno-sanitize-recover)
add_link_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-sanitize-recover -static-libasan -static-libubsan)


add_subdirectory(../src/qth ${CMAKE_BINARY_DIR}/src/qth)
add_executable(test_qth test_qth.cpp)
target_link_libraries(test_qth PRIVATE QTH Catch2::Catch2WithMain)
add_test(NAME test_qth COMMAND $<TARGET_FILE:test_qth> --colour-mode=ansi )

# test_tz: clock time zones (plain C, no dependencies)
add_executable(test_tz test_tz.cpp ${PROJECT_SOURCE_DIR}/src/tz_display.c)
target_link_libraries(test_tz PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_tz COMMAND $<TARGET_FILE:test_tz> --colour-mode=ansi)

# test_ops: cockpits, log rules, POTA hunt and spot parsing
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(JANSSON REQUIRED IMPORTED_TARGET jansson)
add_executable(test_ops test_ops.cpp
    ${PROJECT_SOURCE_DIR}/src/cockpit_rules.c ${PROJECT_SOURCE_DIR}/src/log_rules.c
    ${PROJECT_SOURCE_DIR}/src/hunt_target.c ${PROJECT_SOURCE_DIR}/src/pota_spots.c)
target_compile_definitions(test_ops PRIVATE _GNU_SOURCE)
target_link_libraries(test_ops PRIVATE Catch2::Catch2WithMain PkgConfig::JANSSON Threads::Threads)
add_test(NAME test_ops COMMAND $<TARGET_FILE:test_ops> --colour-mode=ansi)

# On-line services: parsers only (no network in tests)
add_executable(test_dxc test_dxc.cpp ${PROJECT_SOURCE_DIR}/src/dx_spots.c)
target_compile_definitions(test_dxc PRIVATE _GNU_SOURCE)
target_link_libraries(test_dxc PRIVATE Catch2::Catch2WithMain PkgConfig::JANSSON)
add_test(NAME test_dxc COMMAND $<TARGET_FILE:test_dxc> --colour-mode=ansi)

add_executable(test_spacewx test_spacewx.cpp ${PROJECT_SOURCE_DIR}/src/space_weather.c)
target_compile_definitions(test_spacewx PRIVATE _GNU_SOURCE)
target_link_libraries(test_spacewx PRIVATE Catch2::Catch2WithMain PkgConfig::JANSSON)
add_test(NAME test_spacewx COMMAND $<TARGET_FILE:test_spacewx> --colour-mode=ansi)

add_executable(test_callbook test_callbook.cpp ${PROJECT_SOURCE_DIR}/src/callbook.c)
target_compile_definitions(test_callbook PRIVATE _GNU_SOURCE)
target_link_libraries(test_callbook PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_callbook COMMAND $<TARGET_FILE:test_callbook> --colour-mode=ansi)

add_subdirectory(../src/ft8 ${CMAKE_BINARY_DIR}/src/ft8)
add_executable(test_ft8_qso test_ft8_qso.cpp)
target_link_libraries(test_ft8_qso PRIVATE ft8_core QTH aether_x6100_control Catch2::Catch2WithMain)
add_test(NAME test_ft8_qso COMMAND $<TARGET_FILE:test_ft8_qso> --colour-mode=ansi )


# test_js8_db: params.db for every kind of card (src/cfg/js8_db.c with
# migrations.c; plain C, sqlite3 only). Its own binary: it stands in for
# db.cpp's cfg_db_get(), which test_cfg links.
add_executable(test_js8_db test_js8_db.cpp
    ${PROJECT_SOURCE_DIR}/src/cfg/js8_db.c ${PROJECT_SOURCE_DIR}/src/cfg/migrations.c)
target_include_directories(test_js8_db PRIVATE ${PROJECT_SOURCE_DIR}/src/cfg)
target_link_libraries(test_js8_db PRIVATE Catch2::Catch2WithMain sqlite3)
add_test(NAME test_js8_db COMMAND $<TARGET_FILE:test_js8_db> --colour-mode=ansi)

add_subdirectory(cfg)

add_subdirectory(../src/cw_decoder ${CMAKE_BINARY_DIR}/src/cw_decoder)
add_subdirectory(cw_decoder)

add_subdirectory(../src/cat ${CMAKE_BINARY_DIR}/src/cat)
add_executable(test_civ test_civ.cpp)
target_include_directories(test_civ PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(test_civ PRIVATE cat_core Catch2::Catch2WithMain)
add_test(NAME test_civ COMMAND $<TARGET_FILE:test_civ> --colour-mode=ansi)
