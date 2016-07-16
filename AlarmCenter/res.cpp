#include "stdafx.h"
#include "res.h"
#include <sstream>
#include <fstream>


using namespace detail;

res::res()
{
}


res::~res()
{
}

bool res::parse(const std::string& content)
{
	std::istringstream in(content);

	std::string line;

	auto npos = std::string::npos;

	while (std::getline(in, line)) {
		auto pos = line.find_first_of('"');
		
		if (pos != npos) {
			line = line.substr(pos + 1);
			pos = line.find(" = ");
			
			if (pos != npos) {
				std::string key, value;
				key = line.substr(0, pos - 1);
				value = line.substr(pos + 4);
				pos = value.find_last_of('"');
				
				if (pos != npos) {
					value = value.substr(0, pos);

					key = string_literal(key);
					value = string_literal(value);

					//auto wkey = utf8::a2w(key);
					auto wvalue = utf8::a2w(value);
					//buff_.push_back(wkey);
					map_[key] = wvalue;
				}
			}
		}
	}

	return !map_.empty();
}

bool res::parse_file(const std::wstring & path)
{
	std::ifstream in(path);
	if (!in)return false;

	std::stringstream ss;
	ss << in.rdbuf();
	in.close();
	return parse(ss.str());
}

std::wstring res::get(const char* id)
{
	/*USES_CONVERSION;
	auto mid = utf8::mbcs_to_utf8(id);
	auto wid = utf8::a2w(mid);*/
	auto iter = map_.find(id);
	if (iter != map_.end()) {
		return iter->second;
	}
	
	return std::wstring(L"invalid string id: ") + utf8::a2w(id);
}

CString res::get_as_cstring(const char* id)
{
	auto iter = map_.find(id);
	if (iter != map_.end()) {
		return CString(iter->second.c_str());
	}
	return CString((L"invalid string id: " + utf8::a2w(id)).c_str());
}
