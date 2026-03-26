cmake_minimum_required(VERSION 3.19)
project(motion VERSION 1.0.0 LANGUAGES CXX)

# 查找 Qt6 Core 组件，启用自动 moc（因类使用了 Q_OBJECT）
find_package(Qt6 6.2 REQUIRED COMPONENTS Core)
qt_standard_project_setup()   # 启用 AUTOMOC、AUTOUIC、AUTORCC

# 显式列出源文件，这是开发规范
# 列出头文件，保证MOC能够启动
set(SOURCES 
    src/motion_controller.cpp   
    src/motion_manager.cpp 
    src/motion_solver.cpp 
    src/motion_worker.cpp 
    src/sample_worker.cpp
)

set(HEADERS 
    include/motion_controller.h   
    include/motion_manager.h 
    include/motion_solver.h 
    include/motion_worker.h
    include/sample_worker.h
    include/motion_manager_private.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})   # 定义动态库

target_include_directories(${PROJECT_NAME}					# 收集源文件
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/LTSMC/include
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 第三方库 LTSMC 的路径设置
set(LTSMC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/LTSMC/lib)
set(LTSMC_IMPLIB ${LTSMC_LIB_DIR}/libLTSMC.a)    # 静态导入库（或动态库的导入库）
set(LTSMC_DLL ${LTSMC_LIB_DIR}/LTSMC.dll)        # 运行时 DLL

# 检查必要的库文件是否存在，提前暴露问题
if(NOT EXISTS ${LTSMC_IMPLIB})
    message(FATAL_ERROR "LTSMC import library not found: ${LTSMC_IMPLIB}")
endif()
if(NOT EXISTS ${LTSMC_DLL})
    message(WARNING "LTSMC DLL not found: ${LTSMC_DLL}, runtime may fail")
endif()

# 链接依赖（私有）：第三方库和 QtCore
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LTSMC_IMPLIB}
    Qt::Core
)


target_compile_definitions(${PROJECT_NAME} PRIVATE MOTION_LIBRARY)   #编译动态库

# 将第三方 DLL 复制到输出目录，便于运行时查找（构建后自动执行）
add_custom_command(TARGET motion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LTSMC_DLL}"
        "$<TARGET_FILE_DIR:motion>"
    COMMENT "Copying LTSMC.dll to output directory"
)

# 注意：本项目输出目录由根目录 CMakeLists.txt 统一管理，此处不再单独设置。
# 如需独立调整输出位置，可在根目录中设置 CMAKE_RUNTIME_OUTPUT_DIRECTORY 等变量。