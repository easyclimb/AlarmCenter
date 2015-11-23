#pragma once

#include <list>

namespace core
{
	class CDetectorInfo;

	typedef enum InversionControlZoneCommand {
		ICZC_ALARM_START,	// 报警
		ICZC_ALARM_STOP,	// 消警
		ICZC_SET_FOCUS,		// 高亮
		ICZC_KILL_FOCUS,	// 取消高亮
		ICZC_ROTATE,		// 旋转
		ICZC_DISTANCE,		// 调整间距(仅针对对射探头)
		ICZC_MOVE,			// 移动
		ICZC_CLICK,			// 单击
		ICZC_RCLICK,		// 右击
							//ICZC_ALIAS_CHANGED, // 别名已修改
		ICZC_DESTROY,		// CZoneInfo已析构
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
