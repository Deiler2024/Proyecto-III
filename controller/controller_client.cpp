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
using tecmfs::BlockRequest;
using tecmfs::BlockResponse;

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
            std::cout << "Ping Response: " << response.message() << std::endl;
        } else {
            std::cerr << "Ping failed: " << status.error_message() << std::endl;
        }
    }

    void WriteTestBlock() {
        BlockRequest request;
        request.set_index(0);
        request.set_data("Hola, disco!");

        BlockResponse response;
        ClientContext context;

        Status status = stub_->WriteBlock(&context, request, &response);

        if (status.ok()) {
            std::cout << "WriteBlock: " << (response.success() ? "✅ OK" : "❌ Falló")
                      << " - " << response.message() << std::endl;
        } else {
            std::cerr << "Error RPC WriteBlock: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<DiskService::Stub> stub_;
};

int main() {
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    ControllerClient client(channel);

    client.SendPing();
    client.WriteTestBlock();

    return 0;
}
