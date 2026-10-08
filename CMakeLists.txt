cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

#!！！！！！!
#将此设置改为 ON 而非 OFF 以发布游戏！
#这基本上会将RESOURCES_PATH更改为本地路径
#!！！！！！!
#如果 Visual Studio 无法识别更改，请在修改后删除 out 文件夹
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)

# raylib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as static libraries" FORCE)

project(mygame)

# 编译器类型只有 project() 之后才会确定，放在前面会让 MSVC 分支永远不生效
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
	# 源码是 UTF-8 保存的，必须显式告诉 MSVC，否则在中文系统上会按 GBK(936) 解析
	# 导致 C4819 警告，以及注释末尾字节吃掉换行、把下一行代码注释掉的诡异报错
	add_compile_options(/utf-8)
else()
	add_compile_options(-mavx2 -mfma) # make sure SIMD optimizations take place, but on linux and other platforms
endif()



# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
# 添加可执行文件
add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)



if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


# 生产版本中不要创建控制台窗口
if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	if(PRODUCTION_BUILD)
		#YOU CAN REMOVE THE CONSOLE WITH THIS LINE!
		set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	endif()

elseif(MINGW) # Windows / MinGW GCC/Clang
	if(PRODUCTION_BUILD)
		# Remove console on Windows under MinGW
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE
			-mwindows
		)
	endif()

elseif(APPLE OR UNIX) # macOS / Linux
	if(PRODUCTION_BUILD)
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -s)
	endif()
endif()


target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/gamelayer")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/platform")

# 把 raylib 源码目录加入构建
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third-party/raylib-6.0")
# imgui
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui-docking")
# rllimgui
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third-party/rlimgui")


# link raylib  imgui
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE raylib_static imgui rlimgui)


