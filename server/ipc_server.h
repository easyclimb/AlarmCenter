#pragma once

class ipc_server : public dp::singleton<ipc_server>
{
protected:
	ipc_server();
public:
	
	~ipc_server();
};

