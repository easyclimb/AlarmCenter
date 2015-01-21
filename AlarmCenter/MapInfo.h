#pragma once

namespace core {

class CMapInfo
{
	DECLARE_UNCOPYABLE(CMapInfo)
private:
	int _id;
	int _type;
	int _ademco_id;
	wchar_t* _path;
public:
	CMapInfo();
	~CMapInfo();
	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_type);
	DEALARE_GETTER_SETTER_INT(_ademco_id);
	DECLARE_STRING_SETTER(_path);
	DECLARE_STRING_GETTER(_path);
};

NAMESPACE_END
