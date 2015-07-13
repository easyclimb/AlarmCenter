#pragma once

typedef enum RemoteControlCommandConn
{
	RCCC_HANGUP,
	RCCC_RESUME,
	RCCC_DISCONN,
}RemoteControlCommandConn;

typedef void(__stdcall *RemoteControlCommandConnCB)(void* udata, RemoteControlCommandConn rccc);
typedef struct RemoteControlCommandConnObj
{
	void* udata;
	RemoteControlCommandConnCB cb;
	RemoteControlCommandConnObj() : udata(NULL), cb(NULL) {}
	void reset() { udata = NULL; cb = NULL; }
	bool valid() const { return ((udata != NULL) && (cb != NULL)); }
}RemoteControlCommandConnObj;
