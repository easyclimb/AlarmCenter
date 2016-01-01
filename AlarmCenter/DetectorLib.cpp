#include "stdafx.h"
#include "DetectorLib.h"
#include <iterator>

namespace core
{

IMPLEMENT_SINGLETON(CDetectorLib)

CDetectorLib::CDetectorLib()
{}


CDetectorLib::~CDetectorLib()
{
	m_detectorLibDataList.clear();
}


const CDetectorLibDataPtr CDetectorLib::GetDetectorLibData(int id)
{
	for (auto data : m_detectorLibDataList) {
		if (data->get_id() == id) {
			return data;
		}
	}
	return nullptr;
}


void CDetectorLib::GetAllLibData(std::list<CDetectorLibDataPtr>& list)
{
	std::copy(m_detectorLibDataList.begin(), m_detectorLibDataList.end(), std::back_inserter(list));
}

NAMESPACE_END
