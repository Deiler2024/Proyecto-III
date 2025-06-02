#ifndef METADATA_MANAGER_H
#define METADATA_MANAGER_H

#include <string>
#include <utility>

class MetadataManager {
public:
    MetadataManager();  // Constructor declarado correctamente

    // Guarda el número total de bloques de datos y el tamaño del último bloque
    void SaveMetadata(int totalDataBlocks, int lastBlockSize);

    // Carga la metadata previamente guardada
    std::pair<int, int> LoadMetadata();
};

#endif  // METADATA_MANAGER_H
