#pragma once

#include <array>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "../../common/generated/tecmfs.grpc.pb.h"

class ControllerClient {
public:
    ControllerClient();
    std::shared_ptr<tecmfs::DiskService::Stub> GetStub(int index);
    void SendPing();

private:
    std::array<std::shared_ptr<tecmfs::DiskService::Stub>, 4> stubs_;
};
