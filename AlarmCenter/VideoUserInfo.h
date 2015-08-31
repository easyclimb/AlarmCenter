#pragma once

#include "video.h"

namespace core {
namespace video {
class CVideoUserInfo
{
protected:
	int _id;
	std::wstring _name;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::wstring, _name);

	CVideoUserInfo();
	virtual ~CVideoUserInfo();
};

NAMESPACE_END
NAMESPACE_END
