#pragma once

#include "DetectorBindInterface.h"

namespace core {

	class camera_info : public detector_bind_interface
	{
	public:
		explicit camera_info() : detector_bind_interface() {}
		virtual ~camera_info() {}

		// detector_bind_interface methods:
		virtual std::wstring FormatTooltip() const override;
		virtual void DoClick() override {};
		virtual void DoRClick() override {};
		virtual bool get_alarming() const override { return false; }
		virtual DetectorInterfaceType GetInterfaceType() const override { return m_dit; }

		DECLARE_GETTER_SETTER_INT(_ademco_id);
		DECLARE_GETTER_SETTER_INT(_sub_machine_id);
		DECLARE_GETTER_SETTER_INT(_device_info_id);
		DECLARE_GETTER_SETTER_INT(_productor);
	private:
		int _ademco_id = -1;
		int _sub_machine_id = -1;
		int _device_info_id = -1;
		int _productor = -1;
		static const DetectorInterfaceType m_dit = DIT_CAMERA_INFO;
	};

};
