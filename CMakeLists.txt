# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: SonicDE

cmake_minimum_required(VERSION 3.21)

project(SonicVia VERSION 6.27.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# REQUIRED FOR TESTS: the unit-test .cpp files contain Q_OBJECT and end with
# `#include "test_*.moc"`. ECM does NOT enable AUTOMOC, so it must be set here or those moc
# includes are unresolved at compile time. Also needed for any QObject in examples.
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ECM discovery (provides all KDE/ECM modules used below).
include(FeatureSummary)
find_package(ECM 6.26.0 REQUIRED NO_MODULE)
set_package_properties(ECM PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Extra CMake Modules."
    URL "https://commits.kde.org/extra-cmake-modules")
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

# Clang-format setup, excluding the ccls index cache directory.
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
list(FILTER ALL_CLANG_FORMAT_SOURCE_FILES EXCLUDE REGEX ".*\\.ccls-cache/.*")
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

# Compiler settings MUST be included after the module path is set.
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(CMakePackageConfigHelpers)

# Qt6 — Gui is required because the public API uses QColor.
set(QT_MIN_VERSION 6.10.0)
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)

# Versioning: generates ${PROJECT_BINARY_DIR}/sonicvia_version.h and the config-version file,
# and defines SONICVIA_VERSION / SONICVIA_SOVERSION for use on the target.
ecm_setup_version(PROJECT
    VARIABLE_PREFIX SONICVIA
    VERSION_HEADER "${PROJECT_BINARY_DIR}/sonicvia_version.h"
    PACKAGE_VERSION_FILE "${PROJECT_BINARY_DIR}/SonicViaConfigVersion.cmake"
    SOVERSION 6
)

# Optional hidapi backend detection. The pkg-config module name varies by distro:
# Arch ships `hidapi-hidraw`, Debian ships `hidapi` (from libhidapi-dev), some ship
# `libhidapi-hidraw`. Probe each in turn until one is found.
#
# IMPORTANT (export-safety): we use the plain pkg-config VARIABLES (${hidapi_LIBRARIES} etc.) and
# NOT the `PkgConfig::hidapi` IMPORTED target. Linking an imported target and then install(EXPORT)-
# ing SonicVia can leak `PkgConfig::hidapi` into SonicViaTargets.cmake, breaking find_package(SonicVia)
# in the consumer ("Target PkgConfig::hidapi was not found"). Plain library names record cleanly in
# the export and the consumer never needs hidapi directly (it is fully encapsulated, PRIVATE).
set(HAVE_VIA_SUPPORT OFF)
find_package(PkgConfig)
if(PkgConfig_FOUND AND NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    pkg_check_modules(hidapi hidapi-hidraw)
    if(NOT hidapi_FOUND)
        pkg_check_modules(hidapi libhidapi-hidraw)
    endif()
    if(NOT hidapi_FOUND)
        pkg_check_modules(hidapi hidapi)
    endif()
    if(hidapi_FOUND)
        set(HAVE_VIA_SUPPORT ON)
    endif()
endif()

if(HAVE_VIA_SUPPORT)
    message(STATUS "SonicVia: building WITH VIA support (hidapi found: ${hidapi_MODULES}).")
else()
    message(STATUS "SonicVia: building STUB (no hidapi pkg-config module found or FreeBSD).")
endif()

add_subdirectory(src)
add_subdirectory(examples)

# CMake package config (installed to ${KDE_INSTALL_CMAKEPACKAGEDIR}/SonicVia).
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/SonicVia")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/SonicViaConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SonicViaConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SonicViaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SonicViaConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

# Install the generated version header at the include root so it is includable as
# <sonicvia_version.h> by consumers (the public headers include it that way).
install(FILES "${PROJECT_BINARY_DIR}/sonicvia_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
