#include "MetadataManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "vector"

namespace fs = std::filesystem;

const std::string kMetadataDir = "controller/utils/raid_metadata/";

MetadataManager::MetadataManager() {
    if (!fs::exists(kMetadataDir)) {
        fs::create_directories(kMetadataDir);
    }
}

void MetadataManager::SaveMetadata(const std::string& filename, int totalDataBlocks, int lastBlockSize) {
    std::string path = kMetadataDir + filename + ".meta";
    std::ofstream out(path);
    if (!out) {
        std::cerr << "❌ Error al escribir metadatos en " << path << std::endl;
        return;
    }

    out << totalDataBlocks << " " << lastBlockSize << "\n";
    out.close();
}
void MetadataManager::DeleteMetadata(const std::string& filename) {
    std::string path = kMetadataDir + filename + ".meta";
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
        std::cout << "[INFO] Metadatos eliminados: " << path << std::endl;
    }
}


std::pair<int, int> MetadataManager::LoadMetadata(const std::string& filename) {
    std::string path = kMetadataDir + filename + ".meta";
    std::ifstream in(path);
    if (!in) {
        std::cerr << "❌ Error al leer metadatos desde " << path << std::endl;
        return {0, 0};
    }

    int total, lastSize;
    in >> total >> lastSize;
    return {total, lastSize};
}

std::vector<std::string> MetadataManager::ListStoredFiles() {
    std::vector<std::string> fileList;
    for (const auto& entry : fs::directory_iterator(kMetadataDir)) {
        if (entry.path().extension() == ".meta") {
            fileList.push_back(entry.path().stem().string());
        }
    }
    return fileList;
}
