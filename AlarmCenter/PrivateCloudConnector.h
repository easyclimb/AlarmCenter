#pragma once
#include "video.h"

namespace video {
namespace ezviz {
	
class CPrivateCloudConnector : private boost::noncopyable
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
	DECLARE_SINGLETON(CPrivateCloudConnector)
};

};
};

