#pragma once
#include "video.h"

namespace video {
namespace ezviz {
	
class CPrivateCloudConnector
{
public:

	bool get_accToken(const std::string& ip, unsigned int port, 
					  const std::string& appKey,
					  std::string& accToken,
					  const std::string& phone,
					  const std::string& user_id, 
					  MsgType type);

	//CPrivateCloudConnector();
	~CPrivateCloudConnector();
	DECLARE_UNCOPYABLE(CPrivateCloudConnector)
	DECLARE_SINGLETON(CPrivateCloudConnector)
};

NAMESPACE_END
NAMESPACE_END
