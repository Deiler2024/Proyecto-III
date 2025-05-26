cmake_minimum_required(VERSION 3.16)
project(TECMFS)

set(CMAKE_CXX_STANDARD 17)

# Incluir Protobuf y gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/common/generated
)

# Archivos generados por Protobuf/gRPC
set(PROTO_SRCS
    common/generated/tecmfs.pb.cc
    common/generated/tecmfs.grpc.pb.cc
)

set(PROTO_HDRS
    common/generated/tecmfs.pb.h
    common/generated/tecmfs.grpc.pb.h
)

# Disk Node
add_executable(disk_node
    disk_node/disk_server.cpp
    ${PROTO_SRCS}
)
target_link_libraries(disk_node
    gRPC::grpc++
    ${Protobuf_LIBRARIES}
)

# Controller
add_executable(controller
    controller/controller_client.cpp
    ${PROTO_SRCS}
)
target_link_libraries(controller
    gRPC::grpc++
    ${Protobuf_LIBRARIES}
)
