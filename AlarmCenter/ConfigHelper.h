#pragma once
#include <string>

namespace util {

enum ApplicationLanguage
{
	AL_CHINESE = 0,
	AL_ENGLISH,
	AL_TAIWANESE,
};

class CConfigHelper
{
public:
	ApplicationLanguage GetLanguage() { return _lang; }
	bool SetValue(const char* name, const char* value);
	const char* GetValue(const char* name);
	~CConfigHelper();
private:
	std::string _file;
	ApplicationLanguage _lang;
protected:
	void InitializeDefaultConfiguration();
	void LoadLanguage();
	DECLARE_UNCOPYABLE(CConfigHelper)
	DECLARE_SINGLETON(CConfigHelper)
};

NAMESPACE_END

