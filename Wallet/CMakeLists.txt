# This file is part of Gram Wallet Desktop,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL

add_executable(Wallet WIN32 MACOSX_BUNDLE)
init_target(Wallet)

if (${CMAKE_GENERATOR} MATCHES "(Visual Studio|Xcode)")
    set(output_folder ${CMAKE_BINARY_DIR})
elseif (DESKTOP_APP_SPECIAL_TARGET STREQUAL "")
    set(output_folder ${CMAKE_BINARY_DIR}/bin)
else()
    set(output_folder ${CMAKE_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>)
endif()
set(update_packer_output_folder ${output_folder})

add_subdirectory(lib_rpl)
add_subdirectory(lib_crl)
add_subdirectory(lib_base)
add_subdirectory(lib_ui)
add_subdirectory(lib_tl)
add_subdirectory(lib_storage)
add_subdirectory(lib_lottie)
add_subdirectory(lib_qr)
add_subdirectory(lib_ton)
if (NOT disable_autoupdate)
    add_subdirectory(lib_updater)
endif()
add_subdirectory(lib_wallet)
add_subdirectory(codegen)

include(CheckCXXSourceCompiles)
include(lib_ui/cmake/generate_styles.cmake)

get_filename_component(src_loc SourceFiles REALPATH)
get_filename_component(res_loc Resources REALPATH)

include(cmake/wallet_options.cmake)

set(style_files
    wallet/wrapper.style
)

set(dependent_style_files
    ${submodules_loc}/lib_ui/ui/colors.palette
    ${submodules_loc}/lib_ui/ui/basic.style
    ${submodules_loc}/lib_ui/ui/layers/layers.style
    ${submodules_loc}/lib_ui/ui/widgets/widgets.style
)

generate_styles(Wallet ${src_loc} "${style_files}" "${dependent_style_files}")

set_target_properties(Wallet PROPERTIES AUTORCC ON)

target_link_libraries(Wallet
PRIVATE
    desktop-app::lib_crl
    desktop-app::lib_ui
    desktop-app::lib_tl
    desktop-app::lib_ton
    desktop-app::lib_storage
    desktop-app::lib_lottie
    desktop-app::lib_qr
    desktop-app::lib_wallet
    desktop-app::lib_base
    desktop-app::lib_base_crash_report_writer
    desktop-app::external_rlottie
    desktop-app::external_qt
    desktop-app::external_qr_code_generator
    desktop-app::external_crash_reports
    desktop-app::external_auto_updates
    desktop-app::external_openssl
)

if (NOT disable_autoupdate)
    target_link_libraries(Wallet
    PRIVATE
        desktop-app::lib_updater
    )
endif()

if (DESKTOP_APP_USE_PACKAGED)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads)

    target_link_libraries(Wallet
    PRIVATE
        ${CMAKE_DL_LIBS}
        Threads::Threads
    )
endif()

target_precompile_headers(Wallet PRIVATE ${src_loc}/core/pch.h)
nice_target_sources(Wallet ${src_loc}
PRIVATE
    ${style_files}

    core/base_integration.cpp
    core/base_integration.h
    core/launcher.cpp
    core/launcher.h
    core/main.cpp
    core/sandbox.cpp
    core/sandbox.h
    core/ui_integration.cpp
    core/ui_integration.h
    core/version.h
    wallet/application.cpp
    wallet/application.h
    wallet/config_upgrade_checker.cpp
    wallet/config_upgrade_checker.h
    wallet/phrases.cpp
    wallet/phrases.h
    wallet/ton_default_settings.cpp
    wallet/ton_default_settings.h
    wallet/update_info_provider.cpp
    wallet/update_info_provider.h
)

if (DESKTOP_APP_USE_PACKAGED)
    message(FATAL_ERROR "Packaged build is not supported yet.")
    nice_target_sources(Wallet ${src_loc} PRIVATE core/qt_functions.cpp)
else()
    nice_target_sources(Wallet ${src_loc} PRIVATE core/qt_static_plugins.cpp)
endif()

nice_target_sources(Wallet ${res_loc}
PRIVATE
    qrc/emoji_1.qrc
    qrc/emoji_2.qrc
    qrc/emoji_3.qrc
    qrc/emoji_4.qrc
    qrc/emoji_5.qrc
    qrc/config.qrc
    qrc/wrapper.qrc
    win/Wallet.rc
    win/Wallet.manifest
)

if (WIN32)
elseif (APPLE)
    set(icons_path ${res_loc}/art/Images.xcassets)
    set_target_properties(Wallet PROPERTIES RESOURCE ${icons_path})
    target_sources(Wallet PRIVATE ${icons_path})

    if (NOT build_macstore AND NOT DESKTOP_APP_DISABLE_CRASH_REPORTS)
        add_custom_command(TARGET Wallet
        PRE_LINK
            COMMAND mkdir -p $<TARGET_FILE_DIR:Wallet>/../Helpers
            COMMAND cp ${libs_loc}/crashpad/out/$<IF:$<CONFIG:Debug>,Debug,Release>/crashpad_handler $<TARGET_FILE_DIR:Wallet>/../Helpers/
        )
    endif()
elseif (LINUX)
    #if (NOT WALLET_DISABLE_GTK_INTEGRATION)
        find_package(PkgConfig REQUIRED)

        pkg_search_module(GTK REQUIRED gtk+-2.0 gtk+-3.0)
        target_include_directories(Wallet PRIVATE ${GTK_INCLUDE_DIRS})
        target_compile_options(Wallet PRIVATE -Wno-register)

        if (DESKTOP_APP_USE_PACKAGED)
            find_library(X11_LIBRARY X11)
            target_link_libraries(Wallet PRIVATE ${X11_LIBRARY})
        endif()
    #endif()
endif()

if (build_macstore)
    message(FATAL_ERROR "Mac App Store build is not supported.")
else()
    set(bundle_identifier "org.ton.wallet.desktop$<$<CONFIG:Debug>:.debug>")
    set(bundle_entitlements "Wallet.entitlements")
    if (LINUX AND DESKTOP_APP_USE_PACKAGED)
        set(output_name "wallet-desktop")
    else()
        set(output_name "Wallet")
    endif()
endif()

set_target_properties(Wallet PROPERTIES
    OUTPUT_NAME ${output_name}
    MACOSX_BUNDLE_GUI_IDENTIFIER ${bundle_identifier}
    MACOSX_BUNDLE_INFO_PLIST ${res_loc}/mac/Wallet.plist
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${res_loc}/mac/${bundle_entitlements}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${bundle_identifier}
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION ${desktop_app_version_string}
    XCODE_ATTRIBUTE_PRODUCT_NAME ${output_name}
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT $<$<NOT:$<CONFIG:Debug>>:dwarf-with-dsym>
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES YES
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP NO
    XCODE_ATTRIBUTE_ALWAYS_SEARCH_USER_PATHS NO
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++
    XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS --deep
)
set(entitlement_sources
    "${res_loc}/mac/Wallet.entitlements"
)
target_sources(Wallet PRIVATE ${entitlement_sources})
source_group(TREE ${res_loc} PREFIX Resources FILES ${entitlement_sources})

target_include_directories(Wallet PRIVATE ${src_loc})

set_target_properties(Wallet PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_folder})

if (NOT disable_autoupdate)
    add_dependencies(Wallet update_packer)
endif()
