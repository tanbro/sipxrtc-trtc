cmake_minimum_required(VERSION 3.15)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

# 导出 Clang compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# 全局语言标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")

# pkg-config 是必须的!
find_package(PkgConfig REQUIRED)

# 项目：这个执行文件
project(sipxtrtc)

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWERCASE)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} SYSTEM_PROCESSOR_LOWERCASE)
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_LOWERCASE)

# source 目录
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
# 源文件列表
set (SOURCES
  ${SRC_DIR}/main.cxx
)

# 加构建目标: 可执行
add_executable(${PROJECT_NAME} ${SOURCES})

# include 目录
target_include_directories(${PROJECT_NAME}
  PRIVATE ${SRC_DIR}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/private
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/public
)

# output dir, name, rpath, ...
set(OUTDIR ${CMAKE_CURRENT_LIST_DIR}/out/${CMAKE_BUILD_TYPE}/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID})
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    BUILD_RPATH ../lib
    # 输出目录
    RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}/bin
)

# TRTC SDK
set(TRTC_SDK_DIR ${CMAKE_CURRENT_LIST_DIR}/SDKs/trtc_7.9.54)
target_include_directories(${PROJECT_NAME} PRIVATE ${TRTC_SDK_DIR}/include)
target_link_directories(${PROJECT_NAME} PRIVATE ${TRTC_SDK_DIR}/lib)
target_link_libraries(${PROJECT_NAME} TRTCEngine)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -af "${TRTC_SDK_DIR}/lib/*" "${OUTDIR}/lib"
)
