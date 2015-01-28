#pragma once
#include <string>

class CConfigHelper
{
public:
	bool SetValue(const char* name, const char* value);
	const char* GetValue(const char* name);
	~CConfigHelper();
private:
	std::string _file;
protected:
	void InitializeDefaultConfiguration();



	DECLARE_UNCOPYABLE(CConfigHelper)
	DECLARE_SINGLETON(CConfigHelper)
};

