// grpc_streaming_greeter_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "hellostreamingworld.grpc.pb.h"

class GreeterServiceImpl final : public hellostreamingworld::MultiGreeter::Service
{
	virtual ::grpc::Status sayHello(::grpc::ServerContext* context, 
									const ::hellostreamingworld::HelloRequest* request, 
									::grpc::ServerWriter< ::hellostreamingworld::HelloReply>* writer) {
		hellostreamingworld::HelloReply reply;
		std::string hello("hello ");
		auto num = std::stoi(request->num_greetings());
		while (num-- > 0) {
			reply.set_message(hello + request->name());
			if (!writer->Write(reply)) {
				break;
			}
		}
		return ::grpc::Status::OK;
	}
};


int main()
{
	std::string server_address("0.0.0.0:50051");
	GreeterServiceImpl service;
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();


    return 0;
}

