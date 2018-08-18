#1.cmake verson，指定cmake版本 
cmake_minimum_required(VERSION 3.3)

#2.设置项目名称
SET(PROJECT_NAME aprilTag)

# 设置用debug还是release模式。debug允许断点，而release更快
#set( CMAKE_BUILD_TYPE Debug )
set( CMAKE_BUILD_TYPE Release )

# 设置编译选项
# 允许c++11标准、O3优化、多线程。match选项可避免一些cpu上的问题
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")

# set output binary path  
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)  


#设定存放编译出来的库文件的目录
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib) 

#2.project name，指定项目的名称，一般和项目的文件夹名称对应
PROJECT(${PROJECT_NAME})

#set AprilTag
SET(AprilTag_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/apriltagSourceCode)
SET(AprilTag_LIBS ${CMAKE_SOURCE_DIR}/apriltagSourceCode/libapriltag.so)

#set Camera Driver
SET(MindVersion_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/MindVisionAPI/Include)
SET(MindVersion_LIBS ${CMAKE_SOURCE_DIR}/MindVisionAPI/Library/libMVSDK.so)


#寻找OpenCV库
FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

#打印调试信息
MESSAGE(STATUS "Project: ${PROJECT_NAME}")
MESSAGE(STATUS "OpenCV library status:")
MESSAGE(STATUS "    version: ${OpenCV_VERSION}")
MESSAGE(STATUS "    libraries: ${OpenCV_LIBS}")
MESSAGE(STATUS "    libraries: ${AprilTag_LIBS}")
MESSAGE(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
MESSAGE(STATUS "    include path: ${AprilTag_INCLUDE_DIRS}")

#3.head file path，头文件目录
INCLUDE_DIRECTORIES(
 ${AprilTag_INCLUDE_DIRS}
 ${MindVersion_INCLUDE_DIRS}
)

#4.source directory，源文件目录,#获取代码，在项目中，将所有代码都放在src文件夹中
AUX_SOURCE_DIRECTORY(src DIR_SRCS)
MESSAGE(STATUS "Src file: ${DIR_SRCS}")

#5.set environment variable，设置环境变量，编译用到的源文件全部都要放到这里，否则编译能够通过，但是执行的时候会出现各种问题，比如"symbol lookup error xxxxx , undefined symbol"
SET(TEST_MATH
${DIR_SRCS}
)


#6.add executable file，添加要编译的可执行文件
ADD_EXECUTABLE(${PROJECT_NAME} ${TEST_MATH})

#7.add link library，添加可执行文件所需要的库，比如我们用到了libm.so（命名规则：lib+name+.so），就添加该库的名称
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenCV_LIBS} ${AprilTag_LIBS}  ${MindVersion_LIBS})
