#include "MetadataManager.h"
#include <fstream>
#include <iostream>
#include <ostream>
#include <filesystem>

namespace fs = std::filesystem;

const std::string kMetadataDir = "controller/utils/raid_metadata/";
const std::string kMetadataFile = kMetadataDir + "raid_metadata.txt";

MetadataManager::MetadataManager() {
    if (!fs::exists(kMetadataDir)) {
        fs::create_directories(kMetadataDir);
    }
}

void MetadataManager::SaveMetadata(int totalDataBlocks, int lastBlockSize) {
    std::ofstream out(kMetadataFile);
    if (!out) {
        std::cerr << "❌ Error: No se pudo abrir " << kMetadataFile << " para escribir." << std::endl;
        return;
    }

    out << totalDataBlocks << " " << lastBlockSize << "\n";
    out.close();
}

std::pair<int, int> MetadataManager::LoadMetadata() {
    std::ifstream in(kMetadataFile);
    if (!in) {
        std::cerr << "❌ Error: No se pudo abrir " << kMetadataFile << " para leer." << std::endl;
        return {0, 0};
    }

    int blocks, lastSize;
    in >> blocks >> lastSize;
    return {blocks, lastSize};
}
