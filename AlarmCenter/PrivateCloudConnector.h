#pragma once
namespace core {
namespace video {

class CPrivateCloudConnector
{
private:
	std::string _ip;
	int _port;
public:

	bool get_accToken(std::string& accToken,
					  const char* phone,
					  const char* user_id,
					  const char* appKey);

	DECLARE_GETTER_SETTER(std::string, _ip);
	DECLARE_GETTER_SETTER_INT(_port);

	//CPrivateCloudConnector();
	~CPrivateCloudConnector();
	DECLARE_UNCOPYABLE(CPrivateCloudConnector)
	DECLARE_SINGLETON(CPrivateCloudConnector)
};


NAMESPACE_END
NAMESPACE_END
