#pragma once
#include <map>

// translate to std::wstring
#define tr(id) res::get_instance()->get(id)

// translate to CString
#define TR(id) res::get_instance()->get_as_cstring(id)


class res : public dp::singleton<res>
{
protected:
	res();
public:
	virtual ~res();

	bool parse(const std::string& content);
	bool parse_file(const std::wstring& path);

	std::wstring get(const char* id);
	CString get_as_cstring(const char* id);


	std::map<std::string, std::wstring>& get_map() { return map_; }

private:
	std::map<std::string, std::wstring> map_ = {};
	//std::list<std::wstring> buff_ = {};
};


namespace detail {

static char strings[] = "ntvbrfa\\?'\"0x";
static char literals[] = "\n\t\v\b\r\f\a\\\?\'\"\0";

inline std::string string_literal(const std::string& txt) {
	auto npos = std::string::npos;
	auto str = txt;
	auto pos = str.find_first_of('\\');
	while (pos != npos) {
		auto c = str.at(pos + 1);
		size_t n = 0;
		for (auto s : strings) {
			if (s == c) {
				auto left = str.substr(0, pos);
				auto right = str.substr(pos + 2);
				str = left + literals[n] + right;
				break;
			}
			n++;
		}

		pos = str.find_first_of('\\');
	}
	return str;
}

inline std::string literal_to_string(const std::string& txt) {
	auto npos = std::string::npos;
	auto str = txt;

	int ndx = 0;
	char local_strings[] = "nr0\\";
	char local_literals[] = "\n\r";
	for (auto l : local_literals) {
		std::string s;
		s.push_back('\\');
		s.push_back(local_strings[ndx]);
		auto pos = str.find_first_of(l);
		std::string left;
		while (pos != npos) {
			left += str.substr(0, pos); 
			left += s;
			str = str.substr(pos + 1);
			//str.replace(pos, pos + 1, s);
			pos = str.find_first_of(l);
		}
		str = left + str;
		ndx++;
	}

	return str;
}

inline void LoadFileInResource(int name, const wchar_t* type, DWORD& size, const char*& data)
{
	HMODULE handle = ::GetModuleHandle(NULL);
	HRSRC rc = ::FindResource(handle, MAKEINTRESOURCE(name), type);
	HGLOBAL rcData = ::LoadResource(handle, rc);
	size = ::SizeofResource(handle, rc);
	data = static_cast<const char*>(::LockResource(rcData));
}

inline std::string load_txt_from_rc(int id) {
	DWORD size = 0;
	const char* data = NULL;
	LoadFileInResource(id, L"TXT", size, data);
	if (size > 3 && static_cast<unsigned char>(data[0]) == 0xEF && static_cast<unsigned char>(data[1]) == 0xBB && static_cast<unsigned char>(data[2]) == 0xBF) {
		std::string res(data + 3, size - 3);
		return res;
	} else {
		std::string res(data, size);
		return res;
	}

}

}

