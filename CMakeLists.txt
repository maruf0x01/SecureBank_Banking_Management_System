cmake_minimum_required(VERSION 3.16)
project(SecureBank LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(CORE_SOURCES src/Account.cpp src/FileManager.cpp)

# Console version
add_executable(securebank src/main.cpp src/BankSystem.cpp ${CORE_SOURCES})
target_include_directories(securebank PRIVATE src)

# Qt6 GUI version
find_package(Qt6 COMPONENTS Widgets)
if(Qt6_FOUND)
    add_executable(securebank_gui
        src/gui/main_gui.cpp
        src/gui/LoginDialog.cpp
        src/gui/BankService.cpp
        src/gui/Dialogs.cpp
        src/gui/AdminWindow.cpp
        src/gui/CustomerWindow.cpp
        ${CORE_SOURCES})
    target_include_directories(securebank_gui PRIVATE src src/gui)
    target_link_libraries(securebank_gui PRIVATE Qt6::Widgets)
else()
    message(STATUS "Qt6 not found - GUI target skipped")
endif()
