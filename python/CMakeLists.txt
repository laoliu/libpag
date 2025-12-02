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

# 选项:使用预编译的 libpag 或重新编译
option(USE_PREBUILT_LIBPAG "Use prebuilt libpag library" ON)

if(USE_PREBUILT_LIBPAG)
    # 使用预编译的 libpag
    set(LIBPAG_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release" CACHE PATH "Path to prebuilt libpag")
    
    # 创建导入的库目标
    add_library(pag SHARED IMPORTED)
    set_target_properties(pag PROPERTIES
        IMPORTED_LOCATION "${LIBPAG_BUILD_DIR}/libpag.dll"
        IMPORTED_IMPLIB "${LIBPAG_BUILD_DIR}/libpag.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    )
    
    # 检查文件是否存在
    if(NOT EXISTS "${LIBPAG_BUILD_DIR}/libpag.lib")
        message(FATAL_ERROR "Prebuilt libpag not found at: ${LIBPAG_BUILD_DIR}/libpag.lib")
    endif()
    
    message(STATUS "Using prebuilt libpag from: ${LIBPAG_BUILD_DIR}")
else()
    # 添加父目录的 CMakeLists.txt 以构建 libpag
    add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/libpag EXCLUDE_FROM_ALL)
endif()

# 创建 Python 模块（简化版本，先验证能编译）
pybind11_add_module(pypag MODULE
    src/pypag_simple.cpp
    src/bindings/pag_file_simple.cpp
    src/bindings/pag_surface.cpp
    src/bindings/pag_player.cpp
    src/bindings/pag_image.cpp
)

# Windows MSVC: 处理 Debug/Release Python 库问题
if(WIN32 AND MSVC)
    # 标准 Python 安装通常没有 Debug 版本库
    # 在 Debug 配置下也使用 Release Python 库
    target_compile_definitions(pypag PRIVATE 
        $<$<CONFIG:Debug>:NDEBUG>           # Debug 配置时定义 NDEBUG
    )
    target_compile_options(pypag PRIVATE 
        $<$<CONFIG:Debug>:/U_DEBUG>         # Debug 配置时取消 _DEBUG
    )
endif()

# 链接 libpag
target_link_libraries(pypag PRIVATE pag)
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

# 安装规则
install(TARGETS pypag
    LIBRARY DESTINATION "${Python_SITELIB}"
    RUNTIME DESTINATION "${Python_SITELIB}"
)

# 也安装 libpag.dll (如果使用预编译版本)
if(USE_PREBUILT_LIBPAG AND WIN32)
    install(FILES "${LIBPAG_BUILD_DIR}/libpag.dll"
        DESTINATION "${Python_SITELIB}"
    )
endif()
