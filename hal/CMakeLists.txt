# CMakeList.txt for HAL
#   Build a library (`hal`) which exposes the header files as "hal/*.h"
#   Use header as: #include "hal/button.h"

include_directories(hal/include)
include_directories(common/include)

file(GLOB MY_SOURCES "src/*.c")

add_library(hal STATIC ${MY_SOURCES})

target_link_libraries(hal PRIVATE common)
target_link_libraries(hal LINK_PRIVATE lcd)
target_link_libraries(hal LINK_PRIVATE lgpio)

find_package(ALSA REQUIRED)
target_link_libraries(hal LINK_PRIVATE asound)

target_include_directories(hal PUBLIC include)
