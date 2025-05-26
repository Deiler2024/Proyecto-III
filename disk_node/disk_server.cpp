#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "../common/generated/tecmfs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using tecmfs::DiskService;
using tecmfs::PingRequest;
using tecmfs::PingResponse;

class DiskServiceImpl final : public DiskService::Service {
    Status Ping(ServerContext* context, const PingRequest* request, PingResponse* response) override {
        std::cout << "Received ping: " << request->message() << std::endl;
        response->set_message("Pong from Disk Node");
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    DiskServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Disk Node ESCUCHA en: " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
