#pragma once
#include <array>
#include <memory>
#include <string>
#include "../../common/generated/tecmfs.grpc.pb.h"





#include <utility>
#include "../client/ControllerClient.h"

class RAID5Manager {
public:
    explicit RAID5Manager(ControllerClient& controller);

    // Escribe el archivo y retorna: (total bloques de datos escritos, tamaño del último bloque)
    std::pair<int, int> WriteFile(const std::string& filepath);

    // Reconstruye el archivo usando los metadatos (ignora el segundo argumento)
    void ReadFile(const std::string& outputPath, int ignored = 0);

private:
    ControllerClient& controller_;
};
