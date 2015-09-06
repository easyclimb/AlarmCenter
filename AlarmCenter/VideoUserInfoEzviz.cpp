#include "stdafx.h"
#include "VideoUserInfoEzviz.h"
#include "VideoInfo.h"

namespace core {
namespace video {
namespace ezviz {
CVideoUserInfoEzviz::CVideoUserInfoEzviz()
	: _user_phone()
	, _user_accToken()
{}


CVideoUserInfoEzviz::~CVideoUserInfoEzviz()
{}


bool CVideoUserInfoEzviz::execute_set_user_accToken(const std::string& accToken)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString sql; 
	sql.Format(L"update user_info set user_accToken='%s' where ID=%d",
			   A2W(accToken.c_str()), _id);
	if (CVideoManager::GetInstance()->Execute(sql)) {
		set_user_accToken(accToken);
		return true;
	}
	return false;
}




NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
