cmake_minimum_required(VERSION 3.10)

# --- 工具链配置 ---
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR nds32)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "^(Windows|CYGWIN|MSYS|MINGW)")
  set(TOOLCHAIN "C:/Andestech/AndeSight_RDS_v323/toolchains")# ←←← 这里改成你的 windows下 andes 工具链路径
  set(TOOLCHAIN_SUFFIX ".exe")
else()
  set(TOOLCHAIN "/Andes/toolchains")   # ←←← 这里改成你的 Linux 下 andes 工具链路径
  set(TOOLCHAIN_SUFFIX "")
endif()

set(CROSS_COMPILE "${TOOLCHAIN}/nds32le-elf-mculib-v3s/bin/nds32le-elf-")

set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc${TOOLCHAIN_SUFFIX})
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++${TOOLCHAIN_SUFFIX})
set(CMAKE_AR           ${CROSS_COMPILE}ar${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJCOPY      ${CROSS_COMPILE}objcopy${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJDUMP      ${CROSS_COMPILE}objdump${TOOLCHAIN_SUFFIX})
set(CMAKE_READELF      ${CROSS_COMPILE}readelf${TOOLCHAIN_SUFFIX})
set(CMAKE_NM      ${CROSS_COMPILE}nm${TOOLCHAIN_SUFFIX})

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# 【关键修复1】：必须启用 ASM，否则 .S 文件（包括 crt0.S）无法被正确编译成 .o
project(roboeffect_demo C ASM)

# --- 编译选项 ---
add_compile_options(-Og -g3 -mcmodel=medium -W -mcpu=d1088-spu -c -fmessage-length=0 -std=gnu99 -ffunction-sections -fdata-sections -mext-dsp -mext-zol -MMD -MP)

# --- 目录配置 ---
set(SRC_DIRS 
    "main" 
    "driver/driver_api/src" 
    "driver/driver_api/src/otg/device" 
    "middleware/audio/src" 
    "middleware/mv_utils/src" 
    "startup" 
    "middleware/roboeffect/roboeffect_lib" 
    "middleware/roboeffect/roboeffect_lib/wraps" 
    "middleware/roboeffect/graphics" "middleware/fatfs/src"
    "middleware/audio_core"
)

set(INC_DIRS 
    "main" 
    "user" 
    "middleware/audio/inc" 
    "driver/driver/inc" 
    "driver/driver_api/inc" 
    "middleware/mv_utils/inc" 
    "driver/driver_api/inc/otg" 
    "middleware/roboeffect/roboeffect_lib" 
    "middleware/roboeffect/roboeffect_lib/wraps" 
    "middleware/roboeffect/graphics" 
    "middleware/fatfs/inc" 
    "middleware/roboeffect/third_party_effect"
    "middleware/audio_core"
)

# --- 1. 搜寻手动指定的源码 ---
set(ALL_SRCS "")
foreach(dir ${SRC_DIRS})
    file(GLOB_RECURSE DIR_SRCS 
        "${dir}/*.c" 
        "${dir}/*.S"
    )
    list(APPEND ALL_SRCS ${DIR_SRCS})
endforeach()

# --- 2. 递归追加 third_party_effect 源码 ---
file(GLOB_RECURSE REC_SRCS 
    "middleware/roboeffect/third_party_effect/*.c" 
)

# --- 3. 【关键：正确合并】 ---
set(FINAL_COMPILATION_SRCS ${ALL_SRCS} ${REC_SRCS})

# --- 4. 递归处理头文件 ---
file(GLOB_RECURSE REC_HEADERS "middleware/roboeffect/third_party_effect/*.h")
set(REC_INC_DIRS "")
foreach(_headerFile ${REC_HEADERS})
    get_filename_component(_dir ${_headerFile} DIRECTORY)
    list(APPEND REC_INC_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES REC_INC_DIRS)
list(APPEND INC_DIRS ${REC_INC_DIRS})

# --- 链接配置 ---
get_filename_component(ABS_LD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/nds32-ae210p.ld" ABSOLUTE)

add_link_options(
    -Og -nostartfiles -static -mcmodel=medium -flto -g3 
    -mcpu=d1088-spu -ffunction-sections -fdata-sections -fmessage-length=0
    "-T${ABS_LD_PATH}"
    -Wl,--gc-sections -mvh -mext-dsp -ldsp  
)

# 1. 定义启动文件路径（建议用绝对路径确保万无一失）
set(STARTUP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/startup/crt0.S")

# --- 构建目标 ---
# 使用合并后的 FINAL_COMPILATION_SRCS + 显式启动文件（确保 crt0 一定在链接列表中，且只出现一次）
add_executable(${PROJECT_NAME}.adx ${STARTUP_FILE} ${FINAL_COMPILATION_SRCS})

target_include_directories(${PROJECT_NAME}.adx PUBLIC ${INC_DIRS})

# --- 自动递归搜寻 roboeffect 下所有包含静态库的文件夹 ---
file(GLOB_RECURSE ALL_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/middleware/roboeffect/*.a")
set(ROBOEFFECT_LINK_DIRS "")
foreach(_libFile ${ALL_LIBS})
    get_filename_component(_dir ${_libFile} DIRECTORY)
    list(APPEND ROBOEFFECT_LINK_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ROBOEFFECT_LINK_DIRS)


target_link_directories(${PROJECT_NAME}.adx PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/audio"
    "${CMAKE_CURRENT_SOURCE_DIR}/driver/driver"
    ${ROBOEFFECT_LINK_DIRS}# ←←← 这里直接代入动态搜寻到的所有库路径
)

# 注意：dsp 和 m 库通常放在最后
target_link_libraries(${PROJECT_NAME}.adx PRIVATE 
    Driver 
    Roboeffect 
    AudioDecoderLibrary 
    AudioEffectLibrary 
    VocalCut 
    Beamforming
    dsp 
    m
)

# --- 后期处理：生成 bin 和 disasm ---
add_custom_command(TARGET ${PROJECT_NAME}.adx POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -S -O binary ${PROJECT_NAME}.adx ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJDUMP} -x -d -C ${PROJECT_NAME}.adx > objdump.txt
    COMMAND ${CMAKE_NM} -n -l -C ${PROJECT_NAME}.adx > symbol.txt
    COMMENT "Generating .adx .bin and symbol files..."
)

set_directory_properties(PROPERTIES 
    ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.bin;objdump.txt;symbol.txt"
)
