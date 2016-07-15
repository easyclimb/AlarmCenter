// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: alarm_center_video.proto

#ifndef PROTOBUF_alarm_5fcenter_5fvideo_2eproto__INCLUDED
#define PROTOBUF_alarm_5fcenter_5fvideo_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace alarm_center_video {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_alarm_5fcenter_5fvideo_2eproto();
void protobuf_AssignDesc_alarm_5fcenter_5fvideo_2eproto();
void protobuf_ShutdownFile_alarm_5fcenter_5fvideo_2eproto();

class alarm_info;
class dev_info;
class hisroty_record;
class reply;
class request;

// ===================================================================

class request : public ::google::protobuf::Message {
 public:
  request();
  virtual ~request();

  request(const request& from);

  inline request& operator=(const request& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const request& default_instance();

  void Swap(request* other);

  // implements Message ----------------------------------------------

  inline request* New() const { return New(NULL); }

  request* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const request& from);
  void MergeFrom(const request& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(request* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string place_holder = 1;
  void clear_place_holder();
  static const int kPlaceHolderFieldNumber = 1;
  const ::std::string& place_holder() const;
  void set_place_holder(const ::std::string& value);
  void set_place_holder(const char* value);
  void set_place_holder(const char* value, size_t size);
  ::std::string* mutable_place_holder();
  ::std::string* release_place_holder();
  void set_allocated_place_holder(::std::string* place_holder);

  // @@protoc_insertion_point(class_scope:alarm_center_video.request)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr place_holder_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_AssignDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_ShutdownFile_alarm_5fcenter_5fvideo_2eproto();

  void InitAsDefaultInstance();
  static request* default_instance_;
};
// -------------------------------------------------------------------

class reply : public ::google::protobuf::Message {
 public:
  reply();
  virtual ~reply();

  reply(const reply& from);

  inline reply& operator=(const reply& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const reply& default_instance();

  void Swap(reply* other);

  // implements Message ----------------------------------------------

  inline reply* New() const { return New(NULL); }

  reply* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const reply& from);
  void MergeFrom(const reply& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(reply* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string place_holder = 1;
  void clear_place_holder();
  static const int kPlaceHolderFieldNumber = 1;
  const ::std::string& place_holder() const;
  void set_place_holder(const ::std::string& value);
  void set_place_holder(const char* value);
  void set_place_holder(const char* value, size_t size);
  ::std::string* mutable_place_holder();
  ::std::string* release_place_holder();
  void set_allocated_place_holder(::std::string* place_holder);

  // @@protoc_insertion_point(class_scope:alarm_center_video.reply)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr place_holder_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_AssignDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_ShutdownFile_alarm_5fcenter_5fvideo_2eproto();

  void InitAsDefaultInstance();
  static reply* default_instance_;
};
// -------------------------------------------------------------------

class dev_info : public ::google::protobuf::Message {
 public:
  dev_info();
  virtual ~dev_info();

  dev_info(const dev_info& from);

  inline dev_info& operator=(const dev_info& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const dev_info& default_instance();

  void Swap(dev_info* other);

  // implements Message ----------------------------------------------

  inline dev_info* New() const { return New(NULL); }

  dev_info* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const dev_info& from);
  void MergeFrom(const dev_info& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(dev_info* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 dev_id = 1;
  void clear_dev_id();
  static const int kDevIdFieldNumber = 1;
  ::google::protobuf::int32 dev_id() const;
  void set_dev_id(::google::protobuf::int32 value);

  // optional string user = 2;
  void clear_user();
  static const int kUserFieldNumber = 2;
  const ::std::string& user() const;
  void set_user(const ::std::string& value);
  void set_user(const char* value);
  void set_user(const char* value, size_t size);
  ::std::string* mutable_user();
  ::std::string* release_user();
  void set_allocated_user(::std::string* user);

  // optional string note = 3;
  void clear_note();
  static const int kNoteFieldNumber = 3;
  const ::std::string& note() const;
  void set_note(const ::std::string& value);
  void set_note(const char* value);
  void set_note(const char* value, size_t size);
  ::std::string* mutable_note();
  ::std::string* release_note();
  void set_allocated_note(::std::string* note);

  // optional string productor = 4;
  void clear_productor();
  static const int kProductorFieldNumber = 4;
  const ::std::string& productor() const;
  void set_productor(const ::std::string& value);
  void set_productor(const char* value);
  void set_productor(const char* value, size_t size);
  ::std::string* mutable_productor();
  ::std::string* release_productor();
  void set_allocated_productor(::std::string* productor);

  // @@protoc_insertion_point(class_scope:alarm_center_video.dev_info)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr user_;
  ::google::protobuf::internal::ArenaStringPtr note_;
  ::google::protobuf::internal::ArenaStringPtr productor_;
  ::google::protobuf::int32 dev_id_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_AssignDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_ShutdownFile_alarm_5fcenter_5fvideo_2eproto();

  void InitAsDefaultInstance();
  static dev_info* default_instance_;
};
// -------------------------------------------------------------------

class alarm_info : public ::google::protobuf::Message {
 public:
  alarm_info();
  virtual ~alarm_info();

  alarm_info(const alarm_info& from);

  inline alarm_info& operator=(const alarm_info& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const alarm_info& default_instance();

  void Swap(alarm_info* other);

  // implements Message ----------------------------------------------

  inline alarm_info* New() const { return New(NULL); }

  alarm_info* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const alarm_info& from);
  void MergeFrom(const alarm_info& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(alarm_info* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .alarm_center_video.dev_info devinfo = 1;
  bool has_devinfo() const;
  void clear_devinfo();
  static const int kDevinfoFieldNumber = 1;
  const ::alarm_center_video::dev_info& devinfo() const;
  ::alarm_center_video::dev_info* mutable_devinfo();
  ::alarm_center_video::dev_info* release_devinfo();
  void set_allocated_devinfo(::alarm_center_video::dev_info* devinfo);

  // repeated string alarm_msg = 2;
  int alarm_msg_size() const;
  void clear_alarm_msg();
  static const int kAlarmMsgFieldNumber = 2;
  const ::std::string& alarm_msg(int index) const;
  ::std::string* mutable_alarm_msg(int index);
  void set_alarm_msg(int index, const ::std::string& value);
  void set_alarm_msg(int index, const char* value);
  void set_alarm_msg(int index, const char* value, size_t size);
  ::std::string* add_alarm_msg();
  void add_alarm_msg(const ::std::string& value);
  void add_alarm_msg(const char* value);
  void add_alarm_msg(const char* value, size_t size);
  const ::google::protobuf::RepeatedPtrField< ::std::string>& alarm_msg() const;
  ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_alarm_msg();

  // @@protoc_insertion_point(class_scope:alarm_center_video.alarm_info)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::alarm_center_video::dev_info* devinfo_;
  ::google::protobuf::RepeatedPtrField< ::std::string> alarm_msg_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_AssignDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_ShutdownFile_alarm_5fcenter_5fvideo_2eproto();

  void InitAsDefaultInstance();
  static alarm_info* default_instance_;
};
// -------------------------------------------------------------------

class hisroty_record : public ::google::protobuf::Message {
 public:
  hisroty_record();
  virtual ~hisroty_record();

  hisroty_record(const hisroty_record& from);

  inline hisroty_record& operator=(const hisroty_record& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const hisroty_record& default_instance();

  void Swap(hisroty_record* other);

  // implements Message ----------------------------------------------

  inline hisroty_record* New() const { return New(NULL); }

  hisroty_record* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const hisroty_record& from);
  void MergeFrom(const hisroty_record& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(hisroty_record* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 ademco_id = 1;
  void clear_ademco_id();
  static const int kAdemcoIdFieldNumber = 1;
  ::google::protobuf::int32 ademco_id() const;
  void set_ademco_id(::google::protobuf::int32 value);

  // optional int32 zone_value = 2;
  void clear_zone_value();
  static const int kZoneValueFieldNumber = 2;
  ::google::protobuf::int32 zone_value() const;
  void set_zone_value(::google::protobuf::int32 value);

  // optional string record = 3;
  void clear_record();
  static const int kRecordFieldNumber = 3;
  const ::std::string& record() const;
  void set_record(const ::std::string& value);
  void set_record(const char* value);
  void set_record(const char* value, size_t size);
  ::std::string* mutable_record();
  ::std::string* release_record();
  void set_allocated_record(::std::string* record);

  // optional string timestamp = 4;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 4;
  const ::std::string& timestamp() const;
  void set_timestamp(const ::std::string& value);
  void set_timestamp(const char* value);
  void set_timestamp(const char* value, size_t size);
  ::std::string* mutable_timestamp();
  ::std::string* release_timestamp();
  void set_allocated_timestamp(::std::string* timestamp);

  // @@protoc_insertion_point(class_scope:alarm_center_video.hisroty_record)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 ademco_id_;
  ::google::protobuf::int32 zone_value_;
  ::google::protobuf::internal::ArenaStringPtr record_;
  ::google::protobuf::internal::ArenaStringPtr timestamp_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_AssignDesc_alarm_5fcenter_5fvideo_2eproto();
  friend void protobuf_ShutdownFile_alarm_5fcenter_5fvideo_2eproto();

  void InitAsDefaultInstance();
  static hisroty_record* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// request

// optional string place_holder = 1;
inline void request::clear_place_holder() {
  place_holder_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& request::place_holder() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.request.place_holder)
  return place_holder_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void request::set_place_holder(const ::std::string& value) {
  
  place_holder_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.request.place_holder)
}
inline void request::set_place_holder(const char* value) {
  
  place_holder_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.request.place_holder)
}
inline void request::set_place_holder(const char* value, size_t size) {
  
  place_holder_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.request.place_holder)
}
inline ::std::string* request::mutable_place_holder() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.request.place_holder)
  return place_holder_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* request::release_place_holder() {
  // @@protoc_insertion_point(field_release:alarm_center_video.request.place_holder)
  
  return place_holder_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void request::set_allocated_place_holder(::std::string* place_holder) {
  if (place_holder != NULL) {
    
  } else {
    
  }
  place_holder_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), place_holder);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.request.place_holder)
}

// -------------------------------------------------------------------

// reply

// optional string place_holder = 1;
inline void reply::clear_place_holder() {
  place_holder_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& reply::place_holder() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.reply.place_holder)
  return place_holder_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void reply::set_place_holder(const ::std::string& value) {
  
  place_holder_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.reply.place_holder)
}
inline void reply::set_place_holder(const char* value) {
  
  place_holder_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.reply.place_holder)
}
inline void reply::set_place_holder(const char* value, size_t size) {
  
  place_holder_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.reply.place_holder)
}
inline ::std::string* reply::mutable_place_holder() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.reply.place_holder)
  return place_holder_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* reply::release_place_holder() {
  // @@protoc_insertion_point(field_release:alarm_center_video.reply.place_holder)
  
  return place_holder_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void reply::set_allocated_place_holder(::std::string* place_holder) {
  if (place_holder != NULL) {
    
  } else {
    
  }
  place_holder_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), place_holder);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.reply.place_holder)
}

