#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "../common/generated/tecmfs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using tecmfs::DiskService;
using tecmfs::PingRequest;
using tecmfs::PingResponse;

class ControllerClient {
public:
    ControllerClient(std::shared_ptr<Channel> channel)
        : stub_(DiskService::NewStub(channel)) {}

    void SendPing() {
        PingRequest request;
        request.set_message("Hello from Controller");

        PingResponse response;
        ClientContext context;

        Status status = stub_->Ping(&context, request, &response);

        if (status.ok()) {
            std::cout << "Received: " << response.message() << std::endl;
        } else {
            std::cout << "RPC failed: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<DiskService::Stub> stub_;
};

int main() {
    ControllerClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    client.SendPing();
    return 0;
}
