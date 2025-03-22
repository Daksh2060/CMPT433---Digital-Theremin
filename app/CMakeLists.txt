# Build the app, using the HAL

include_directories(include)
include_directories(common/include)

file(GLOB MY_SOURCES "src/*.c")
add_executable(digital_theremin ${MY_SOURCES})

# ALSA support
find_package(ALSA REQUIRED)
target_link_libraries(digital_theremin LINK_PRIVATE asound)

target_link_libraries(digital_theremin LINK_PRIVATE hal)
target_link_libraries(digital_theremin PRIVATE gpiod)
target_link_libraries(digital_theremin PRIVATE common)
target_link_libraries(digital_theremin LINK_PRIVATE lcd)
target_link_libraries(digital_theremin LINK_PRIVATE lgpio)

# Copy executable to final location (change `digital_theremin` to project name as needed)
add_custom_command(TARGET digital_theremin POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:digital_theremin>"
     "~/cmpt433/public/myApps/digital_theremin" 
  COMMENT "Copying ARM executable to public NFS directory")


# Copy the folder of WAVE files to NFS
add_custom_command(TARGET digital_theremin POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
"${CMAKE_SOURCE_DIR}/wave-files"
"~/cmpt433/public/myApps/wave-files"
COMMENT "Copying WAVE files to public NFS directory")
