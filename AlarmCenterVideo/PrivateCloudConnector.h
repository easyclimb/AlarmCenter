#pragma once
#include "../video/video.h"

namespace video {
namespace ezviz {
	
class private_cloud_connector : public dp::singleton<private_cloud_connector>
{
public:

	bool get_accToken(const std::string& ip, unsigned int port, 
					  const std::string& appKey,
					  std::string& accToken,
					  const std::string& phone,
					  const std::string& user_id, 
					  msg_type type);

	//private_cloud_connector();
	~private_cloud_connector();
	//DECLARE_SINGLETON(private_cloud_connector)

protected:

	private_cloud_connector();
};

};
};

