#include <iostream>
#include "../raid/RAID5Manager.h"

#include "ControllerClient.h"
#include <iostream>

using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;
using tecmfs::PingRequest;
using tecmfs::PingResponse;
using grpc::ClientContext;
using grpc::Status;

ControllerClient::ControllerClient() {
    std::vector<std::string> addresses = {
        "127.0.0.1:50051",
        "127.0.0.2:50052",
        "127.0.0.3:50053",
        "127.0.0.4:50054"
    };

    for (int i = 0; i < 4; ++i) {
        stubs_[i] = tecmfs::DiskService::NewStub(CreateChannel(addresses[i], InsecureChannelCredentials()));
    }
}

std::shared_ptr<tecmfs::DiskService::Stub> ControllerClient::GetStub(int index) {
    return stubs_[index];
}

void ControllerClient::SendPing() {
    PingRequest request;
    request.set_message("Hello from Controller");

    PingResponse response;
    ClientContext context;

    Status status = stubs_[0]->Ping(&context, request, &response);
    if (status.ok()) {
        std::cout << "Ping Response: " << response.message() << std::endl;
    } else {
        std::cerr << "Ping failed: " << status.error_message() << std::endl;
    }
}
