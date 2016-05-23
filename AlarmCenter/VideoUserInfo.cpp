#include "stdafx.h"
#include "VideoUserInfo.h"
#include "VideoDeviceInfo.h"
#include <iterator>
#include "VideoManager.h"

namespace video {

//productor_info video_user_info::_productorInfo(ProductorUnknown);

video_user_info::video_user_info()
	: _id(0)
	, _user_name()
	, _productorInfo(ProductorUnknown)
	, _deviceList()
{}


video_user_info::~video_user_info()
{	

}


void video_user_info::GetDeviceList(video_device_info_list& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}


bool video_user_info::execute_set_user_name(const std::wstring& name)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"update user_info set user_name='%s' where ID=%d",
			   name.c_str(), _id);
	if (video_manager::GetInstance()->Execute(sql)) {
		set_user_name(name);
		return true;
	}
	return false;
}

};
