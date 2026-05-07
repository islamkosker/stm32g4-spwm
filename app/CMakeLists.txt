# app/CMakeLists.txt

# MAIN
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/app.c
)

# INTERFACE
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pwm/src/interface_pwm.c
)

# PWM
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/pwm/src/app_pwm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pwm/src/app_pwm_constant.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pwm/src/pwm_bindings.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/
    ${CMAKE_CURRENT_SOURCE_DIR}/pwm/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pwm/inc/
)