# tests/CMakeLists.txt

add_library(unity
    ${CMAKE_SOURCE_DIR}/unity/unity.c
)

target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/unity
)


add_executable(unit_tests
    app/main/test_main.c

    app/pwm/test_pwm_phase.c
    app/pwm/test_pwm_commit.c

    app/protocol/test_protocol.c

    app/mocks/interface_stubs.c
    app/mocks/protocol_uart_tx_stub.c
)

target_sources(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/app/pwm/src/app_pwm.c
    ${CMAKE_SOURCE_DIR}/app/pwm/src/app_pwm_constant.c
    ${CMAKE_SOURCE_DIR}/app/protocol/src/app_protocol.c
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/app/mocks/
    ${CMAKE_SOURCE_DIR}/tests/app/common/

    ${CMAKE_SOURCE_DIR}/app/pwm/inc/
    ${CMAKE_SOURCE_DIR}/app/protocol/inc/
    ${CMAKE_SOURCE_DIR}/app/interface/
    ${CMAKE_SOURCE_DIR}/app/interface/pwm/inc/
    ${CMAKE_SOURCE_DIR}/app/interface/timer/inc/
    ${CMAKE_SOURCE_DIR}/app/interface/uart/inc/

    ${CMAKE_SOURCE_DIR}/unity/
)


target_link_libraries(unit_tests PRIVATE unity)
target_link_libraries(unit_tests PRIVATE m)

add_test(NAME unit_tests COMMAND unit_tests)
