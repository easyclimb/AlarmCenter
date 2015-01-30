#include "stdafx.h"
#include "ConfigHelper.h"
#include "./tinyxml/tinyxml.h"
using namespace tinyxml;

IMPLEMENT_SINGLETON(CConfigHelper)

#define VALUE		"val"
#define STR_LANG	"language"
#define STR_LANG_CH "Chinese"
#define STR_LANG_EN "English"
#define STR_LANG_TW "Taiwanese"

CConfigHelper::CConfigHelper()
	: _file()
	, _lang(AL_CHINESE)
{
	CString path;
	path.Format(L"%s\\config", GetModuleFilePath());
	CreateDirectory(path, NULL);
	path += L"\\config.xml";
	USES_CONVERSION;
	_file = W2A(path);

	TiXmlDocument doc(_file.c_str());
	if (!doc.LoadFile()) {
		InitializeDefaultConfiguration();
	}

	LoadLanguage();
}


CConfigHelper::~CConfigHelper()
{

}


void CConfigHelper::InitializeDefaultConfiguration()
{
	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("AlarmCenterNet_configuration");
	doc.LinkEndChild(root);

	TiXmlElement *elem = new TiXmlElement(STR_LANG);
	elem->SetAttribute(VALUE, STR_LANG_CH);
	root->LinkEndChild(elem);
	
	doc.SaveFile(_file.c_str());
}


const char* CConfigHelper::GetValue(const char* name)
{
	do {
		TiXmlDocument doc(_file.c_str());
		if (!doc.LoadFile()) 
			break;

		TiXmlElement* root = doc.RootElement();
		if (!root) break;

		TiXmlElement* elem = root->FirstChildElement(name);
		if (!elem) break;

		const char* value = elem->Attribute(VALUE);

		return value ? _strdup(value) : NULL;
	} while (0);

	return NULL;
}


bool CConfigHelper::SetValue(const char* name, const char* value)
{
	do {
		TiXmlDocument doc(_file.c_str());
		if (!doc.LoadFile()) break;

		TiXmlElement* root = doc.RootElement();
		if (!root) break;

		TiXmlElement* elem = root->FirstChildElement(name);
		if (!elem) {
			elem = new TiXmlElement(name);
			root->LinkEndChild(elem);
		}

		elem->SetAttribute(VALUE, value);
		
		return doc.SaveFile();;
	} while (0);

	return false;
}


void CConfigHelper::LoadLanguage()
{
	const char* lang = GetValue(STR_LANG);
	if (lang) {
		if (strcmp(lang, STR_LANG_CH)) {
			_lang = AL_CHINESE;
		} else if (strcmp(lang, STR_LANG_EN) == 0) {
			_lang = AL_ENGLISH;
		} else if (strcmp(lang, STR_LANG_TW) == 0) {
			_lang = AL_TAIWANESE;
		}
	}

	_lang = AL_CHINESE;
}



