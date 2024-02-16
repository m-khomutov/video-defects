cmake_minimum_required(VERSION 3.20)
project(videodefects)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/include/opencv4/)
add_executable(videodefects
               main.cpp
               reader.cpp
               encoder.cpp
               defects.cpp
               rtsp/socket.cpp
               rtsp/poll.cpp
               rtsp/connection.cpp
               rtsp/rtp.cpp
               rtsp/service.cpp)
target_link_libraries(videodefects opencv_core opencv_imgcodecs opencv_highgui opencv_videoio opencv_imgproc x264 uuid)
