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

# Ignore safe exceptions linkage from old detours lib. It doesn't throw.
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
SET(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /SAFESEH:NO")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /SAFESEH:NO")
SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
SET(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} /SAFESEH:NO")
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
SET(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /SAFESEH:NO")
SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
SET(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "${CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL} /SAFESEH:NO")
SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
SET(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} /SAFESEH:NO")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5WebSockets)

SET (LIB_TYPE SHARED)

include_directories(    $ENV{MGEN_INSTALL_PATH}/include
                        $ENV{MNET_INSTALL_PATH}/include
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/detours
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/turbojpeg
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../../gpt-common/src_generated/main/cpp   )

add_library(            ${PROJECT_NAME} ${LIB_TYPE}
                        Distributor.cpp
                        ID3D9Wrapper.cpp
                        ID3D9Wrapper.h
                        ID3D9Wrapper_Device.cpp
                        ID3D9Wrapper_Device.h
                        Logger.cpp
                        main.cpp
                        stdafx.h     )

target_link_libraries(  ${PROJECT_NAME} 
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/detours/detours.lib
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/turbojpeg/jpeg.lib
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/turbojpeg/turbojpeg.lib
                        Qt5::WebSockets     )
