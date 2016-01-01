#pragma once
#include <list>
#include "core.h"

namespace core {

typedef struct AlarmText {
	int _zone;
	int _subzone;
	int _event;
	CString _txt;
	AlarmText() : _zone(0), _subzone(0), _event(0), _txt(_T("")) { AUTO_LOG_FUNCTION; JLOG(L"%p", this); }
	AlarmText(const AlarmText& rhs) : _zone(rhs._zone), _subzone(rhs._subzone), _event(rhs._event), _txt(rhs._txt) { AUTO_LOG_FUNCTION; JLOG(L"%p", this); }
	
	AlarmText& operator=(const AlarmText& rhs) {
		_zone = rhs._zone;
		_subzone = rhs._subzone;
		_event = rhs._event;
		_txt = rhs._txt;
		return *this;
	}
}AlarmText;

// ������Ƶ�ͼʵ������
enum InversionControlMapCommand {
	ICMC_SHOW,				// ��ʾ��ͼ
	ICMC_ADD_ALARM_TEXT,	// ��ӱ������ֲ���ʾ(�踽�Ӳ���AlarmText)
	ICMC_DEL_ALARM_TEXT,	// ɾ����������
	ICMC_CLR_ALARM_TEXT,	// �����������
	ICMC_MODE_EDIT,			// ����༭ģʽ
	ICMC_MODE_NORMAL,		// �˳��༭ģʽ
	ICMC_RENAME,			// ������
	ICMC_CHANGE_IMAGE,		// ����ͼƬ
	ICMC_NEW_DETECTOR,		// ����̽ͷ
	ICMC_DEL_DETECTOR,		// ɾ��̽ͷ
	ICMC_DESTROY,			// �ͷŶ��Լ�������
};

typedef void(__stdcall *OnInversionControlMapCB)(void* udata,
												 InversionControlMapCommand icmc,
												 const AlarmText* at);

enum MapType {
	MAP_MACHINE,
	MAP_SUB_MACHINE,
};



class CMapInfo
{
	//const char *__class_name;
private:
	int _id;
	MapType _type;
	int _machine_id;
	CString _alias;
	CString _path;
	std::list<CDetectorBindInterfacePtr> _interfaceList;
	std::list<AlarmText*> _alarmTextList;
	CLock _lock4AlarmTextList;
	void* _udata;
	OnInversionControlMapCB _cb;
	bool _alarming;
	CDetectorInfoList _noZoneDetectorList;
	CDetectorBindInterfacePtr _activeInterface;
public:
	CMapInfo();
	~CMapInfo();
	void AddInterface(CDetectorBindInterfacePtr pInterface) { _interfaceList.push_back(pInterface); }
	void RemoveInterface(CDetectorBindInterfacePtr pInterface) { _interfaceList.remove(pInterface); }
	void GetAllInterfaceInfo(std::list<CDetectorBindInterfacePtr>& list);
	CDetectorBindInterfacePtr GetActiveInterfaceInfo() { return _activeInterface; }
	void SetActiveInterfaceInfo(CDetectorBindInterfacePtr pInterface) { _activeInterface = pInterface; }

	DECLARE_GETTER_SETTER_INT(_id);
	void set_type(int type) { _type = Integer2MapType(type); }
	MapType get_type() const { return _type; }

	// 2015��3��20�� 16:33:54 �����Ѿ����ͼ�󶨣���δ������󶨵�̽ͷ
	void AddNoZoneDetectorInfo(CDetectorInfoPtr detInfo) { _noZoneDetectorList.push_back(detInfo); }
	void GetNoZoneDetectorInfo(CDetectorInfoList& list);
	void RemoveNoZoneDetectorInfo(CDetectorInfoPtr detInfo) { _noZoneDetectorList.remove(detInfo); }

	bool execute_delete_no_zone_detector_info(CDetectorInfoPtr detInfo);

	DECLARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER(bool, _alarming);

	void SetInversionControlCallBack(void* udata, OnInversionControlMapCB cb);
	//void AddNewAlarmText(AlarmText* at);
	void TraverseAlarmText(void* udata, OnInversionControlMapCB cb);

	// 2015��3��20�� 17:20:03 ���ӷ������mapViewʵ�������
	void InversionControl(InversionControlMapCommand icmc, AlarmText* at = nullptr);

protected:
	static MapType Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}

	void clear_alarm_text_list();
	
	DECLARE_UNCOPYABLE(CMapInfo)
};

NAMESPACE_END