// -------------------------------------------------------------------

// dev_info

// optional int32 dev_id = 1;
inline void dev_info::clear_dev_id() {
  dev_id_ = 0;
}
inline ::google::protobuf::int32 dev_info::dev_id() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.dev_info.dev_id)
  return dev_id_;
}
inline void dev_info::set_dev_id(::google::protobuf::int32 value) {
  
  dev_id_ = value;
  // @@protoc_insertion_point(field_set:alarm_center_video.dev_info.dev_id)
}

// optional string user = 2;
inline void dev_info::clear_user() {
  user_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& dev_info::user() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.dev_info.user)
  return user_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void dev_info::set_user(const ::std::string& value) {
  
  user_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.dev_info.user)
}
inline void dev_info::set_user(const char* value) {
  
  user_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.dev_info.user)
}
inline void dev_info::set_user(const char* value, size_t size) {
  
  user_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.dev_info.user)
}
inline ::std::string* dev_info::mutable_user() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.dev_info.user)
  return user_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* dev_info::release_user() {
  // @@protoc_insertion_point(field_release:alarm_center_video.dev_info.user)
  
  return user_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void dev_info::set_allocated_user(::std::string* user) {
  if (user != NULL) {
    
  } else {
    
  }
  user_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), user);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.dev_info.user)
}

