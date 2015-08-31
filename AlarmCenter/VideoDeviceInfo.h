#pragma once
#include "video.h"

namespace core {
namespace video {
class CVideoDeviceInfo
{
protected:
	int _id;
	std::string _note;
public:

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER(std::string, _note);

	CVideoDeviceInfo();
	virtual ~CVideoDeviceInfo();
};
NAMESPACE_END
NAMESPACE_END
