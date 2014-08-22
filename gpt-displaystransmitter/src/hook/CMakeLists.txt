cmake_minimum_required(VERSION 2.8.11)

if ("$ENV{MGEN_INSTALL_PATH}" STREQUAL "")
	message(FATAL_ERROR "Unable to find MGen install path, please check that MGEN_INSTALL_PATH environmental variable is set")
else()
	message("MGen install path found: $ENV{MGEN_INSTALL_PATH}")
endif()

if ("$ENV{MNET_INSTALL_PATH}" STREQUAL "")
	message(FATAL_ERROR "Unable to find MGen install path, please check that MNET_INSTALL_PATH environmental variable is set")
else()
	message("MGen install path found: $ENV{MNET_INSTALL_PATH}")
endif()

project(gpt_displaystransmitter_hook)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5WebSockets)

include_directories(    $ENV{MGEN_INSTALL_PATH}/include
                        $ENV{MNET_INSTALL_PATH}/include
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}../../ext/detours
                        ${CMAKE_CURRENT_SOURCE_DIR}../../../gpt-common/src_generated/main/cpp   )

add_executable(         ${PROJECT_NAME} 
                        main.cpp    )

target_link_libraries(  ${PROJECT_NAME} 
                        ${CMAKE_CURRENT_SOURCE_DIR}../../ext/detours/detours.lib
                        Qt5::WebSockets     )
