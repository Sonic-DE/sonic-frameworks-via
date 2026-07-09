# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: SonicDE

set(sonicvia_LIB_SRCS
    via/viacommand.cpp
    via/viadevice.cpp
    via/viadiscovery.cpp
    via/viaprotocol.cpp
)

set(sonicvia_LIB_HDRS
    via/via.h
    via/viacommand.h
    via/viadevice.h
    via/viadiscovery.h
    via/viaprotocol.h
)

add_library(SonicVia SHARED ${sonicvia_LIB_SRCS} ${sonicvia_LIB_HDRS})
add_library(SonicVia::SonicVia ALIAS SonicVia)

# Proper soname (libSonicVia.so.6) — ecm_setup_version defines these variables.
set_target_properties(SonicVia PROPERTIES
    VERSION ${SONICVIA_VERSION}
    SOVERSION ${SONICVIA_SOVERSION}
    EXPORT_NAME SonicVia
)

# Generate the export header into the build tree as via/sonicvia_export.h.
ecm_generate_export_header(SonicVia
    BASE_NAME SONICVIA
    EXPORT_FILE_NAME via/sonicvia_export.h
    VERSION ${PROJECT_VERSION}
    DEPRECATED_BASE_VERSION 0
)

# Generate the logging-category header at the build root as sonicvia_debug.h so the sources'
# bare `#include "sonicvia_debug.h"` resolves via the ${CMAKE_CURRENT_BINARY_DIR} include path.
ecm_qt_declare_logging_category(SonicVia
    HEADER sonicvia_debug.h
    IDENTIFIER SONICVIA
    CATEGORY_NAME org.sonic.via
    DESCRIPTION "SonicVia VIA keyboard protocol library"
)

# Build-tree includes:
#   ${PROJECT_BINARY_DIR}            -> <sonicvia_version.h>
#   <src dir>                        -> <via/*.h> sources (headers live in src/via/)
#   <src binary dir>                 -> generated <via/sonicvia_export.h> and <sonicvia_debug.h>
target_include_directories(SonicVia
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(SonicVia
    PUBLIC
        Qt6::Core
        Qt6::Gui
)

if(HAVE_VIA_SUPPORT)
    # Link by plain library name (export-safe — see the detection comment above). The .so's NEEDED
    # entry records libhidapi; the consumer links only SonicVia and needs no hidapi on its path.
    target_link_libraries(SonicVia PRIVATE ${hidapi_LIBRARIES})
    target_include_directories(SonicVia PRIVATE ${hidapi_INCLUDE_DIRS})
    target_compile_options(SonicVia PRIVATE ${hidapi_CFLAGS_OTHER})
    target_compile_definitions(SonicVia PRIVATE HAVE_VIA_SUPPORT=1)
endif()

install(TARGETS SonicVia
    EXPORT SonicViaTargets
    ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(EXPORT SonicViaTargets
    FILE SonicViaTargets.cmake
    NAMESPACE SonicVia::
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/SonicVia
    COMPONENT Devel
)

install(FILES
    ${sonicvia_LIB_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/via/sonicvia_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/sonicvia_debug.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/via
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(../tests tests)
endif()
