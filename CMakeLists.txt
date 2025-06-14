cmake_minimum_required(VERSION 3.16)
project(TECMFS)

set(CMAKE_CXX_STANDARD 17)

# Buscar dependencias necesarias
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(LibXml2 REQUIRED)

# Incluir directorios
include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/controller
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/client
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/raid
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/utils
)

# Archivos generados por Protobuf/gRPC
set(PROTO_SRCS
    common/generated/tecmfs.pb.cc
    common/generated/tecmfs.grpc.pb.cc
)

# ------------------------------
# Ejecutable del nodo de disco
# ------------------------------
add_executable(disk_node
    disk_node/disk_server.cpp
    ${PROTO_SRCS}
)

target_link_libraries(disk_node
    gRPC::grpc++
    ${Protobuf_LIBRARIES}
    ${LIBXML2_LIBRARIES}  # ← necesario por uso de XML
)

# ------------------------------
# Ejecutable del controlador
# ------------------------------
add_executable(controller
    main.cpp
    controller/client/ControllerClient.cpp
    controller/raid/RAID5Manager.cpp
    controller/utils/Logger.cpp
    controller/utils/MetadataManager.cpp
    ${PROTO_SRCS}
)

target_link_libraries(controller
    gRPC::grpc++
    ${Protobuf_LIBRARIES}
    ${LIBXML2_LIBRARIES}  # ← necesario si MetadataManager usa libxml2 para metadatos
)

