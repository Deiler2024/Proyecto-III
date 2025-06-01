#pragma once
#include <array>
#include <memory>
#include <string>
#include "../../common/generated/tecmfs.grpc.pb.h"


#pragma once

#include <string>
#include "../client/ControllerClient.h"

class RAID5Manager {
public:
    RAID5Manager(ControllerClient& controller);
    int WriteFile(const std::string& filepath);
    void ReadFile(const std::string& outputPath, int totalDataBlocks);

private:
    ControllerClient& controller_;
};
