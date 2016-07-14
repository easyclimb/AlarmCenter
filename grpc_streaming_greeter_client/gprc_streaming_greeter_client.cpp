// gprc_streaming_greeter_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "hellostreamingworld.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using hellostreamingworld::HelloRequest;
using hellostreamingworld::HelloReply;
using hellostreamingworld::MultiGreeter;

class GreeterClient {
public:
	GreeterClient(std::shared_ptr<Channel> channel)
		: stub_(MultiGreeter::NewStub(channel)) {}

	// Assambles the client's payload, sends it and presents the response back
	// from the server.
	void SayHello(const std::string& user, const std::string& num_greetings) {
		// Data we are sending to the server.
		HelloRequest request;
		request.set_name(user);
		request.set_num_greetings(num_greetings);
		// Container for the data we expect from the server.
		//HelloReply reply;

		// Context for the client. It could be used to convey extra information to
		// the server and/or tweak certain RPC behaviors.
		ClientContext context;

		// The actual RPC.
		//Status status = stub_->AsyncsayHello(&context, request, &reply);

		// Act upon its status.
		/*if (status.ok()) {
			return reply.message();
		} else {
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;
			return "RPC failed";
		}*/

		auto reader = stub_->sayHello(&context, request);
		HelloReply reply;
		while (reader->Read(&reply)) {
			std::cout << "Greeter received: " << reply.message() << std::endl;
		}

		Status status = reader->Finish();
		if (status.ok()) {
			std::cout << "SayHello rpc succeeded." << std::endl;
		} else {
			std::cout << "SayHello rpc failed." << std::endl;
		}
	}

private:
	std::unique_ptr<MultiGreeter::Stub> stub_;
};



int main(int argc, char** argv) {
	// Instantiate the client. It requires a channel, out of which the actual RPCs
	// are created. This channel models a connection to an endpoint (in this case,
	// localhost at port 50051). We indicate that the channel isn't authenticated
	// (use of InsecureChannelCredentials()).
	GreeterClient greeter(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
	std::string user("world"), num("3");
	//std::string reply = greeter.SayHello(user);
	//std::cout << "Greeter received: " << reply << std::endl;
	greeter.SayHello(user, num);
	std::system("pause");
	return 0;
}
