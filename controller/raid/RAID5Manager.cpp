#include "RAID5Manager.h"
#include "../utils/MetadataManager.h"
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

std::pair<int, int> RAID5Manager::WriteFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo: " << filepath << std::endl;
        return {0, 0};
    }

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

        lastBlockSize = validSizes.back();  // Último tamaño válido leído

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
    meta.SaveMetadata(writtenDataBlocks, lastBlockSize);

    return {writtenDataBlocks, lastBlockSize};
}

void RAID5Manager::ReadFile(const std::string& outputPath, int /*unused*/) {
    MetadataManager meta;
    auto [totalDataBlocks, lastBlockSize] = meta.LoadMetadata();

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
        std::array<std::string, 4> blockData{};
        std::array<bool, 4> blockOk{false, false, false, false};

        // Leer los 4 bloques del grupo
        for (int offset = 0; offset < 4; ++offset) {
            int node = (parityNode + offset) % 4;
            BlockIndex request;
            request.set_index(globalIndex + offset);

            BlockData response;
            ClientContext context;
            Status status = controller_.GetStub(node)->ReadBlock(&context, request, &response);

            if (status.ok() && response.success()) {
                blockData[offset] = response.data();
                blockOk[offset] = true;
            }
        }

        // Recuperar los bloques de datos, excepto paridad
        for (int offset = 0; offset < 4 && blocksRecovered < totalDataBlocks; ++offset) {
            int node = (parityNode + offset) % 4;
            if (offset == 0) continue;  // Paridad

            std::string data;
            if (blockOk[offset]) {
                data = blockData[offset];
            } else {
                // Reconstrucción por XOR
                data.assign(blockSize, 0);
                for (int j = 0; j < 4; ++j) {
                    if (j == offset || !blockOk[j]) continue;
                    for (int k = 0; k < blockData[j].size(); ++k) {
                        data[k] ^= blockData[j][k];
                    }
                }
                std::cout << "⚠️  Bloque " << blocksRecovered << " reconstruido por paridad XOR\n";
            }

            if (blocksRecovered == totalDataBlocks - 1) {
                data.resize(lastBlockSize);
            }

            outFile.write(data.data(), data.size());
            std::cout << "Bloque de datos " << blocksRecovered
                      << " procesado (" << data.size() << " bytes)" << std::endl;
            blocksRecovered++;
        }

        globalIndex += 4;
        group++;
    }

    outFile.close();
    std::cout << "Archivo RAID reconstruido como: " << outputPath << std::endl;
}
