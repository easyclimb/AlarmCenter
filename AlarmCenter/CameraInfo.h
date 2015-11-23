#pragma once

#include "DetectorBindInterface.h"

namespace core {

	class CCameraInfo : public CDetectorBindInterface
	{
	public:
		explicit CCameraInfo() : CDetectorBindInterface() {}
		virtual ~CCameraInfo() {}

		void SetDetectorInfo(CDetectorInfo* detectorInfo) {
			_detectorInfo = detectorInfo;
		}
		// CDetectorBindInterface methods:
		virtual CDetectorInfo* GetDetectorInfo() const override { return _detectorInfo; }
		virtual std::wstring FormatTooltip() const override;
		virtual void DoClick() override {};
		virtual void DoRClick() override {};
		virtual void SetInversionControlCallback(void*, OnInversionControlZoneCB) override {}
		virtual bool get_alarming() const override { return false; }
		virtual DetectorInterfaceType GetInterfaceType() const override { return m_dit; }

		DECLARE_GETTER_SETTER_INT(_ademco_id);
		DECLARE_GETTER_SETTER_INT(_sub_machine_id);
		DECLARE_GETTER_SETTER_INT(_device_info_id);
		DECLARE_GETTER_SETTER_INT(_productor);
	private:
		int _ademco_id = -1;
		int _sub_machine_id = -1;
		CDetectorInfo* _detectorInfo = nullptr;
		int _device_info_id = -1;
		int _productor = -1;
		static const DetectorInterfaceType m_dit = DIT_CAMERA_INFO;
	};

};