// optional string note = 3;
inline void dev_info::clear_note() {
  note_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& dev_info::note() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.dev_info.note)
  return note_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void dev_info::set_note(const ::std::string& value) {
  
  note_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.dev_info.note)
}
inline void dev_info::set_note(const char* value) {
  
  note_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.dev_info.note)
}
inline void dev_info::set_note(const char* value, size_t size) {
  
  note_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.dev_info.note)
}
inline ::std::string* dev_info::mutable_note() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.dev_info.note)
  return note_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* dev_info::release_note() {
  // @@protoc_insertion_point(field_release:alarm_center_video.dev_info.note)
  
  return note_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void dev_info::set_allocated_note(::std::string* note) {
  if (note != NULL) {
    
  } else {
    
  }
  note_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), note);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.dev_info.note)
}

// optional string productor = 4;
inline void dev_info::clear_productor() {
  productor_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& dev_info::productor() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.dev_info.productor)
  return productor_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void dev_info::set_productor(const ::std::string& value) {
  
  productor_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.dev_info.productor)
}
inline void dev_info::set_productor(const char* value) {
  
  productor_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.dev_info.productor)
}
inline void dev_info::set_productor(const char* value, size_t size) {
  
  productor_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.dev_info.productor)
}
inline ::std::string* dev_info::mutable_productor() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.dev_info.productor)
  return productor_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* dev_info::release_productor() {
  // @@protoc_insertion_point(field_release:alarm_center_video.dev_info.productor)
  
  return productor_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void dev_info::set_allocated_productor(::std::string* productor) {
  if (productor != NULL) {
    
  } else {
    
  }
  productor_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), productor);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.dev_info.productor)
}

