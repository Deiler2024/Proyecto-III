#include "RAID5Manager.h"
#include "../utils/MetadataManager.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

using grpc::ClientContext;
using grpc::Status;
using tecmfs::BlockRequest;
using tecmfs::BlockResponse;
using tecmfs::BlockIndex;
using tecmfs::BlockData;

RAID5Manager::RAID5Manager(ControllerClient& controller)
    : controller_(controller) {}

std::pair<int, int> RAID5Manager::WriteFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo: " << filepath << std::endl;
        return {0, 0};
    }

    std::string filename = std::filesystem::path(filepath).filename().string();

    const int blockSize = 4096;
    int globalIndex = 0;
    int writtenDataBlocks = 0;
    int lastBlockSize = blockSize;
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

        lastBlockSize = validSizes.back();

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

    MetadataManager meta;
    meta.SaveMetadata(filename, writtenDataBlocks, lastBlockSize);
    return {writtenDataBlocks, lastBlockSize};
}

bool RAID5Manager::ReadFile(const std::string& outputPath, const std::string& originalFilename) {
    MetadataManager meta;
    auto [totalDataBlocks, lastBlockSize] = meta.LoadMetadata(originalFilename);
    if (totalDataBlocks == 0 || lastBlockSize == 0) {
        std::cerr << "❌ Error: No se encontraron metadatos para '" << originalFilename << "'." << std::endl;
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "No se pudo crear el archivo: " << outputPath << std::endl;
        return false;
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
                globalIndex++;
                continue;
            }

            BlockIndex request;
            request.set_index(globalIndex++);

            BlockData response;
            ClientContext context;
            Status status = controller_.GetStub(node)->ReadBlock(&context, request, &response);

            if (status.ok() && response.success()) {
                std::string data = response.data();
                if (blocksRecovered == totalDataBlocks - 1) {
                    data.resize(lastBlockSize);
                }

                outFile.write(data.data(), data.size());
                std::cout << "Bloque de datos " << blocksRecovered
                          << " leído desde Nodo " << node << " (" << data.size() << " bytes)" << std::endl;

                blocksRecovered++;
            } else {
                std::cerr << "❌ Error al leer bloque de datos " << blocksRecovered
                          << " del Nodo " << node << ": "
                          << (status.ok() ? response.message() : status.error_message()) << std::endl;
                outFile.close();
                return false;
            }
        }

        group++;
    }

    outFile.close();
    std::cout << "Archivo RAID reconstruido como: " << outputPath << std::endl;
    return true;
}


void RAID5Manager::ListFiles() {
    MetadataManager meta;
    auto files = meta.ListStoredFiles();

    std::cout << "[INFO] Archivos almacenados en RAID 5:\n";
    for (const auto& name : files) {
        std::cout << " - " << name << "\n";
    }
}
void RAID5Manager::DeleteFile(const std::string& filename) {
    MetadataManager meta;
    auto [totalBlocks, _] = meta.LoadMetadata(filename);  // usamos solo totalBlocks

    if (totalBlocks == 0) {
        std::cerr << "❌ No se encontraron metadatos para el archivo '" << filename << "'." << std::endl;
        return;
    }

    const int blocksPerStripe = 3;
    int totalGroups = (totalBlocks + blocksPerStripe - 1) / blocksPerStripe;
    int globalIndex = 0;

    for (int group = 0; group < totalGroups; ++group) {
        int parityNode = group % 4;

        for (int offset = 0; offset < 4; ++offset) {
            int node = (parityNode + offset) % 4;

            // No podemos eliminar más bloques de los que se escribieron
            if (globalIndex >= (totalBlocks + totalGroups)) break;

            BlockIndex request;
            request.set_index(globalIndex++);

            BlockResponse response;
            ClientContext context;

            Status status = controller_.GetStub(node)->DeleteBlock(&context, request, &response);
            if (status.ok() && response.success()) {
                std::cout << "[Nodo " << node << "] Bloque " << request.index()
                          << " eliminado ✅" << std::endl;
            } else {
                std::cerr << "[Nodo " << node << "] Error al eliminar bloque "
                          << request.index() << ": "
                          << (status.ok() ? response.message() : status.error_message()) << std::endl;
            }
        }
    }

    meta.DeleteMetadata(filename);  // elimina el archivo .meta
    std::cout << "[INFO] Archivo '" << filename << "' eliminado del sistema RAID 5." << std::endl;
}

