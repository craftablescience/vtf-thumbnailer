cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# set up project
project(vtf-thumbnailer
        DESCRIPTION "Preview VTF files in your file explorer of choice."
        VERSION "1.1.1"
        HOMEPAGE_URL "https://github.com/craftablescience/vtf-thumbnailer")
set(PROJECT_NAME_PRETTY "VTF-Thumbnailer")
set(PROJECT_ORGANIZATION_NAME "craftablescience" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers")
include(CS_All)

# initial VS integration
cs_setup_vs_defaults()

# set common compile options
option(VTF_THUMBNAILER_USE_LTO "Compile ${PROJECT_NAME_PRETTY} with LTO/IPO" OFF)
cs_configure_defaults(VTF_THUMBNAILER_USE_LTO)

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

# Create thumbnailer
if(WIN32)
    # Exports
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/src/windows.def.in"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/windows.def")

    add_library(${PROJECT_NAME} SHARED
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/windows.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/windows.def")
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Shlwapi)
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
else()
    option(VTF_THUMBNAILER_BUILD_KDE5_PLUGIN "Build KDE5 thumbnailer plugin" OFF)
    if(VTF_THUMBNAILER_BUILD_KDE5_PLUGIN)
        set(QT_MIN_VERSION "5.15.2")
        set(KF_MIN_VERSION "5.92.0")
        set(KDE_COMPILERSETTINGS_LEVEL "5.82")
        find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
        set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

        include(ECMOptionalAddSubdirectory)
        include(KDEInstallDirs${QT_MAJOR_VERSION})
        include(KDECMakeSettings)
        include(KDECompilerSettings NO_POLICY_SCOPE)
        include(FeatureSummary)
        include(ECMDeprecationSettings)

        find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui)
        find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS KIO)

        ecm_set_disabled_deprecation_versions(QT 5.15.2 KF 5.100.0)
		set(BUILD_SHARED_LIBS ON)
        kcoreaddons_add_plugin(${PROJECT_NAME} INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/${PROJECT_NAME}")
        target_sources(${PROJECT_NAME} PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/kde.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/kde.h")
        target_link_libraries(${PROJECT_NAME} PUBLIC
                KF${QT_MAJOR_VERSION}::KIOGui
                Qt::Gui)
        target_compile_options(${PROJECT_NAME} PUBLIC "-fexceptions")
        install(TARGETS ${PROJECT_NAME}
                DESTINATION ${KDE_INSTALL_PLUGINDIR})

        # Desktop
        if(QT_MAJOR_VERSION STREQUAL "6")
            set(KDE_INSTALL_KSERVICESDIR "${KDE_INSTALL_DATADIR}/kio")
        endif()
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/desktop.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/${PROJECT_NAME}.desktop")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/${PROJECT_NAME}.desktop"
                DESTINATION ${KDE_INSTALL_KSERVICESDIR})

        # Metadata
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/metainfo.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/info.craftablescience.${PROJECT_NAME}.metainfo.xml")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/info.craftablescience.${PROJECT_NAME}.metainfo.xml"
                DESTINATION ${KDE_INSTALL_METAINFODIR})

        # MIME type info
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/mime-type.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/mime-type.xml")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/mime-type.xml"
                DESTINATION ${KDE_INSTALL_MIMEDIR}
                RENAME "${PROJECT_NAME}-kde${QT_MAJOR_VERSION}.xml")

        # Plugin data
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/plugin.json.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/kde/generated/plugin.json")
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
                DESTINATION "/usr/share/mime/packages"
                RENAME "${PROJECT_NAME}.xml")

        # Thumbnailer
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/thumbnailer.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/generated/${PROJECT_NAME}.thumbnailer")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/generated/${PROJECT_NAME}.thumbnailer"
                DESTINATION "/usr/share/thumbnailers")
    endif()
endif()
cs_configure_target(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE sourcepp::vtfpp)

# CPack stuff
if(NOT WIN32 AND VTF_THUMBNAILER_BUILD_KDE5_PLUGIN)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-kde${QT_MAJOR_VERSION}")
else()
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
endif()
set(CPACK_PACKAGE_VENDOR "craftablescience")
set(CPACK_PACKAGE_CONTACT "lauralewisdev@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_DESCRIPTION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_NAME_PRETTY}")
set(CPACK_STRIP_FILES ON)
set(CPACK_THREADS 0)
set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(WIN32)
    if(NOT (CPACK_GENERATOR STREQUAL "NSIS"))
        message(WARNING "CPACK_GENERATOR on Windows must be NSIS! Setting generator to NSIS...")
        set(CPACK_GENERATOR "NSIS" CACHE INTERNAL "" FORCE)
    endif()
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME_PRETTY}")
    set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME_PRETTY}")
    set(CPACK_NSIS_URL_INFO_ABOUT "${CMAKE_PROJECT_HOMEPAGE_URL}")
    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait 'regsvr32 /s \\\"$INSTDIR\\\\${PROJECT_NAME}.dll\\\"'")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "ExecWait 'regsvr32 /u /s \\\"$INSTDIR\\\\${PROJECT_NAME}.dll\\\"'")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_PRETTY}-${PROJECT_VERSION}-Windows")
else()
    if(VTF_THUMBNAILER_BUILD_KDE5_PLUGIN)
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_PRETTY}-${PROJECT_VERSION}-KDE${QT_MAJOR_VERSION}")
        set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    else()
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_PRETTY}-${PROJECT_VERSION}-Linux")
        set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${PROJECT_NAME}")
    endif()
    if(CPACK_GENERATOR STREQUAL "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
        set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
    elseif(CPACK_GENERATOR STREQUAL "RPM")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        if(CMAKE_VERSION VERSION_LESS "3.31")
            set(CPACK_RPM_COMPRESSION_TYPE "xz")
        else()
            set(CPACK_RPM_COMPRESSION_TYPE "zstd")
        endif()
    else()
        message(FATAL_ERROR "CPACK_GENERATOR is unset! Only DEB and RPM generators are supported.")
    endif()
endif()
include(CPack)
