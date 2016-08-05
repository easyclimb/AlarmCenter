protoc --cpp_out=. alarm_center_video.proto
protoc --grpc_out=. --plugin=protoc-gen-grpc="D:\dev_libs\google\protoc-3.0.0-beta-3-win32\grpc_cpp_plugin.exe" alarm_center_video.proto
PAUSE