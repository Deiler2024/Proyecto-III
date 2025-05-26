#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "../common/generated/tecmfs.grpc.pb.h"

struct DiskConfig {
    std::string address;
    int port;
    std::string path;
    int diskSize;
    int blockSize;
};

DiskConfig loadConfig(const std::string& filename) {
    DiskConfig config;

    xmlDocPtr doc = xmlReadFile(filename.c_str(), NULL, 0);
    if (doc == nullptr) {
        std::cerr << "Error al abrir " << filename << std::endl;
        exit(1);
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    for (xmlNodePtr node = root->children; node != nullptr; node = node->next) {
        if (node->type != XML_ELEMENT_NODE) continue;

        std::string tag = (const char*)node->name;
        std::string value = (const char*)xmlNodeGetContent(node);

        if (tag == "Address") config.address = value;
        else if (tag == "Port") config.port = std::stoi(value);
        else if (tag == "Path") config.path = value;
        else if (tag == "DiskSize") config.diskSize = std::stoi(value);
        else if (tag == "BlockSize") config.blockSize = std::stoi(value);
    }

    xmlFreeDoc(doc);
    return config;
}

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using tecmfs::DiskService;
using tecmfs::PingRequest;
using tecmfs::PingResponse;

class DiskServiceImpl final : public DiskService::Service {
    Status Ping(ServerContext* context, const PingRequest* request, PingResponse* response) override {
        std::cout << "Received ping: " << request->message() << std::endl;
        response->set_message("Pong from Disk Node");
        return Status::OK;
    }
};

void RunServer() {
    DiskConfig config = loadConfig("disk_node/config.xml");

    std::cout << "Configuración cargada:\n";
    std::cout << "IP: " << config.address << "\n";
    std::cout << "Puerto: " << config.port << "\n";
    std::cout << "Ruta: " << config.path << "\n";
    std::cout << "Tamaño del disco (bloques): " << config.diskSize << "\n";
    std::cout << "Tamaño de bloque (bytes): " << config.blockSize << "\n";

    std::string server_address = config.address + ":" + std::to_string(config.port);
    DiskServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Disk Node ESCUCHA en: " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
