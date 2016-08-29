#pragma once

#include "core.h"

namespace core
{

// 探头种类
enum detector_type {
	DT_SINGLE		= 1,	// 独立探头
	DT_DOUBLE		= 2,	// 对射探头
	DT_SUB_MACHINE	= 4,	// 分机
	DT_CAMERA		= 8,	// 摄像头
	DT_MAX			= 16,
};

// 探头编号
enum detector_index {
	DI_CAMERA = 27,			// 摄像机
};

// 射线条数
enum ant_line_num
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
enum ant_line_gap {
	ALG_0 = 0, // 无射线时设置此值
	ALG_12 = 12,
	ALG_14 = 14,
	ALG_16 = 16,
};


class detector_lib_data : private boost::noncopyable
{
private:
	int _id;
	detector_type _type;
	CString _detector_name;
	CString _path;
	CString _path_pair;
	int _antline_num;
	int _antline_gap;

public:
	detector_lib_data() : _id(0), _type(DT_SINGLE), _detector_name(),
						_path(), _path_pair(), 
						_antline_num(0), _antline_gap(0)
	{}

	~detector_lib_data()
	{}

	void set_type(int type) { _type = IntegerToDetectorType(type); }
	detector_type get_type() const { return _type; }

	DECLARE_GETTER_SETTER_INT(_id);
	//DECLARE_GETTER_SETTER_INT(_type);
	DECLARE_GETTER_SETTER_INT(_antline_num);
	DECLARE_GETTER_SETTER_INT(_antline_gap);

	DECLARE_GETTER_SETTER_STRING(_detector_name);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER_SETTER_STRING(_path_pair);
protected:
	static detector_type IntegerToDetectorType(int type) {
		switch (type) {
			case DT_SINGLE:		return DT_SINGLE;	break;
			case DT_DOUBLE:		return DT_DOUBLE;	break;
			case DT_SUB_MACHINE:	return DT_SUB_MACHINE;	break;
			case DT_CAMERA:		return DT_CAMERA;	break;
			default:			return DT_MAX;		break;
		}
	}
};


static const int g_obsolete_detecotr_lib_ids[] = { 1, 2, 10, 14, 20 };

inline bool is_obsolete_detector_lib_id(int id) {
	for (auto i : g_obsolete_detecotr_lib_ids) {
		if (id == i) {
			return true;
		}
	}
	return false;
}

class detector_lib_manager : public dp::singleton<detector_lib_manager>
{
private:
	std::list<detector_lib_data_ptr> m_detectorLibDataList;

protected:
	detector_lib_manager();

public:
	int GetLibCount() const { m_detectorLibDataList.size(); }
	void GetAllLibData(std::list<detector_lib_data_ptr>& list);
	void AddDetectorLibData(const detector_lib_data_ptr& data) { m_detectorLibDataList.push_back(data); }
	const detector_lib_data_ptr GetDetectorLibData(int id);
	~detector_lib_manager();
	//DECLARE_SINGLETON(detector_lib_manager)
};







};


