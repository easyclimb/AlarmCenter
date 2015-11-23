#include "stdafx.h"
#include "DetectorBindInterface.h"
#include "DetectorInfo.h"

namespace core {

	CDetectorBindInterface::~CDetectorBindInterface() {
		if (_cb) { _cb(_udata, ICZC_DESTROY, 0); }
		_iczcCommandList.clear();
		SAFEDELETEP(_detectorInfo);
	}









};

