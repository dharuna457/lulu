cmake_minimum_required(VERSION 3.16)

project(LinuxDroid VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# Source files for main application
set(MAIN_SOURCES
    src/main.cpp
    src/core/qemu_manager.cpp
    src/core/vm_config.cpp
    src/core/download_manager.cpp
    src/utils/system_checker.cpp
    src/gui/main_window.cpp
    src/gui/setup_wizard.cpp
)

set(MAIN_HEADERS
    src/core/qemu_manager.h
    src/core/vm_config.h
    src/core/download_manager.h
    src/utils/system_checker.h
    src/gui/main_window.h
    src/gui/setup_wizard.h
)

# Source files for daemon
set(DAEMON_SOURCES
    src/daemon.cpp
    src/core/download_manager.cpp
)

set(DAEMON_HEADERS
    src/core/download_manager.h
)

# Main application executable
add_executable(linuxdroid ${MAIN_SOURCES} ${MAIN_HEADERS})
target_link_libraries(linuxdroid
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Daemon executable
add_executable(linuxdroid-daemon ${DAEMON_SOURCES} ${DAEMON_HEADERS})
target_link_libraries(linuxdroid-daemon
    Qt6::Core
    Qt6::Network
)

# Enable warnings
target_compile_options(linuxdroid PRIVATE -Wall -Wextra)
target_compile_options(linuxdroid-daemon PRIVATE -Wall -Wextra)

# Install targets
install(TARGETS linuxdroid linuxdroid-daemon
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

# Install desktop file
install(FILES debian/linuxdroid.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
)

# Install systemd service
install(FILES debian/linuxdroid-download.service
    DESTINATION /lib/systemd/system
)

# Install icon (create placeholder)
install(FILES resources/icons/linuxdroid.png
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/512x512/apps
)

# Install resources
install(FILES resources/android_images.json
    DESTINATION /opt/linuxdroid
)

# Create directories during install
install(DIRECTORY DESTINATION /opt/linuxdroid/images)
install(DIRECTORY DESTINATION /opt/linuxdroid/instances)
install(DIRECTORY DESTINATION /var/log/linuxdroid)

# CPack configuration for DEB package
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "linuxdroid")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "Dharun Ashokkumar <contact@tripletech.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional Android Emulator for Linux")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qemu-system-x86, libqt6core6, libqt6gui6, libqt6widgets6, libqt6network6, libvirt0, libgl1, libsdl2-2.0-0, curl, wget, systemd")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "cpu-checker, virt-manager")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/debian/postinst"
    "${CMAKE_SOURCE_DIR}/debian/postrm"
)

include(CPack)
