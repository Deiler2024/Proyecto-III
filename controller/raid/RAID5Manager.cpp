#include "RAID5Manager.h"
#include <fstream>
#include <vector>
#include <iostream>
#include "../../common/generated/tecmfs.grpc.pb.h"

using grpc::ClientContext;
using grpc::Status;
using tecmfs::BlockRequest;
using tecmfs::BlockResponse;
using tecmfs::BlockIndex;
using tecmfs::BlockData;

RAID5Manager::RAID5Manager(ControllerClient& controller)
    : controller_(controller) {}

int RAID5Manager::WriteFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo: " << filepath << std::endl;
        return 0;
    }

    const int blockSize = 4096;
    int globalIndex = 0;
    int writtenDataBlocks = 0;
    int group = 0;

    while (!file.eof()) {
        std::vector<std::string> dataBlocks;
        std::vector<int> validSizes;

        for (int i = 0; i < 3; ++i) {
            std::vector<char> buffer(blockSize);
            file.read(buffer.data(), blockSize);
            std::streamsize bytesRead = file.gcount();

            if (bytesRead <= 0) break;

            dataBlocks.emplace_back(buffer.data(), bytesRead);
            validSizes.push_back((int)bytesRead);
        }

        if (dataBlocks.empty()) break;

        std::string parity(blockSize, 0);
        for (int i = 0; i < dataBlocks.size(); ++i) {
            for (int j = 0; j < validSizes[i]; ++j) {
                parity[j] ^= dataBlocks[i][j];
            }
        }

        int parityNode = group % 4;
        int dataIndex = 0;

        for (int i = 0; i < 4; ++i) {
            int node = (parityNode + i) % 4;
            BlockRequest request;
            request.set_index(globalIndex++);

            if (node == parityNode) {
                request.set_data(parity);
            } else if (dataIndex < dataBlocks.size()) {
                request.set_data(dataBlocks[dataIndex++]);
                writtenDataBlocks++;
            } else {
                continue;
            }

            BlockResponse response;
            ClientContext context;
            Status status = controller_.GetStub(node)->WriteBlock(&context, request, &response);

            if (status.ok() && response.success()) {
                std::cout << "[Nodo " << node << "] Bloque " << request.index()
                          << " escrito ✅ (" << request.data().size() << " bytes)" << std::endl;
            } else {
                std::cerr << "[Nodo " << node << "] Error al escribir bloque "
                          << request.index() << ": "
                          << (status.ok() ? response.message() : status.error_message()) << std::endl;
            }
        }

        group++;
    }

    file.close();
    return writtenDataBlocks;
}

void RAID5Manager::ReadFile(const std::string& outputPath, int totalDataBlocks) {
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "No se pudo crear el archivo: " << outputPath << std::endl;
        return;
    }

    const int blockSize = 4096;
    int group = 0;
    int globalIndex = 0;
    int blocksRecovered = 0;

    while (blocksRecovered < totalDataBlocks) {
        int parityNode = group % 4;

        for (int offset = 0; offset < 4 && blocksRecovered < totalDataBlocks; ++offset) {
            int node = (parityNode + offset) % 4;

            if (offset == 0) {
                globalIndex++; // Saltear bloque de paridad
                continue;
            }

            BlockIndex request;
            request.set_index(globalIndex++);

            BlockData response;
            ClientContext context;

            Status status = controller_.GetStub(node)->ReadBlock(&context, request, &response);

            if (status.ok() && response.success()) {
                const std::string& data = response.data();
                outFile.write(data.data(), data.size());
                std::cout << "Bloque de datos " << blocksRecovered
                          << " leído desde Nodo " << node << " (" << data.size() << " bytes)" << std::endl;

                if (data.size() < blockSize) {
                    std::cout << "Último bloque alcanzado." << std::endl;
                    outFile.close();
                    return;
                }

                blocksRecovered++;
            } else {
                std::cerr << "❌ Error al leer bloque de datos " << blocksRecovered
                          << " del Nodo " << node << ": "
                          << (status.ok() ? response.message() : status.error_message()) << std::endl;
                outFile.close();
                return;
            }
        }

        group++;
    }

    outFile.close();
    std::cout << "Archivo RAID reconstruido como: " << outputPath << std::endl;
}
