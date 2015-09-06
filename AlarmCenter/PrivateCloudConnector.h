#pragma once

namespace core {
namespace video {
namespace ezviz {
class CPrivateCloudConnector
{
private:
	std::string _ip;
	int _port;
	std::string _appKey;
	
public:

	bool get_accToken(std::string& accToken,
					  const std::string& phone,
					  const std::string& user_id);

	DECLARE_GETTER_SETTER(std::string, _ip);
	DECLARE_GETTER_SETTER_INT(_port);
	DECLARE_GETTER_SETTER(std::string, _appKey);

	//CPrivateCloudConnector();
	~CPrivateCloudConnector();
	DECLARE_UNCOPYABLE(CPrivateCloudConnector)
	DECLARE_SINGLETON(CPrivateCloudConnector)
};

NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
