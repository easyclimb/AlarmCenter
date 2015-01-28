#include "stdafx.h"
#include "ConfigHelper.h"
#include "./tinyxml/tinyxml.h"
using namespace tinyxml;

IMPLEMENT_SINGLETON(CConfigHelper)
#define VALUE "val"

CConfigHelper::CConfigHelper()
	: _file()
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

	TiXmlElement *elem = new TiXmlElement("language");
	elem->SetAttribute(VALUE, "Chinese");
	root->LinkEndChild(elem);
	//TiXmlText* text = new TiXmlText("Chinese");
	//elem->LinkEndChild(text);
	
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
