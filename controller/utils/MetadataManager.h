#ifndef METADATA_MANAGER_H
#define METADATA_MANAGER_H
#include "vector"
#include <string>
#include <utility>

class MetadataManager {
public:
    MetadataManager();

    // Guarda los metadatos específicos para un archivo
    void SaveMetadata(const std::string& filename, int totalDataBlocks, int lastBlockSize);
    void DeleteMetadata(const std::string& filename);

    

    // Carga los metadatos de un archivo específico
    std::pair<int, int> LoadMetadata(const std::string& filename);
    std::vector<std::string> ListStoredFiles();

};

#endif  // METADATA_MANAGER_H
