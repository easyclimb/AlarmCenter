#pragma once

#include <list>

namespace core
{

// 探头种类
enum DetectorType {
	DT_SINGLE = 0,	// 独立探头
	DT_DOUBLE,		// 对射探头
	DT_MAX,
};

// 射线条数
enum AntLineNum
{
	ALN_0 = 0, // 无射线时(独立探头)设置此值
	ALN_1,
	ALN_2,
	ALN_3,
	ALN_4,
	ALN_5,
	ALN_6,
	ALN_7,
	ALN_8,
	ALN_MAX,
};

// 射线间距(单位：像素)
enum AntLineGap {
	ALG_0 = 0, // 无射线时设置此值
	ALG_12 = 12,
	ALG_14 = 14,
	ALG_16 = 16,
};

class CDetectorLibData {
	DECLARE_UNCOPYABLE(CDetectorLibData)
private:
	int _id;
	DetectorType _type;
	wchar_t* _detector_name;
	wchar_t* _path;
	wchar_t* _path_pair;
	int _antline_num;
	int _antline_gap;
public:
	CDetectorLibData() : _id(0), _type(DT_SINGLE), _detector_name(NULL),
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

	void set_type(int type) { _type = IntegerToDetectorType(type); }
	DetectorType get_type() const { return _type; }

	DECLARE_GETTER_SETTER_INT(_id);
	//DECLARE_GETTER_SETTER_INT(_type);
	DECLARE_GETTER_SETTER_INT(_antline_num);
	DECLARE_GETTER_SETTER_INT(_antline_gap);

	DECLARE_GETTER_SETTER_STRING(_detector_name);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER_SETTER_STRING(_path_pair);
protected:
	static DetectorType IntegerToDetectorType(int type) {
		switch (type) {
			case DT_SINGLE:		return DT_SINGLE;	break;
			case DT_DOUBLE:		return DT_DOUBLE;	break;
			default:			return DT_MAX;		break;
		}
	}
};

class CDetectorLib
{
	DECLARE_UNCOPYABLE(CDetectorLib)
private:
	std::list<CDetectorLibData*> m_detectorLibDataList;
public:
	int GetLibCount() const { m_detectorLibDataList.size(); }
	void GetAllLibData(std::list<CDetectorLibData*>& list);
	void AddDetectorLibData(CDetectorLibData* data) { m_detectorLibDataList.push_back(data); }
	const CDetectorLibData* GetDetectorLibData(int id);
	~CDetectorLib();
	DECLARE_SINGLETON(CDetectorLib)
};







NAMESPACE_END

