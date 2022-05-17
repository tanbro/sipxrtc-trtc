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

option(
  CMAKE_FIND_GFLAGS
  "Whether to find gflags with cmake. If not, try to find it with pkg-config"
  OFF
)
set(
  CMAKE_FIND_GFLAGS_TARGET "" CACHE STRING
  "To request a particular imported gflags library target to link against. Only availvable when `CMAKE_FIND_GFLAGS` is ON."
)

option(
  CMAKE_FIND_GLOG
  "Whether to find glog with cmake. If not, try to find it with pkg-config"
  OFF
)

execute_process (
  COMMAND git describe
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_REV_PARSE_OUTPUT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 项目：这个执行文件
project(sipxtrtc)

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWERCASE)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} SYSTEM_PROCESSOR_LOWERCASE)
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_LOWERCASE)

# source 目录
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
# 源文件列表
set (SOURCES
  ${SRC_DIR}/main.cc
  ${SRC_DIR}/global.cc
  ${SRC_DIR}/RoomCallback.cc
  ${SRC_DIR}/AudioRecvCallback.cc
  ${SRC_DIR}/MixerCallback.cc
  ${SRC_DIR}/SdkLogger.cc
  ${SRC_DIR}/UdsBase.cc
  ${SRC_DIR}/UdsReader.cc
  ${SRC_DIR}/UdsWriter.cc
  ${SRC_DIR}/EventPub.cc
  ${SRC_DIR}/Flags.cc
  ${SRC_DIR}/utils.cc
  ${SRC_DIR}/version.cc
)

# 加构建目标: 可执行
add_executable(${PROJECT_NAME} ${SOURCES})

if (GIT_DESCRIBE_OUTPUT)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE __GIT_DESC__="${GIT_DESCRIBE_OUTPUT}"
  )
endif ()
if (GIT_REV_PARSE_OUTPUT)
  target_compile_definitions(${PROJECT_NAME}
  PRIVATE __GIT_REV__="${GIT_REV_PARSE_OUTPUT}"
  )
endif ()

# include 目录
target_include_directories(${PROJECT_NAME}
  PRIVATE ${SRC_DIR}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/private
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/public
)

# output dir, name, rpath, ...
set( OUTDIR ${CMAKE_CURRENT_LIST_DIR}/out/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE} )
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    BUILD_RPATH ../lib
    # 输出目录
    RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}/bin
)

# TRTC SDK
if (TRTC_SDK_DIR)
  message("TRTC_SDK_DIR was set explicitly: \"${TRTC_SDK_DIR}\"")
else ()
  if (DEFINED ENV{TRTC_SDK_DIR})
    set(TRTC_SDK_DIR $ENV{TRTC_SDK_DIR})
    message("TRTC_SDK_DIR was set from environment: \"${TRTC_SDK_DIR}\"")
  else ()
  set(TRTC_SDK_DIR ${CMAKE_CURRENT_LIST_DIR}/SDKs/trtc_7.9.54)
    message("Use default TRTC_SDK_DIR: \"${TRTC_SDK_DIR}\"")
  endif ()
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${TRTC_SDK_DIR}/include)
target_link_directories(${PROJECT_NAME} PRIVATE ${TRTC_SDK_DIR}/lib)
target_link_libraries(${PROJECT_NAME} TRTCEngine)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND mkdir -p ${OUTDIR}/lib && yes | cp -auv "${TRTC_SDK_DIR}/lib/libTRTCEngine.so" "${OUTDIR}/lib"
)

# TRTC SDK Requires pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)


# dependency of this project: gflags
if (${CMAKE_FIND_GFLAGS})
  if ("${CMAKE_FIND_GFLAGS_TARGET}" STREQUAL "")
    find_package(gflags REQUIRED)
  else ()
    find_package(gflags REQUIRED COMPONENTS "${CMAKE_FIND_GFLAGS_TARGET}")
  endif()
  target_link_libraries(${PROJECT_NAME} gflags)
else ()
  pkg_check_modules(GFLAGS REQUIRED gflags)
  target_link_libraries(${PROJECT_NAME} ${GFLAGS_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${GFLAGS_INCLUDE_DIRS})
  target_compile_options(${PROJECT_NAME} PRIVATE ${GFLAGS_CFLAGS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${GFLAGS_LIBRARY_DIRS})
  target_link_options(${PROJECT_NAME} PRIVATE ${GFLAGS_LDFLAGS})
endif()

# dependency of this project: glog
if (${CMAKE_FIND_GLOG})
  find_package (glog REQUIRED)
  target_link_libraries (${PROJECT_NAME} glog::glog)
else ()
  pkg_check_modules(LIBGLOG REQUIRED libglog)
  target_link_libraries(${PROJECT_NAME} ${LIBGLOG_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${LIBGLOG_INCLUDE_DIRS})
  target_compile_options(${PROJECT_NAME} PRIVATE ${LIBGLOG_CFLAGS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${LIBGLOG_LIBRARY_DIRS})
  target_link_options(${PROJECT_NAME} PRIVATE ${LIBGLOG_LDFLAGS})
endif()
