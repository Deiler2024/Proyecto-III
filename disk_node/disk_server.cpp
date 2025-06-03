#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>

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

void initializeDisk(const DiskConfig& config) {
    namespace fs = std::filesystem;

    fs::path storagePath(config.path);

    // Crear la carpeta si no existe
    if (!fs::exists(storagePath)) {
        std::cout << "Creando directorio: " << config.path << std::endl;
        fs::create_directories(storagePath);
    }

    // Crear los bloques binarios si no existen
    for (int i = 0; i < config.diskSize; ++i) {
        std::string filename = config.path + "/block_" + std::to_string(i) + ".bin";

        if (!fs::exists(filename)) {
            std::ofstream blockFile(filename, std::ios::binary);
            if (!blockFile) {
                std::cerr << "Error creando el bloque: " << filename << std::endl;
                continue;
            }

            std::vector<char> emptyBlock(config.blockSize, 0);
            blockFile.write(emptyBlock.data(), emptyBlock.size());
            blockFile.close();
        }
    }

    std::cout << "Inicialización de disco completa (" << config.diskSize
              << " bloques de " << config.blockSize << " bytes).\n";
}

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using tecmfs::DiskService;
using tecmfs::PingRequest;
using tecmfs::PingResponse;

using tecmfs::BlockRequest;
using tecmfs::BlockResponse;
using tecmfs::BlockIndex;
using tecmfs::BlockData;


class DiskServiceImpl final : public DiskService::Service {
    public:
        DiskServiceImpl(const DiskConfig& config) : config_(config) {}
    
        Status Ping(ServerContext* context, const PingRequest* request, PingResponse* response) override {
            std::cout << "Received ping: " << request->message() << std::endl;
            response->set_message("Pong from Disk Node");
            return Status::OK;
        }
    
        Status WriteBlock(ServerContext* context, const BlockRequest* request, BlockResponse* response) override {
            int index = request->index();
    
            if (index < 0 || index >= config_.diskSize) {
                response->set_success(false);
                response->set_message("Índice de bloque fuera de rango");
                return Status::OK;
            }
    
            std::string filename = config_.path + "/block_" + std::to_string(index) + ".bin";
            std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
            if (!outFile) {
                response->set_success(false);
                response->set_message("No se pudo abrir el archivo para escritura");
                return Status::OK;
            }
    
            outFile.write(request->data().data(), request->data().size());
            outFile.close();
    
            response->set_success(true);
            response->set_message("Bloque escrito exitosamente");
            std::cout << "Bloque " << index << " escrito (" << request->data().size() << " bytes)\n";
    
            return Status::OK;
        }
        Status ReadBlock(ServerContext* context, const BlockIndex* request, BlockData* response) override {
            int index = request->index();
        
            if (index < 0 || index >= config_.diskSize) {
                response->set_success(false);
                response->set_message("Índice de bloque fuera de rango");
                return Status::OK;
            }
        
            std::string filename = config_.path + "/block_" + std::to_string(index) + ".bin";
            std::ifstream inFile(filename, std::ios::binary);
            if (!inFile) {
                response->set_success(false);
                response->set_message("No se pudo abrir el archivo para lectura");
                return Status::OK;
            }
        
            std::string data((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
            response->set_data(data);
            response->set_success(true);
            response->set_message("Bloque leído correctamente");
        
            std::cout << "Bloque " << index << " leído (" << data.size() << " bytes)\n";
        
            return Status::OK;
        }
        Status DeleteBlock(ServerContext* context, const BlockIndex* request, BlockResponse* response) override {
            int index = request->index();
        
            if (index < 0 || index >= config_.diskSize) {
                response->set_success(false);
                response->set_message("Índice de bloque fuera de rango");
                return Status::OK;
            }
        
            std::string filename = config_.path + "/block_" + std::to_string(index) + ".bin";
            if (std::filesystem::exists(filename)) {
                std::filesystem::remove(filename);
                response->set_success(true);
                response->set_message("Bloque eliminado exitosamente");
                std::cout << "Bloque " << index << " eliminado.\n";
            } else {
                response->set_success(false);
                response->set_message("El archivo de bloque no existe");
            }
        
            return Status::OK;
        }
        
        
    
    private:
        DiskConfig config_;
    };
    

    void RunServer(const std::string& configPath) {
        DiskConfig config = loadConfig(configPath);
    
        std::cout << "Configuración cargada desde: " << configPath << "\n";
        std::cout << "IP: " << config.address << "\n";
        std::cout << "Puerto: " << config.port << "\n";
        std::cout << "Ruta: " << config.path << "\n";
        std::cout << "Tamaño del disco (bloques): " << config.diskSize << "\n";
        std::cout << "Tamaño de bloque (bytes): " << config.blockSize << "\n";
    
        initializeDisk(config);
    
        std::string server_address = config.address + ":" + std::to_string(config.port);
        DiskServiceImpl service(config); // ← Se pasa la configuración
    
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
    
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Disk Node ESCUCHA en: " << server_address << std::endl;
        server->Wait();
    }
    
    
    

int main(int argc, char** argv) {
    std::string configPath = "disk_node/config.xml";  // por defecto

    if (argc >= 2) {
        configPath = argv[1];
    }

    RunServer(configPath);
    return 0;
}

