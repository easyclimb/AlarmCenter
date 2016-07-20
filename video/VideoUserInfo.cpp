#include "stdafx.h"
#include "VideoUserInfo.h"
#include "VideoDeviceInfo.h"
#include <iterator>

namespace video {

//productor_info user::_productorInfo(ProductorUnknown);

user::user()
{}


user::~user()
{	

}


void user::GetDeviceList(device_list& list)
{
	std::copy(device_list_.begin(), device_list_.end(), std::back_inserter(list));
}




};
