#pragma once

#include <list>

namespace core
{
	class CDetectorInfo;

	typedef enum InversionControlZoneCommand {
		ICZC_ALARM_START,	// ����
		ICZC_ALARM_STOP,	// ����
		ICZC_SET_FOCUS,		// ����
		ICZC_KILL_FOCUS,	// ȡ������
		ICZC_ROTATE,		// ��ת
		ICZC_DISTANCE,		// �������(����Զ���̽ͷ)
		ICZC_MOVE,			// �ƶ�
		ICZC_CLICK,			// ����
		ICZC_RCLICK,		// �һ�
							//ICZC_ALIAS_CHANGED, // �������޸�
		ICZC_DESTROY,		// CZoneInfo������
	}InversionControlZoneCommand;

	typedef void(__stdcall *OnInversionControlZoneCB)(void* udata,
													  InversionControlZoneCommand iczc,
													  DWORD dwExtra);

	typedef enum DetectorInterfaceType
	{
		DIT_ZONE_INFO = 0,
		DIT_CAMERA_INFO = 1,
	}DetectorInterfaceType;

	class CDetectorBindInterface
	{
	public:
		explicit CDetectorBindInterface()
			: _udata(nullptr)
			, _cb(nullptr)
		{}
		virtual ~CDetectorBindInterface() {}
		virtual CDetectorInfo* GetDetectorInfo() const = 0;
		virtual bool get_alarming() const = 0;
		virtual std::wstring FormatTooltip() const = 0;
		virtual void DoClick() = 0;
		virtual void DoRClick() = 0;
		virtual DetectorInterfaceType GetInterfaceType() const = 0;
		virtual void SetInversionControlCallback(void* udata, OnInversionControlZoneCB cb) {
			_udata = udata; _cb = cb;
			if (udata && cb && _iczcCommandList.size() > 0) {
				for (auto iczc : _iczcCommandList) {
					cb(udata, iczc, 0);
				}
				_iczcCommandList.clear();
			}
		}
		virtual void InversionControl(InversionControlZoneCommand iczc)
		{
			AUTO_LOG_FUNCTION;
			if (_cb) {
				_cb(_udata, iczc, 0);
			} else {
				_iczcCommandList.push_back(iczc);
			}
		}
	protected:
		void* _udata;
		OnInversionControlZoneCB _cb;
		std::list<InversionControlZoneCommand> _iczcCommandList;
		DECLARE_UNCOPYABLE(CDetectorBindInterface)
	};
	typedef std::list<CDetectorBindInterface*> CDetectorBindInterfaceList;


};
