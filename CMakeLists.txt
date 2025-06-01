cmake_minimum_required(VERSION 3.16)
project(TECMFS)

set(CMAKE_CXX_STANDARD 17)

# Incluir Protobuf, gRPC y libxml2
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(LibXml2 REQUIRED)  # ← Agregado

include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}  # ← Agregado para libxml2
    ${CMAKE_CURRENT_SOURCE_DIR}/common/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/controller  # ← Agregar para encontrar RAID5Manager.h
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
    ${LIBXML2_LIBRARIES}  # ← Agregado para libxml2
)

# Controller
add_executable(controller
    controller/controller_client.cpp
    controller/raid/RAID5Manager.cpp  # ← Agregar el nuevo archivo de implementación
    ${PROTO_SRCS}
)
target_link_libraries(controller
    gRPC::grpc++
    ${Protobuf_LIBRARIES}
)
