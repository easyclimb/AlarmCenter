#pragma once


typedef void(__stdcall *ConnHangupCB)(void* udata, bool hangup);
typedef struct ConnHangupObj
{
	void* udata;
	ConnHangupCB cb;
	ConnHangupObj() : udata(NULL), cb(NULL) {}
	void reset() { udata = NULL; cb = NULL; }
	bool valid() const { return ((udata != NULL) && (cb != NULL)); }
}ConnHangupObj;
