cmake_minimum_required(VERSION 3.28.0)                  #版本号
project(RaceCar)                                         #项目名
include_directories(include)                           #把include纳入包含目录中
link_libraries("${PROJECT_SOURCE_DIR}/lib/libglfw3.a") #链接之前生成的静态库文件
aux_source_directory(./src DIR_ALL_SRC)                #src目录下所有文件取个DIR_ALL_SRC名字
add_executable(RaceCar ${DIR_ALL_SRC})                   #生成可执行文件

