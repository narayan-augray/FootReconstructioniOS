cmake_minimum_required(VERSION 3.18)

set(SRCS
floor.cpp floor.h
io.cpp io.h
sandbox.cpp sandbox.h
segmentation.cpp segmentation.h
stitching.cpp stitching.h
util.cpp util.h
alignment.cpp alignment.h
postprocessing.cpp postprocessing.h
reconstruction.cpp reconstruction.h
)

set(Open3D_ROOT C:/Open3D)
find_package(Open3D REQUIRED)

set(OpenCV_DIR C:/opencv/build)
find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS})

add_library(pipeline ${SRCS})

target_link_libraries(pipeline PRIVATE Open3D::Open3D ${OpenCV_LIBS} ../libs/tbb_static)