// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: alarm_center_map.proto
#include "stdafx.h"

#include "alarm_center_map.pb.h"
#include "alarm_center_map.grpc.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>
namespace alarm_center_map {

static const char* map_service_method_names[] = {
  "/alarm_center_map.map_service/get_csr_info",
  "/alarm_center_map.map_service/get_alarming_machines_info",
  "/alarm_center_map.map_service/set_csr_info",
  "/alarm_center_map.map_service/set_machine_info",
};

std::unique_ptr< map_service::Stub> map_service::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
	std::unique_ptr< map_service::Stub> stub(new map_service::Stub(channel));
	return stub;
}

map_service::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
	: channel_(channel), rpcmethod_get_csr_info_(map_service_method_names[0], ::grpc::RpcMethod::NORMAL_RPC, channel)
	, rpcmethod_get_alarming_machines_info_(map_service_method_names[1], ::grpc::RpcMethod::SERVER_STREAMING, channel)
	, rpcmethod_set_csr_info_(map_service_method_names[2], ::grpc::RpcMethod::NORMAL_RPC, channel)
	, rpcmethod_set_machine_info_(map_service_method_names[3], ::grpc::RpcMethod::NORMAL_RPC, channel)
{}

::grpc::Status map_service::Stub::get_csr_info(::grpc::ClientContext* context, const ::alarm_center_map::csr_info& request, ::alarm_center_map::csr_info* response) {
	return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_get_csr_info_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::alarm_center_map::csr_info>* map_service::Stub::Asyncget_csr_infoRaw(::grpc::ClientContext* context, const ::alarm_center_map::csr_info& request, ::grpc::CompletionQueue* cq) {
	return new ::grpc::ClientAsyncResponseReader< ::alarm_center_map::csr_info>(channel_.get(), cq, rpcmethod_get_csr_info_, context, request);
}

::grpc::ClientReader< ::alarm_center_map::machine_info>* map_service::Stub::get_alarming_machines_infoRaw(::grpc::ClientContext* context, const ::alarm_center_map::csr_info& request) {
	return new ::grpc::ClientReader< ::alarm_center_map::machine_info>(channel_.get(), rpcmethod_get_alarming_machines_info_, context, request);
}

::grpc::ClientAsyncReader< ::alarm_center_map::machine_info>* map_service::Stub::Asyncget_alarming_machines_infoRaw(::grpc::ClientContext* context, const ::alarm_center_map::csr_info& request, ::grpc::CompletionQueue* cq, void* tag) {
	return new ::grpc::ClientAsyncReader< ::alarm_center_map::machine_info>(channel_.get(), cq, rpcmethod_get_alarming_machines_info_, context, request, tag);
}

::grpc::Status map_service::Stub::set_csr_info(::grpc::ClientContext* context, const ::alarm_center_map::csr_info& request, ::alarm_center_map::csr_info* response) {
	return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_set_csr_info_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::alarm_center_map::csr_info>* map_service::Stub::Asyncset_csr_infoRaw(::grpc::ClientContext* context, const ::alarm_center_map::csr_info& request, ::grpc::CompletionQueue* cq) {
	return new ::grpc::ClientAsyncResponseReader< ::alarm_center_map::csr_info>(channel_.get(), cq, rpcmethod_set_csr_info_, context, request);
}

::grpc::Status map_service::Stub::set_machine_info(::grpc::ClientContext* context, const ::alarm_center_map::machine_info& request, ::alarm_center_map::machine_info* response) {
	return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_set_machine_info_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::alarm_center_map::machine_info>* map_service::Stub::Asyncset_machine_infoRaw(::grpc::ClientContext* context, const ::alarm_center_map::machine_info& request, ::grpc::CompletionQueue* cq) {
	return new ::grpc::ClientAsyncResponseReader< ::alarm_center_map::machine_info>(channel_.get(), cq, rpcmethod_set_machine_info_, context, request);
}

map_service::Service::Service() {
	(void)map_service_method_names;
	AddMethod(new ::grpc::RpcServiceMethod(
		map_service_method_names[0],
		::grpc::RpcMethod::NORMAL_RPC,
		new ::grpc::RpcMethodHandler< map_service::Service, ::alarm_center_map::csr_info, ::alarm_center_map::csr_info>(
			std::mem_fn(&map_service::Service::get_csr_info), this)));
	AddMethod(new ::grpc::RpcServiceMethod(
		map_service_method_names[1],
		::grpc::RpcMethod::SERVER_STREAMING,
		new ::grpc::ServerStreamingHandler< map_service::Service, ::alarm_center_map::csr_info, ::alarm_center_map::machine_info>(
			std::mem_fn(&map_service::Service::get_alarming_machines_info), this)));
	AddMethod(new ::grpc::RpcServiceMethod(
		map_service_method_names[2],
		::grpc::RpcMethod::NORMAL_RPC,
		new ::grpc::RpcMethodHandler< map_service::Service, ::alarm_center_map::csr_info, ::alarm_center_map::csr_info>(
			std::mem_fn(&map_service::Service::set_csr_info), this)));
	AddMethod(new ::grpc::RpcServiceMethod(
		map_service_method_names[3],
		::grpc::RpcMethod::NORMAL_RPC,
		new ::grpc::RpcMethodHandler< map_service::Service, ::alarm_center_map::machine_info, ::alarm_center_map::machine_info>(
			std::mem_fn(&map_service::Service::set_machine_info), this)));
}

map_service::Service::~Service() {
}

::grpc::Status map_service::Service::get_csr_info(::grpc::ServerContext* context, const ::alarm_center_map::csr_info* request, ::alarm_center_map::csr_info* response) {
	(void)context;
	(void)request;
	(void)response;
	return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status map_service::Service::get_alarming_machines_info(::grpc::ServerContext* context, const ::alarm_center_map::csr_info* request, ::grpc::ServerWriter< ::alarm_center_map::machine_info>* writer) {
	(void)context;
	(void)request;
	(void)writer;
	return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status map_service::Service::set_csr_info(::grpc::ServerContext* context, const ::alarm_center_map::csr_info* request, ::alarm_center_map::csr_info* response) {
	(void)context;
	(void)request;
	(void)response;
	return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status map_service::Service::set_machine_info(::grpc::ServerContext* context, const ::alarm_center_map::machine_info* request, ::alarm_center_map::machine_info* response) {
	(void)context;
	(void)request;
	(void)response;
	return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace alarm_center_map
