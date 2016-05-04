#pragma once

#include <list>

#include "core.h"

namespace core
{
	

	typedef enum detector_interface_type
	{
		DIT_ZONE_INFO = 0,
		DIT_CAMERA_INFO = 1,
	}detector_interface_type;

	class detector_bind_interface
	{
	public:
		explicit detector_bind_interface()
			: _detectorInfo(nullptr)
			, _udata()
			, _cb(nullptr)
		{}
		virtual ~detector_bind_interface();

		virtual void SetDetectorInfo(const detector_info_ptr& detectorInfo) { _detectorInfo = detectorInfo; }
		virtual detector_info_ptr GetDetectorInfo() const { return _detectorInfo; }
		virtual bool get_alarming() const = 0;
		virtual std::wstring FormatTooltip() const = 0;
		virtual void DoClick() = 0;
		virtual void DoRClick() = 0;
		virtual detector_interface_type GetInterfaceType() const = 0;
		virtual void SetInversionControlCallback(CDetectorWeakPtr udata, OnInversionControlZoneCB cb) {
			_udata = udata; _cb = cb;
			if (!udata.expired() && cb && !_iczcList.empty()) {
				for (auto iczc : _iczcList) {
					cb(udata.lock(), iczc);
				}
				_iczcList.clear();
			}
		}
		virtual void InversionControl(inversion_control_zone_command iczc)
		{
			AUTO_LOG_FUNCTION;
			if (_cb) {
				_cb(_udata.lock(), std::make_shared<iczc_buffer>(iczc, 0));
			} else {
				_iczcList.push_back(std::make_shared<iczc_buffer>(iczc, 0));
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
		detector_info_ptr _detectorInfo;
		CDetectorWeakPtr _udata;
		OnInversionControlZoneCB _cb;
		std::list<iczc_buffer_ptr> _iczcList;
		DECLARE_UNCOPYABLE(detector_bind_interface)
	};
	typedef std::list<detector_bind_interface_ptr> CDetectorBindInterfaceList;


};
