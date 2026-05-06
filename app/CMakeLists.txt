# app/CMakeLists.txt

# MAIN
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/app.c
)

# INTERFACE

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pwm/src/interface_pwm.c
)


target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pwm/inc/
)