// -------------------------------------------------------------------

// alarm_info

// optional .alarm_center_video.dev_info devinfo = 1;
inline bool alarm_info::has_devinfo() const {
  return !_is_default_instance_ && devinfo_ != NULL;
}
inline void alarm_info::clear_devinfo() {
  if (GetArenaNoVirtual() == NULL && devinfo_ != NULL) delete devinfo_;
  devinfo_ = NULL;
}
inline const ::alarm_center_video::dev_info& alarm_info::devinfo() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.alarm_info.devinfo)
  return devinfo_ != NULL ? *devinfo_ : *default_instance_->devinfo_;
}
inline ::alarm_center_video::dev_info* alarm_info::mutable_devinfo() {
  
  if (devinfo_ == NULL) {
    devinfo_ = new ::alarm_center_video::dev_info;
  }
  // @@protoc_insertion_point(field_mutable:alarm_center_video.alarm_info.devinfo)
  return devinfo_;
}
inline ::alarm_center_video::dev_info* alarm_info::release_devinfo() {
  // @@protoc_insertion_point(field_release:alarm_center_video.alarm_info.devinfo)
  
  ::alarm_center_video::dev_info* temp = devinfo_;
  devinfo_ = NULL;
  return temp;
}
inline void alarm_info::set_allocated_devinfo(::alarm_center_video::dev_info* devinfo) {
  delete devinfo_;
  devinfo_ = devinfo;
  if (devinfo) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.alarm_info.devinfo)
}

