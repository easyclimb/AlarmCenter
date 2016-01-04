#pragma once

#include <list>

#include "core.h"

namespace core
{
	

	typedef enum DetectorInterfaceType
	{
		DIT_ZONE_INFO = 0,
		DIT_CAMERA_INFO = 1,
	}DetectorInterfaceType;

	class CDetectorBindInterface
	{
	public:
		explicit CDetectorBindInterface()
			: _detectorInfo(nullptr)
			, _udata()
			, _cb(nullptr)
		{}
		virtual ~CDetectorBindInterface();

		virtual void SetDetectorInfo(const CDetectorInfoPtr& detectorInfo) { _detectorInfo = detectorInfo; }
		virtual CDetectorInfoPtr GetDetectorInfo() const { return _detectorInfo; }
		virtual bool get_alarming() const = 0;
		virtual std::wstring FormatTooltip() const = 0;
		virtual void DoClick() = 0;
		virtual void DoRClick() = 0;
		virtual DetectorInterfaceType GetInterfaceType() const = 0;
		virtual void SetInversionControlCallback(CDetectorWeakPtr udata, OnInversionControlZoneCB cb) {
			_udata = udata; _cb = cb;
			if (!udata.expired() && cb && !_iczcList.empty()) {
				for (auto iczc : _iczcList) {
					cb(udata.lock(), iczc);
				}
				_iczcList.clear();
			}
		}
		virtual void InversionControl(InversionControlZoneCommand iczc)
		{
			AUTO_LOG_FUNCTION;
			if (_cb) {
				_cb(_udata.lock(), std::make_shared<IczcBuffer>(iczc, 0));
			} else {
				_iczcList.push_back(std::make_shared<IczcBuffer>(iczc, 0));
			}
		}

		typedef enum DetectorInfoField {
			DIF_X,
			DIF_Y,
			DIF_DISTANCE,
			DIF_ANGLE,
		}DetectorInfoField;
		virtual bool execute_update_detector_info_field(DetectorInfoField dif, int value);

	protected:
		CDetectorInfoPtr _detectorInfo;
		CDetectorWeakPtr _udata;
		OnInversionControlZoneCB _cb;
		std::list<IczcBufferPtr> _iczcList;
		DECLARE_UNCOPYABLE(CDetectorBindInterface)
	};
	typedef std::list<CDetectorBindInterfacePtr> CDetectorBindInterfaceList;


};
