#pragma once
#include <array>
#include <memory>
#include <string>
#include "../../common/generated/tecmfs.grpc.pb.h"

class RAID5Manager {
public:
    RAID5Manager();

    int WriteFileRAID5(const std::string& filepath);
    void ReadFileRAID5(const std::string& outputPath, int totalDataBlocks);

private:
    std::array<std::unique_ptr<tecmfs::DiskService::Stub>, 4> stubs_;
}; 
