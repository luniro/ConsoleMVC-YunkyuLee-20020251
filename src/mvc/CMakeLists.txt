add_library(mvc STATIC
    model/TaskModel.cpp
    view/ConsoleView.cpp
    controller/TaskController.cpp
    App.cpp
)

# PUBLIC: 링크하는 쪽에서 #include "mvc/..." 형태로 접근 가능
target_include_directories(mvc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_compile_features(mvc PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(mvc PRIVATE /W4 /utf-8)
else()
    target_compile_options(mvc PRIVATE -Wall -Wextra)
endif()
