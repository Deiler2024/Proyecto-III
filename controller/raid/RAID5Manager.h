#pragma once

#include <string>
#include "../client/ControllerClient.h"

class RAID5Manager {
public:
    explicit RAID5Manager(ControllerClient& controller);

    // Usa el nombre base para acceder a metadatos por archivo
    std::pair<int, int> WriteFile(const std::string& filepath);
    bool ReadFile(const std::string& outputPath, const std::string& originalFilename);

    void ListFiles();
    void DeleteFile(const std::string& filename);


private:
    ControllerClient& controller_;
};
