cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# Create project
project(vtf-thumbnailer
        DESCRIPTION "Preview VTF files in your file explorer of choice."
        VERSION "0.1.0"
        HOMEPAGE_URL "https://github.com/craftablescience/vtf-thumbnailer")
set(PROJECT_NAME_PRETTY "VTF-Thumbnailer")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include CMake libraries
include(CheckIPOSupported)
include(GNUInstallDirs)

# Set proper runpath
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH $ORIGIN)

# Compile with PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compile with LTO if supported
set(VTF_THUMBNAILER_USE_LTO ON)
if(VTF_THUMBNAILER_USE_LTO)
    check_ipo_supported(RESULT VTF_THUMBNAILER_USE_LTO)
endif()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${VTF_THUMBNAILER_USE_LTO})

# Set default install directory permissions
set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# Macro to set up many things at once for a given target
function(vtf_thumbnailer_configure_target TARGET)
    # Define DEBUG macro
    target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

    # Set optimization flags
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        # Build with debug friendly optimizations and debug symbols (MSVC defaults are fine)
        if(UNIX OR MINGW)
            target_compile_options(${TARGET} PRIVATE -Og -g)
        endif()
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC)
            target_compile_options(${TARGET} PRIVATE -fno-limit-debug-info)
        endif()
    else()
        # Build with optimizations and don't omit stack pointer for debugging (MSVC defaults are fine)
        # Also enable exceptions forcibly because KDE is trying to turn them off???
        if(UNIX OR MINGW)
            target_compile_options(${TARGET} PRIVATE -O2 -fno-omit-frame-pointer -fexceptions)
        endif()
    endif()

    if(WIN32 AND MSVC)
        # Create PDBs in release
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        if((TARGET_TYPE STREQUAL "SHARED_LIBRARY") OR (TARGET_TYPE STREQUAL "EXECUTABLE"))
            target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:/Zi>")
            target_link_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:/DEBUG>" "$<$<CONFIG:Release>:/OPT:REF>" "$<$<CONFIG:Release>:/OPT:ICF>")
        endif()
	endif()
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC)
		target_compile_options(${TARGET} PRIVATE -fexceptions)
    endif()
endfunction()

# argparse
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/argparse")

# sourcepp
set(SOURCEPP_LIBS_START_ENABLED OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_VTFPP           ON CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/sourcepp")

# Configure header
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h")

# Create executable
if(WIN32)
    add_executable(${PROJECT_NAME}
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/windows.cpp")
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
else()
    option(VTF_THUMBNAILER_BUILD_KDE_PLUGIN "Build KDE thumbnailer plugin" OFF)
    if(VTF_THUMBNAILER_BUILD_KDE_PLUGIN)
        set(QT_MIN_VERSION "5.15.2")
        set(KF_MIN_VERSION "5.92.0")
        set(KDE_COMPILERSETTINGS_LEVEL "5.82")
        find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
        set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

        include(ECMOptionalAddSubdirectory)
        include(KDEInstallDirs)
        include(KDECMakeSettings)
        include(KDECompilerSettings NO_POLICY_SCOPE)
        include(FeatureSummary)
        include(ECMDeprecationSettings)

        find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui)
        find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS Archive KIO)

        ecm_set_disabled_deprecation_versions(QT 5.15.2 KF 5.100.0)
		set(BUILD_SHARED_LIBS ON)
        kcoreaddons_add_plugin(${PROJECT_NAME} INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/${PROJECT_NAME}")
        target_sources(${PROJECT_NAME} PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/kde.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/kde.h")
        target_include_directories(${PROJECT_NAME} PUBLIC
                ${KF5CoreAddons_INCLUDE_DIRS}
                ${KIO_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC
                KF${QT_MAJOR_VERSION}::KIOGui
                KF${QT_MAJOR_VERSION}::Archive
                Qt::Core)

        # Plugin data
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/plugin.json.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/plugin.json")

        # Metadata
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/metainfo.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/info.craftablescience.${PROJECT_NAME}.metainfo.xml")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/info.craftablescience.${PROJECT_NAME}.metainfo.xml"
                DESTINATION ${KDE_INSTALL_METAINFODIR})
    else()
        add_executable(${PROJECT_NAME}
                "${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/linux.cpp")
        target_link_libraries(${PROJECT_NAME} PUBLIC argparse::argparse)
        install(TARGETS ${PROJECT_NAME} DESTINATION .)

        # MIME type info
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/mime-type.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/generated/mime-type.xml")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/generated/mime-type.xml"
                DESTINATION "/usr/share/mime/packages/"
                RENAME "${PROJECT_NAME}.xml")

        # Thumbnailer
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/thumbnailer.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/generated/${PROJECT_NAME}.thumbnailer")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/generated/${PROJECT_NAME}.thumbnailer"
                DESTINATION "/usr/share/thumbnailers")
    endif()
endif()
vtf_thumbnailer_configure_target(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC sourcepp::vtfpp)

# CPack stuff
set(CPACK_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
set(CPACK_PACKAGE_VENDOR "craftablescience")
set(CPACK_PACKAGE_CONTACT "lauralewisdev@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME_PRETTY})
if(WIN32)
    if(NOT (CPACK_GENERATOR STREQUAL "NSIS"))
        message(WARNING "CPack generator must be NSIS! Setting generator to NSIS...")
        set(CPACK_GENERATOR "NSIS" CACHE INTERNAL "" FORCE)
    endif()
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME_PRETTY})
    set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
    set(CPACK_NSIS_URL_INFO_ABOUT "${CMAKE_PROJECT_HOMEPAGE_URL}")
    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_PRETTY}-${PROJECT_VERSION}-Windows")
else()
    if(NOT (CPACK_GENERATOR STREQUAL "DEB"))
        message(WARNING "CPack generator must be DEB! Setting generator to DEB...")
        set(CPACK_GENERATOR "DEB" CACHE INTERNAL "" FORCE)
    endif()
    set(CPACK_STRIP_FILES ON)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${PROJECT_NAME}")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
    if(VTF_THUMBNAILER_BUILD_KDE_PLUGIN)
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_PRETTY}-${PROJECT_VERSION}-KDE")
    else()
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_PRETTY}-${PROJECT_VERSION}-Linux")
    endif()
endif()
include(CPack)