// repeated string alarm_msg = 2;
inline int alarm_info::alarm_msg_size() const {
  return alarm_msg_.size();
}
inline void alarm_info::clear_alarm_msg() {
  alarm_msg_.Clear();
}
inline const ::std::string& alarm_info::alarm_msg(int index) const {
  // @@protoc_insertion_point(field_get:alarm_center_video.alarm_info.alarm_msg)
  return alarm_msg_.Get(index);
}
inline ::std::string* alarm_info::mutable_alarm_msg(int index) {
  // @@protoc_insertion_point(field_mutable:alarm_center_video.alarm_info.alarm_msg)
  return alarm_msg_.Mutable(index);
}
inline void alarm_info::set_alarm_msg(int index, const ::std::string& value) {
  // @@protoc_insertion_point(field_set:alarm_center_video.alarm_info.alarm_msg)
  alarm_msg_.Mutable(index)->assign(value);
}
inline void alarm_info::set_alarm_msg(int index, const char* value) {
  alarm_msg_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:alarm_center_video.alarm_info.alarm_msg)
}
inline void alarm_info::set_alarm_msg(int index, const char* value, size_t size) {
  alarm_msg_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.alarm_info.alarm_msg)
}
inline ::std::string* alarm_info::add_alarm_msg() {
  // @@protoc_insertion_point(field_add_mutable:alarm_center_video.alarm_info.alarm_msg)
  return alarm_msg_.Add();
}
inline void alarm_info::add_alarm_msg(const ::std::string& value) {
  alarm_msg_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:alarm_center_video.alarm_info.alarm_msg)
}
inline void alarm_info::add_alarm_msg(const char* value) {
  alarm_msg_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:alarm_center_video.alarm_info.alarm_msg)
}
inline void alarm_info::add_alarm_msg(const char* value, size_t size) {
  alarm_msg_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:alarm_center_video.alarm_info.alarm_msg)
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
alarm_info::alarm_msg() const {
  // @@protoc_insertion_point(field_list:alarm_center_video.alarm_info.alarm_msg)
  return alarm_msg_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
alarm_info::mutable_alarm_msg() {
  // @@protoc_insertion_point(field_mutable_list:alarm_center_video.alarm_info.alarm_msg)
  return &alarm_msg_;
}

// -------------------------------------------------------------------

// hisroty_record

// optional int32 ademco_id = 1;
inline void hisroty_record::clear_ademco_id() {
  ademco_id_ = 0;
}
inline ::google::protobuf::int32 hisroty_record::ademco_id() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.hisroty_record.ademco_id)
  return ademco_id_;
}
inline void hisroty_record::set_ademco_id(::google::protobuf::int32 value) {
  
  ademco_id_ = value;
  // @@protoc_insertion_point(field_set:alarm_center_video.hisroty_record.ademco_id)
}

// optional int32 zone_value = 2;
inline void hisroty_record::clear_zone_value() {
  zone_value_ = 0;
}
inline ::google::protobuf::int32 hisroty_record::zone_value() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.hisroty_record.zone_value)
  return zone_value_;
}
inline void hisroty_record::set_zone_value(::google::protobuf::int32 value) {
  
  zone_value_ = value;
  // @@protoc_insertion_point(field_set:alarm_center_video.hisroty_record.zone_value)
}

// optional string record = 3;
inline void hisroty_record::clear_record() {
  record_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& hisroty_record::record() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.hisroty_record.record)
  return record_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void hisroty_record::set_record(const ::std::string& value) {
  
  record_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.hisroty_record.record)
}
inline void hisroty_record::set_record(const char* value) {
  
  record_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.hisroty_record.record)
}
inline void hisroty_record::set_record(const char* value, size_t size) {
  
  record_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.hisroty_record.record)
}
inline ::std::string* hisroty_record::mutable_record() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.hisroty_record.record)
  return record_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* hisroty_record::release_record() {
  // @@protoc_insertion_point(field_release:alarm_center_video.hisroty_record.record)
  
  return record_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void hisroty_record::set_allocated_record(::std::string* record) {
  if (record != NULL) {
    
  } else {
    
  }
  record_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), record);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.hisroty_record.record)
}

// optional string timestamp = 4;
inline void hisroty_record::clear_timestamp() {
  timestamp_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& hisroty_record::timestamp() const {
  // @@protoc_insertion_point(field_get:alarm_center_video.hisroty_record.timestamp)
  return timestamp_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void hisroty_record::set_timestamp(const ::std::string& value) {
  
  timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:alarm_center_video.hisroty_record.timestamp)
}
inline void hisroty_record::set_timestamp(const char* value) {
  
  timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:alarm_center_video.hisroty_record.timestamp)
}
inline void hisroty_record::set_timestamp(const char* value, size_t size) {
  
  timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:alarm_center_video.hisroty_record.timestamp)
}
inline ::std::string* hisroty_record::mutable_timestamp() {
  
  // @@protoc_insertion_point(field_mutable:alarm_center_video.hisroty_record.timestamp)
  return timestamp_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* hisroty_record::release_timestamp() {
  // @@protoc_insertion_point(field_release:alarm_center_video.hisroty_record.timestamp)
  
  return timestamp_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void hisroty_record::set_allocated_timestamp(::std::string* timestamp) {
  if (timestamp != NULL) {
    
  } else {
    
  }
  timestamp_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), timestamp);
  // @@protoc_insertion_point(field_set_allocated:alarm_center_video.hisroty_record.timestamp)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace alarm_center_video

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_alarm_5fcenter_5fvideo_2eproto__INCLUDED
