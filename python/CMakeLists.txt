cmake_minimum_required(VERSION 3.13)
project(pypag_bindings)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 优先使用指定的 Python 可执行文件
if(Python_EXECUTABLE)
    set(Python_ROOT_DIR "${Python_EXECUTABLE}")
endif()

# 查找 Python - 使用虚拟环境中的版本
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# 查找 pybind11
find_package(pybind11 CONFIG)
if(NOT pybind11_FOUND)
    # 如果系统中没有 pybind11，从 GitHub 下载
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# 禁用不需要的选项
set(PAG_BUILD_FRAMEWORK OFF CACHE BOOL "" FORCE)
set(PAG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PAG_USE_SWIFTSHADER OFF CACHE BOOL "" FORCE)

# 添加父目录的 CMakeLists.txt 以构建 libpag
add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/libpag EXCLUDE_FROM_ALL)

# 创建 Python 模块（简化版本，先验证能编译）
pybind11_add_module(pypag MODULE
    src/pypag_simple.cpp
    src/bindings/pag_file_simple.cpp
    src/bindings/pag_surface.cpp
    src/bindings/pag_player.cpp
    src/bindings/pag_image.cpp
)

# 链接 libpag
target_link_libraries(pypag PRIVATE pag)

# 包含头文件
target_include_directories(pypag PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 设置输出名称和位置
set_target_properties(pypag PROPERTIES
    PREFIX ""
    OUTPUT_NAME "pypag"
    SUFFIX "${Python_SOABI}${CMAKE_SHARED_MODULE_SUFFIX}"
)
