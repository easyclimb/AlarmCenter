#pragma once

namespace core {
class CGroupInfo
{
private:
	int _id;
	int _parent_id;
	wchar_t* _name;
public:
	CGroupInfo();
	~CGroupInfo();
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_parent_id);
	DECLARE_GETTER_SETTER_STRING(_name);
};







NAMESPACE_END

