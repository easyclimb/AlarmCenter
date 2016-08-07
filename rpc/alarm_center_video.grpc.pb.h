// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: alarm_center_video.proto
#ifndef GRPC_alarm_5fcenter_5fvideo_2eproto__INCLUDED
#define GRPC_alarm_5fcenter_5fvideo_2eproto__INCLUDED

#include "alarm_center_video.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/proto_utils.h>
#include <grpc++/impl/codegen/rpc_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/status.h>
#include <grpc++/impl/codegen/stub_options.h>
#include <grpc++/impl/codegen/sync_stream.h>

namespace grpc {
class CompletionQueue;
class Channel;
class RpcService;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc

namespace alarm_center_video {

// WM_VIDEO_INFO_CHANGE
//
// bind/unbind zone
//
// DeleteCameraInfo
//
class video_service GRPC_FINAL {
 public:
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status update_db(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::alarm_center_video::reply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>> Asyncupdate_db(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>>(Asyncupdate_dbRaw(context, request, cq));
    }
    virtual ::grpc::Status is_db_updated(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::alarm_center_video::reply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>> Asyncis_db_updated(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>>(Asyncis_db_updatedRaw(context, request, cq));
    }
    virtual ::grpc::Status get_is_show_video_user_mgr_dlg(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::alarm_center_video::reply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>> Asyncget_is_show_video_user_mgr_dlg(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>>(Asyncget_is_show_video_user_mgr_dlgRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientReaderInterface< ::alarm_center_video::alarm_info>> get_alarming_devs(::grpc::ClientContext* context, const ::alarm_center_video::request& request) {
      return std::unique_ptr< ::grpc::ClientReaderInterface< ::alarm_center_video::alarm_info>>(get_alarming_devsRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::alarm_center_video::alarm_info>> Asyncget_alarming_devs(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::alarm_center_video::alarm_info>>(Asyncget_alarming_devsRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientReaderInterface< ::alarm_center_video::bind_info>> get_updated_bind_infos(::grpc::ClientContext* context, const ::alarm_center_video::request& request) {
      return std::unique_ptr< ::grpc::ClientReaderInterface< ::alarm_center_video::bind_info>>(get_updated_bind_infosRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::alarm_center_video::bind_info>> Asyncget_updated_bind_infos(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::alarm_center_video::bind_info>>(Asyncget_updated_bind_infosRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientWriterInterface< ::alarm_center_video::hisroty_record>> insert_history_record(::grpc::ClientContext* context, ::alarm_center_video::reply* response) {
      return std::unique_ptr< ::grpc::ClientWriterInterface< ::alarm_center_video::hisroty_record>>(insert_history_recordRaw(context, response));
    }
    std::unique_ptr< ::grpc::ClientAsyncWriterInterface< ::alarm_center_video::hisroty_record>> Asyncinsert_history_record(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncWriterInterface< ::alarm_center_video::hisroty_record>>(Asyncinsert_history_recordRaw(context, response, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientWriterInterface< ::alarm_center_video::camera_info>> delete_camera_info(::grpc::ClientContext* context, ::alarm_center_video::reply* response) {
      return std::unique_ptr< ::grpc::ClientWriterInterface< ::alarm_center_video::camera_info>>(delete_camera_infoRaw(context, response));
    }
    std::unique_ptr< ::grpc::ClientAsyncWriterInterface< ::alarm_center_video::camera_info>> Asyncdelete_camera_info(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncWriterInterface< ::alarm_center_video::camera_info>>(Asyncdelete_camera_infoRaw(context, response, cq, tag));
    }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>* Asyncupdate_dbRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>* Asyncis_db_updatedRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::alarm_center_video::reply>* Asyncget_is_show_video_user_mgr_dlgRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientReaderInterface< ::alarm_center_video::alarm_info>* get_alarming_devsRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::alarm_center_video::alarm_info>* Asyncget_alarming_devsRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientReaderInterface< ::alarm_center_video::bind_info>* get_updated_bind_infosRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::alarm_center_video::bind_info>* Asyncget_updated_bind_infosRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientWriterInterface< ::alarm_center_video::hisroty_record>* insert_history_recordRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response) = 0;
    virtual ::grpc::ClientAsyncWriterInterface< ::alarm_center_video::hisroty_record>* Asyncinsert_history_recordRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientWriterInterface< ::alarm_center_video::camera_info>* delete_camera_infoRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response) = 0;
    virtual ::grpc::ClientAsyncWriterInterface< ::alarm_center_video::camera_info>* Asyncdelete_camera_infoRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) = 0;
  };
  class Stub GRPC_FINAL : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status update_db(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::alarm_center_video::reply* response) GRPC_OVERRIDE;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>> Asyncupdate_db(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>>(Asyncupdate_dbRaw(context, request, cq));
    }
    ::grpc::Status is_db_updated(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::alarm_center_video::reply* response) GRPC_OVERRIDE;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>> Asyncis_db_updated(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>>(Asyncis_db_updatedRaw(context, request, cq));
    }
    ::grpc::Status get_is_show_video_user_mgr_dlg(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::alarm_center_video::reply* response) GRPC_OVERRIDE;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>> Asyncget_is_show_video_user_mgr_dlg(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>>(Asyncget_is_show_video_user_mgr_dlgRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientReader< ::alarm_center_video::alarm_info>> get_alarming_devs(::grpc::ClientContext* context, const ::alarm_center_video::request& request) {
      return std::unique_ptr< ::grpc::ClientReader< ::alarm_center_video::alarm_info>>(get_alarming_devsRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::alarm_center_video::alarm_info>> Asyncget_alarming_devs(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::alarm_center_video::alarm_info>>(Asyncget_alarming_devsRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientReader< ::alarm_center_video::bind_info>> get_updated_bind_infos(::grpc::ClientContext* context, const ::alarm_center_video::request& request) {
      return std::unique_ptr< ::grpc::ClientReader< ::alarm_center_video::bind_info>>(get_updated_bind_infosRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::alarm_center_video::bind_info>> Asyncget_updated_bind_infos(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::alarm_center_video::bind_info>>(Asyncget_updated_bind_infosRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientWriter< ::alarm_center_video::hisroty_record>> insert_history_record(::grpc::ClientContext* context, ::alarm_center_video::reply* response) {
      return std::unique_ptr< ::grpc::ClientWriter< ::alarm_center_video::hisroty_record>>(insert_history_recordRaw(context, response));
    }
    std::unique_ptr< ::grpc::ClientAsyncWriter< ::alarm_center_video::hisroty_record>> Asyncinsert_history_record(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncWriter< ::alarm_center_video::hisroty_record>>(Asyncinsert_history_recordRaw(context, response, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientWriter< ::alarm_center_video::camera_info>> delete_camera_info(::grpc::ClientContext* context, ::alarm_center_video::reply* response) {
      return std::unique_ptr< ::grpc::ClientWriter< ::alarm_center_video::camera_info>>(delete_camera_infoRaw(context, response));
    }
    std::unique_ptr< ::grpc::ClientAsyncWriter< ::alarm_center_video::camera_info>> Asyncdelete_camera_info(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncWriter< ::alarm_center_video::camera_info>>(Asyncdelete_camera_infoRaw(context, response, cq, tag));
    }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>* Asyncupdate_dbRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) GRPC_OVERRIDE;
    ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>* Asyncis_db_updatedRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) GRPC_OVERRIDE;
    ::grpc::ClientAsyncResponseReader< ::alarm_center_video::reply>* Asyncget_is_show_video_user_mgr_dlgRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq) GRPC_OVERRIDE;
    ::grpc::ClientReader< ::alarm_center_video::alarm_info>* get_alarming_devsRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request) GRPC_OVERRIDE;
    ::grpc::ClientAsyncReader< ::alarm_center_video::alarm_info>* Asyncget_alarming_devsRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) GRPC_OVERRIDE;
    ::grpc::ClientReader< ::alarm_center_video::bind_info>* get_updated_bind_infosRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request) GRPC_OVERRIDE;
    ::grpc::ClientAsyncReader< ::alarm_center_video::bind_info>* Asyncget_updated_bind_infosRaw(::grpc::ClientContext* context, const ::alarm_center_video::request& request, ::grpc::CompletionQueue* cq, void* tag) GRPC_OVERRIDE;
    ::grpc::ClientWriter< ::alarm_center_video::hisroty_record>* insert_history_recordRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response) GRPC_OVERRIDE;
    ::grpc::ClientAsyncWriter< ::alarm_center_video::hisroty_record>* Asyncinsert_history_recordRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) GRPC_OVERRIDE;
    ::grpc::ClientWriter< ::alarm_center_video::camera_info>* delete_camera_infoRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response) GRPC_OVERRIDE;
    ::grpc::ClientAsyncWriter< ::alarm_center_video::camera_info>* Asyncdelete_camera_infoRaw(::grpc::ClientContext* context, ::alarm_center_video::reply* response, ::grpc::CompletionQueue* cq, void* tag) GRPC_OVERRIDE;
    const ::grpc::RpcMethod rpcmethod_update_db_;
    const ::grpc::RpcMethod rpcmethod_is_db_updated_;
    const ::grpc::RpcMethod rpcmethod_get_is_show_video_user_mgr_dlg_;
    const ::grpc::RpcMethod rpcmethod_get_alarming_devs_;
    const ::grpc::RpcMethod rpcmethod_get_updated_bind_infos_;
    const ::grpc::RpcMethod rpcmethod_insert_history_record_;
    const ::grpc::RpcMethod rpcmethod_delete_camera_info_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status update_db(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response);
    virtual ::grpc::Status is_db_updated(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response);
    virtual ::grpc::Status get_is_show_video_user_mgr_dlg(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response);
    virtual ::grpc::Status get_alarming_devs(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::grpc::ServerWriter< ::alarm_center_video::alarm_info>* writer);
    virtual ::grpc::Status get_updated_bind_infos(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::grpc::ServerWriter< ::alarm_center_video::bind_info>* writer);
    virtual ::grpc::Status insert_history_record(::grpc::ServerContext* context, ::grpc::ServerReader< ::alarm_center_video::hisroty_record>* reader, ::alarm_center_video::reply* response);
    virtual ::grpc::Status delete_camera_info(::grpc::ServerContext* context, ::grpc::ServerReader< ::alarm_center_video::camera_info>* reader, ::alarm_center_video::reply* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_update_db : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_update_db() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_update_db() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status update_db(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestupdate_db(::grpc::ServerContext* context, ::alarm_center_video::request* request, ::grpc::ServerAsyncResponseWriter< ::alarm_center_video::reply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_is_db_updated : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_is_db_updated() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_is_db_updated() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status is_db_updated(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestis_db_updated(::grpc::ServerContext* context, ::alarm_center_video::request* request, ::grpc::ServerAsyncResponseWriter< ::alarm_center_video::reply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_get_is_show_video_user_mgr_dlg : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_get_is_show_video_user_mgr_dlg() {
      ::grpc::Service::MarkMethodAsync(2);
    }
    ~WithAsyncMethod_get_is_show_video_user_mgr_dlg() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status get_is_show_video_user_mgr_dlg(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestget_is_show_video_user_mgr_dlg(::grpc::ServerContext* context, ::alarm_center_video::request* request, ::grpc::ServerAsyncResponseWriter< ::alarm_center_video::reply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_get_alarming_devs : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_get_alarming_devs() {
      ::grpc::Service::MarkMethodAsync(3);
    }
    ~WithAsyncMethod_get_alarming_devs() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status get_alarming_devs(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::grpc::ServerWriter< ::alarm_center_video::alarm_info>* writer) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestget_alarming_devs(::grpc::ServerContext* context, ::alarm_center_video::request* request, ::grpc::ServerAsyncWriter< ::alarm_center_video::alarm_info>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncServerStreaming(3, context, request, writer, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_get_updated_bind_infos : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_get_updated_bind_infos() {
      ::grpc::Service::MarkMethodAsync(4);
    }
    ~WithAsyncMethod_get_updated_bind_infos() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status get_updated_bind_infos(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::grpc::ServerWriter< ::alarm_center_video::bind_info>* writer) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestget_updated_bind_infos(::grpc::ServerContext* context, ::alarm_center_video::request* request, ::grpc::ServerAsyncWriter< ::alarm_center_video::bind_info>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncServerStreaming(4, context, request, writer, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_insert_history_record : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_insert_history_record() {
      ::grpc::Service::MarkMethodAsync(5);
    }
    ~WithAsyncMethod_insert_history_record() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status insert_history_record(::grpc::ServerContext* context, ::grpc::ServerReader< ::alarm_center_video::hisroty_record>* reader, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestinsert_history_record(::grpc::ServerContext* context, ::grpc::ServerAsyncReader< ::alarm_center_video::reply, ::alarm_center_video::hisroty_record>* reader, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncClientStreaming(5, context, reader, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_delete_camera_info : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_delete_camera_info() {
      ::grpc::Service::MarkMethodAsync(6);
    }
    ~WithAsyncMethod_delete_camera_info() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status delete_camera_info(::grpc::ServerContext* context, ::grpc::ServerReader< ::alarm_center_video::camera_info>* reader, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestdelete_camera_info(::grpc::ServerContext* context, ::grpc::ServerAsyncReader< ::alarm_center_video::reply, ::alarm_center_video::camera_info>* reader, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncClientStreaming(6, context, reader, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_update_db<WithAsyncMethod_is_db_updated<WithAsyncMethod_get_is_show_video_user_mgr_dlg<WithAsyncMethod_get_alarming_devs<WithAsyncMethod_get_updated_bind_infos<WithAsyncMethod_insert_history_record<WithAsyncMethod_delete_camera_info<Service > > > > > > > AsyncService;
  template <class BaseClass>
  class WithGenericMethod_update_db : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_update_db() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_update_db() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status update_db(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_is_db_updated : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_is_db_updated() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_is_db_updated() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status is_db_updated(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_get_is_show_video_user_mgr_dlg : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_get_is_show_video_user_mgr_dlg() {
      ::grpc::Service::MarkMethodGeneric(2);
    }
    ~WithGenericMethod_get_is_show_video_user_mgr_dlg() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status get_is_show_video_user_mgr_dlg(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_get_alarming_devs : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_get_alarming_devs() {
      ::grpc::Service::MarkMethodGeneric(3);
    }
    ~WithGenericMethod_get_alarming_devs() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status get_alarming_devs(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::grpc::ServerWriter< ::alarm_center_video::alarm_info>* writer) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_get_updated_bind_infos : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_get_updated_bind_infos() {
      ::grpc::Service::MarkMethodGeneric(4);
    }
    ~WithGenericMethod_get_updated_bind_infos() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status get_updated_bind_infos(::grpc::ServerContext* context, const ::alarm_center_video::request* request, ::grpc::ServerWriter< ::alarm_center_video::bind_info>* writer) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_insert_history_record : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_insert_history_record() {
      ::grpc::Service::MarkMethodGeneric(5);
    }
    ~WithGenericMethod_insert_history_record() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status insert_history_record(::grpc::ServerContext* context, ::grpc::ServerReader< ::alarm_center_video::hisroty_record>* reader, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_delete_camera_info : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_delete_camera_info() {
      ::grpc::Service::MarkMethodGeneric(6);
    }
    ~WithGenericMethod_delete_camera_info() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status delete_camera_info(::grpc::ServerContext* context, ::grpc::ServerReader< ::alarm_center_video::camera_info>* reader, ::alarm_center_video::reply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
};

}  // namespace alarm_center_video


#endif  // GRPC_alarm_5fcenter_5fvideo_2eproto__INCLUDED