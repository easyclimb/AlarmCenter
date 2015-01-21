#pragma once

#include <list>

namespace core
{

class CDetectorLibData {
	DECLARE_UNCOPYABLE(CDetectorLibData)
private:
	int _id;
	int _type;
	wchar_t* _detector_name;
	wchar_t* _path;
	wchar_t* _path_pair;
	int _antline_num;
	int _antline_gap;
public:
	CDetectorLibData() : _id(0), _type(0), _detector_name(NULL), 
						_path(NULL), _path_pair(NULL), 
						_antline_num(0), _antline_gap(0){
		_detector_name = new wchar_t[1];
		_detector_name[0] = 0;
		_path = new wchar_t[1];
		_path[0] = 0;
		_path_pair = new wchar_t[1];
		_path_pair[0] = 0;
	}

	~CDetectorLibData() {
		if (_detector_name) { delete[] _detector_name; }
		if (_path) { delete[] _path; }
		if (_path_pair) { delete[] _path_pair; }
	}

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_type);
	DEALARE_GETTER_SETTER_INT(_antline_num);
	DEALARE_GETTER_SETTER_INT(_antline_gap);

	DECLARE_STRING_GETTER(_detector_name);
	DECLARE_STRING_SETTER(_detector_name);

	DECLARE_STRING_GETTER(_path);
	DECLARE_STRING_SETTER(_path);

	DECLARE_STRING_GETTER(_path_pair);
	DECLARE_STRING_SETTER(_path_pair);
};

class CDetectorLib
{
	DECLARE_UNCOPYABLE(CDetectorLib)
private:
	std::list<CDetectorLibData*> m_detectorLibDataList;
public:
	void AddDetectorLibData(CDetectorLibData* data) { m_detectorLibDataList.push_back(data); }
	const CDetectorLibData* GetDetectorLibData(int id);
	CDetectorLib();
	~CDetectorLib();
};







NAMESPACE_END

