#include "stdafx.h"
#include "DetectorLib.h"
#include <iterator>

namespace core
{

IMPLEMENT_SINGLETON(detector_lib_manager)

detector_lib_manager::detector_lib_manager()
{}


detector_lib_manager::~detector_lib_manager()
{
	m_detectorLibDataList.clear();
}


const detector_lib_data_ptr detector_lib_manager::GetDetectorLibData(int id)
{
	for (auto data : m_detectorLibDataList) {
		if (data->get_id() == id) {
			return data;
		}
	}
	return nullptr;
}


void detector_lib_manager::GetAllLibData(std::list<detector_lib_data_ptr>& list)
{
	std::copy(m_detectorLibDataList.begin(), m_detectorLibDataList.end(), std::back_inserter(list));
}

};

