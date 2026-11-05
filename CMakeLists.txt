cmake_minimum_required(VERSION 3.21)

project(MarhabanConverter
    VERSION 2.2.0
    DESCRIPTION "Marhaban Studio Desktop Media Converter"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_PREFIX_PATH "C:/Qt/6.12.0/mingw_64")

find_package(Qt6 6.12 REQUIRED COMPONENTS Widgets Svg)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(MarhabanConverter
    app_icon.rc
    resources.qrc
    src/main.cpp

    src/app_context.h
    src/app_context.cpp

    src/ui/main_window.h
    src/ui/main_window.cpp
    
    src/ui/drop_list.h
    src/ui/drop_list.cpp
    src/ui/audio_page.h
    src/ui/audio_page.cpp
    src/ui/video_page.h
    src/ui/video_page.cpp
    src/ui/photo_page.h
    src/ui/photo_page.cpp
    src/ui/compress_page.h
    src/ui/compress_page.cpp
    src/ui/calculator_page.h
    src/ui/calculator_page.cpp
    src/ui/calendar_page.h
    src/ui/calendar_page.cpp

    src/ui/dialogs/feedback_dialog.h
    src/ui/dialogs/feedback_dialog.cpp

    src/services/aae_service.h
    src/services/aae_service.cpp
    src/services/audio_converter_service.h
    src/services/audio_converter_service.cpp
    src/services/device_service.h
    src/services/device_service.cpp
    src/services/format_registry.h
    src/services/format_registry.cpp
    src/services/image_compression_service.h
    src/services/image_compression_service.cpp
    src/services/image_converter_service.h
    src/services/image_converter_service.cpp
    src/services/settings_service.h
    src/services/settings_service.cpp
    src/services/svg_converter_service.h
    src/services/svg_converter_service.cpp
    src/services/video_compression_service.h
    src/services/video_compression_service.cpp
    src/services/video_converter_service.h
    src/services/video_converter_service.cpp
    src/services/tabular_hijri_calendar.h
    src/services/tabular_hijri_calendar.cpp
    src/services/civil_calendar.h
    src/services/civil_calendar.cpp
    src/services/calendar_converter_service.h
    src/services/calendar_converter_service.cpp


    src/themes/theme_manager.h
    src/themes/theme_manager.cpp

    src/utils/app_paths.h
    src/utils/app_paths.cpp

    src/workers/conversion_worker.h
    src/workers/conversion_worker.cpp
)

set_target_properties(MarhabanConverter PROPERTIES
    VERSION ${PROJECT_VERSION}
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(MarhabanConverter PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(MarhabanConverter
    PRIVATE
        Qt6::Widgets
        Qt6::Svg
)



add_executable(CalendarTests
    tests/calendar_test.cpp

    src/services/tabular_hijri_calendar.h
    src/services/tabular_hijri_calendar.cpp

    src/services/civil_calendar.h
    src/services/civil_calendar.cpp

    src/services/calendar_converter_service.h
    src/services/calendar_converter_service.cpp
)

target_include_directories(CalendarTests
    PRIVATE
        src/services
)

target_link_libraries(CalendarTests
    PRIVATE
        Qt6::Core
)
