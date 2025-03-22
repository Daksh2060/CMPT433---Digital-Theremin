# CMakeLists.txt for Common

include_directories(include)

file(GLOB COMMON_SOURCES src/*.c)

add_library(common STATIC ${COMMON_SOURCES})

target_include_directories(common PUBLIC include)