project(RubicRecognition) # 设置项目名称
cmake_minimum_required(VERSION 2.8) # 设置所需要的cmake最小版本

#########################################
set(EXECUTABLE_OUTPUT_PATH ../bin) # 设置可执行文件目录 
set(LIBRARY_OUTPUT_PATH ../lib) # 设置生成的lib文件存放目录
set(CMAKE_BUILD_TYPE debug) # 设置编译类型为Debug
#add_definitions(-g -Wall -Werror) # 设置编译选项

## libs
link_directories(
/usr/lib
/usr/local/lib
${PROJECT_SOURCE_DIR}/lib
)

## includes
include_directories(
/usr/local/include
${PROJECT_SOURCE_DIR}/include
${PROJECT_SOURCE_DIR}/src
)

## src
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_DIR)

## apps
add_executable(RubicRecognition
	${SRC_DIR}
)

## link libs
target_link_libraries(RubicRecognition
opencv_core
opencv_imgproc
opencv_highgui
opencv_ml
opencv_video
opencv_features2d
opencv_calib3d
opencv_objdetect
opencv_contrib
opencv_legacy
opencv_flann
)
