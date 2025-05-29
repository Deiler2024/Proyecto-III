#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "../common/generated/tecmfs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tecmfs::DiskService;
using tecmfs::PingRequest;
using tecmfs::PingResponse;
using tecmfs::BlockRequest;
using tecmfs::BlockResponse;
using tecmfs::BlockIndex;
using tecmfs::BlockData;

class ControllerClient {
public:
    ControllerClient(std::shared_ptr<Channel> channel)
        : stub_(DiskService::NewStub(channel)) {}

    void SendPing() {
        PingRequest request;
        request.set_message("Hello from Controller");

        PingResponse response;
        ClientContext context;

        Status status = stub_->Ping(&context, request, &response);

        if (status.ok()) {
            std::cout << "Ping Response: " << response.message() << std::endl;
        } else {
            std::cerr << "Ping failed: " << status.error_message() << std::endl;
        }
    }

    void WriteTestBlock() {
        BlockRequest request;
        request.set_index(0);
        request.set_data("Hola, disco!, todo bien?");

        BlockResponse response;
        ClientContext context;

        Status status = stub_->WriteBlock(&context, request, &response);

        if (status.ok()) {
            std::cout << "WriteBlock: " << (response.success() ? "✅ OK" : "❌ Falló")
                      << " - " << response.message() << std::endl;
        } else {
            std::cerr << "Error RPC WriteBlock: " << status.error_message() << std::endl;
        }
    }

    void ReadTestBlock() {
        BlockIndex request;
        request.set_index(0);  // Leeremos el bloque 0

        BlockData response;
        ClientContext context;

        Status status = stub_->ReadBlock(&context, request, &response);

        if (status.ok()) {
            if (response.success()) {
                std::cout << "ReadBlock: ✅ OK - " << response.message() << std::endl;
                std::cout << "Contenido leído: " << response.data() << std::endl;
            } else {
                std::cout << "ReadBlock: ❌ Falló - " << response.message() << std::endl;
            }
        } else {
            std::cerr << "Error RPC ReadBlock: " << status.error_message() << std::endl;
        }
    }

    void WriteFile(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            std::cerr << "No se pudo abrir el archivo: " << filepath << std::endl;
            return;
        }

        const int blockSize = 4096;  // Idealmente viene del servidor en el futuro
        int index = 0;

        while (!file.eof()) {
            std::vector<char> buffer(blockSize, 0);
            file.read(buffer.data(), blockSize);
            std::streamsize bytesRead = file.gcount();

            if (bytesRead <= 0) break;

            BlockRequest request;
            request.set_index(index);
            request.set_data(buffer.data(), bytesRead);

            BlockResponse response;
            ClientContext context;

            Status status = stub_->WriteBlock(&context, request, &response);
            if (status.ok() && response.success()) {
                std::cout << "Bloque " << index << " escrito ✅ (" << bytesRead << " bytes)" << std::endl;
            } else {
                std::cerr << "Error al escribir el bloque " << index << ": "
                          << (status.ok() ? response.message() : status.error_message()) << std::endl;
                break;
            }

            index++;
        }

        file.close();
        std::cout << "Archivo enviado exitosamente en " << index << " bloques." << std::endl;
    }


    void ReadFile(const std::string& outputPath, int startIndex = 0, int maxBlocks = 100) {
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) {
            std::cerr << "No se pudo crear el archivo: " << outputPath << std::endl;
            return;
        }
    
        const int blockSize = 4096;  // Igual que el usado para escribir
        int index = startIndex;
    
        for (int i = 0; i < maxBlocks; ++i) {
            BlockIndex request;
            request.set_index(index);
    
            BlockData response;
            ClientContext context;
    
            Status status = stub_->ReadBlock(&context, request, &response);
            if (!status.ok() || !response.success()) {
                std::cerr << "Error al leer el bloque " << index << ": "
                          << (status.ok() ? response.message() : status.error_message()) << std::endl;
                break;
            }
    
            const std::string& data = response.data();
            outFile.write(data.data(), data.size());
    
            std::cout << "Bloque " << index << " leído y guardado (" << data.size() << " bytes)" << std::endl;
    
            if (data.size() < blockSize) {
                std::cout << "Último bloque alcanzado." << std::endl;
                break;
            }
    
            ++index;
        }
    
        outFile.close();
        std::cout << "Archivo reconstruido como: " << outputPath << std::endl;
    }
    

private:
    std::unique_ptr<DiskService::Stub> stub_;
};

int main() {
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    ControllerClient client(channel);

    client.SendPing();
    client.WriteTestBlock();
    client.ReadTestBlock();

    client.WriteFile("proyecto.pdf");
    client.ReadFile("reconstruido.pdf", 0, 100);

    return 0;
}
