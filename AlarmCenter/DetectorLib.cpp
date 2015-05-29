#include "stdafx.h"
#include "DetectorLib.h"

namespace core
{

IMPLEMENT_SINGLETON(CDetectorLib)

CDetectorLib::CDetectorLib()
{}


CDetectorLib::~CDetectorLib()
{
	std::list<CDetectorLibData*>::iterator iter = m_detectorLibDataList.begin();
	while (iter != m_detectorLibDataList.end()) {
		CDetectorLibData* data = *iter++;
		delete data;
	}
}


const CDetectorLibData* CDetectorLib::GetDetectorLibData(int id)
{
	std::list<CDetectorLibData*>::iterator iter = m_detectorLibDataList.begin();
	while (iter != m_detectorLibDataList.end()) {
		CDetectorLibData* data = *iter++;
		if (data->get_id() == id) {
			return data;
		}
	}
	return NULL;
}


void CDetectorLib::GetAllLibData(std::list<CDetectorLibData*>& list)
{
	std::list<CDetectorLibData*>::iterator iter = m_detectorLibDataList.begin();
	while (iter != m_detectorLibDataList.end()) {
		CDetectorLibData* data = *iter++;
		list.push_back(data);
	}
}

NAMESPACE_END
