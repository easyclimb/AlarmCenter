// AlarmCenterDaemon.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AlarmCenterDaemon.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <list>
#include <cstddef>
#include <wininet.h>
#pragma comment( lib, "wininet.lib" )
#include "C:/dev/Global/utf8.h"
#include "C:/dev/Global/chrono_wrapper.h"
#include "C:/dev/Global/win32.h"
#include <Windows.h>
#include <Commctrl.h>
#pragma comment(lib, "comctl32.lib")

#ifdef _DEBUG
#pragma comment(lib, "D:/dev_libs/boost_1_59_0/stage/lib/libboost_system-vc140-mt-gd-1_59.lib")
#pragma comment(lib, "D:/dev_libs/boost_1_59_0/stage/lib/libboost_date_time-vc140-mt-gd-1_59.lib")
#pragma comment(lib, "D:/dev_libs/boost_1_59_0/stage/lib/libboost_regex-vc140-mt-gd-1_59.lib")
#else
#pragma comment(lib, "D:/dev_libs/boost_1_59_0/stage/lib/libboost_system-vc140-mt-1_59.lib")
#pragma comment(lib, "D:/dev_libs/boost_1_59_0/stage/lib/libboost_date_time-vc140-mt-1_59.lib")
#pragma comment(lib, "D:/dev_libs/boost_1_59_0/stage/lib/libboost_regex-vc140-mt-1_59.lib")
#endif

#include <boost/asio.hpp>

#include "version_no.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance


using namespace jlib;


namespace detail {

auto MUTEX_NAME = _T("Global//AlarmCenter2015Mutex");

enum status {
	unchecked,
	no_updates,
	dl_updates,
	done_update,
};

status g_status = unchecked;

bool check_running() {
	HANDLE hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(hMutex);
		return true;
	} else {
		CloseHandle(hMutex);
		return false;
	}
}

std::string get_domain_ip(HWND /*hWnd*/, const std::string& domain) {
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(domain, "");
	try {
		auto iter = resolver.resolve(query);
		boost::asio::ip::tcp::endpoint endpoint = *iter;
		return endpoint.address().to_string();
	} catch (std::exception& ) {
		//MessageBoxA(hWnd, e.what(), "Error", MB_ICONERROR);
		return "";
	}
}

long long g_progress = 0;
const int MAXBLOCKSIZE = 4 * 1024;


int download(const char *url, const char *save_as)/*将Url指向的地址的文件下载到save_as指向的本地文件*/
{
	int ret = 0;
	BYTE Temp[MAXBLOCKSIZE] = { 0 };
	ULONG Number = 1;
	FILE *stream;
	HINTERNET hSession = InternetOpenA((LPCSTR)"RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession != NULL) {
		HINTERNET handle2 = InternetOpenUrlA(hSession, (LPCSTR)url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		if (handle2 != NULL) {
			DWORD sizeBuffer;
			DWORD length = sizeof(sizeBuffer);
			bool succeeds = HttpQueryInfo(handle2, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &sizeBuffer, &length, NULL) == TRUE;

			length = 1 * 1024 * 1024; // assume 1M 
			if (succeeds) {
				std::stringstream outputStream;
				outputStream << sizeBuffer;
				outputStream >> length;
			}

			if ((stream = fopen(save_as, "wb")) != NULL) {
				bool aborted = false;
				DWORD read_len = 0;
				HWND hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_PROGRESS), GetDesktopWindow(), nullptr);
				HWND hWndProgress = nullptr;
				if (hWnd) {
					Win32CenterWindow(hWnd);
					ShowWindow(hWnd, SW_SHOW);
					hWndProgress = GetDlgItem(hWnd, IDC_PROGRESS1);
					if (hWndProgress) {
						SendMessage(hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
					}
				}

				auto update_pos = [&hWnd, &hWndProgress](int pos) {
					if (hWndProgress) {
						SendMessage(hWndProgress, PBM_SETPOS, pos, 0);
					}
					if (hWnd) {
						UpdateWindow(hWnd);
					}
				};

				while (Number > 0) {
					/*if (WAIT_OBJECT_0 == WaitForSingleObject(hEventShutdown, 0)) {
						aborted = true;
						break;
					}*/
					InternetReadFile(handle2, Temp, MAXBLOCKSIZE - 1, &Number);
					fwrite(Temp, sizeof(char), Number, stream);
					read_len += Number;
					g_progress = static_cast<long long>(read_len) * 100 / static_cast<long long>(length);
#ifdef _DEBUG
					std::stringstream ss; ss << "g_progress: " << g_progress << ", read_len: " << read_len << ", length: " << length << std::endl;
					OutputDebugStringA(ss.str().c_str());
#endif
					update_pos(static_cast<int>(g_progress));
				}

				if (hWnd) {
					EndDialog(hWnd, 0);
				}
				fclose(stream);
				if (!aborted)
					ret = 1;
			}
			InternetCloseHandle(handle2);
			handle2 = NULL;
		}
		InternetCloseHandle(hSession);
		hSession = NULL;
	}


	return ret;
}

//auto web_site = "http://192.168.168.168/AlarmCenter/"; 
//auto web_site = "http://hb1212.com/AlarmCenter/";
//auto web_site = "http://115.231.175.17/AlarmCenter/";

std::string web_site() {
	static std::string ip = "";
	if (ip.empty()) {
		ip = get_domain_ip(nullptr, "hb1212.com");
	}
	return "http://" + ip + "/AlarmCenter/";
}

void remove_spaces(std::string& str) {
	str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

bool get_version_no_from_update_server(version_no& ver) {
	std::stringstream ss;
	ss << web_site() << version_ini;
	auto url = ss.str();
	auto dl_version_ini_path = get_exe_path_a() + "\\" + version_ini_dl;
	DeleteFileA(dl_version_ini_path.c_str());
	if (!download(url.c_str(), dl_version_ini_path.c_str())) { // set flag
		return false;
	}
	std::ifstream in(dl_version_ini_path);
	if (!in)return false;
	ss.str(""); ss.clear();
	ss << in.rdbuf();
	ver.reset();
	ver.from_string(ss.str());

	in.close();

	return ver.valid();
}

inline bool check_update() {
	version_no local_ver, remote_ver;
	if (!get_version_no_from_ini(local_ver, get_exe_path_a() + "\\" + version_ini)
		|| !get_version_no_from_update_server(remote_ver)) {
		return false;
	}

	if (local_ver < remote_ver) {
		std::stringstream ss;
		ss << web_site() << alarm_center_prefix << remote_ver.to_string() << alarm_center_postfix;
		auto url = ss.str();
		ss.str(""); ss.clear();
		ss << get_exe_path_a() << "\\update\\" << alarm_center_prefix << remote_ver.to_string() << alarm_center_postfix;
		auto installer = ss.str();
		if (!download(url.c_str(), installer.c_str()))
			return false;

		std::ifstream in(installer);
		if (!in) return false;
		in.close();
		return true;
	}

	return false;
}

inline std::string get_change_log_by_version(const version_no& ver) {
	std::stringstream ss;
	ss << web_site() << change_log;
	auto url_change_log = ss.str(); ss.str(""); ss.clear();
	ss << get_exe_path_a() << "\\update\\" << change_log;
	auto dl_change_log = ss.str();
	do {
		if (!download(url_change_log.c_str(), dl_change_log.c_str())) {
			break;
		}

		std::ifstream in(dl_change_log);
		if (!in) {
			break;
		}

		std::string line, version_string, timestamp, updates;
		bool ok = false;
		version_no ver_parsed;
		std::getline(in, line); // ignore author
		while (std::getline(in, line)) {
			if (ok) {
				remove_spaces(line);
				if (line.empty()) {
					break; // done
				}
				updates += line + "\r\n";
			} else {
				std::istringstream is(line);
				std::string content;
				is >> content;
				if (ver_parsed.from_string(content).valid()/* && ver_parsed == ver*/) {
					ok = true;
					version_string = content;
					timestamp = is.str();
				} else if (!line.empty()) {
					break; // failed
				}
			}
		}

		if (ok) {
			return timestamp + "\r\n" + updates;
		}
	} while (0);

	return "";
}

bool ask_user_to_install_update_or_not(const std::wstring& update_msg) {
	wchar_t buff[1024] = { 0 };
	LoadStringW(hInst, IDS_STRING_UPDATE, buff, MAX_LOADSTRING);
	std::wstring msg = buff;
	msg += L"\r\n\r\n";
	msg += update_msg;
	int ret = MessageBox(nullptr, msg.c_str(), L"AlarmCenter", MB_YESNO | MB_ICONQUESTION);
	return ret == IDYES;
}


inline bool check_if_update_installer_already_ready() {
	std::stringstream ss;
	ss << get_exe_path_a() << "\\" << version_ini_dl;
	auto dl_version_ini_path = ss.str();

	ss.str(""); ss.clear();
	ss << get_exe_path_a() << "\\" << version_ini;
	auto cur_version_ini_path = ss.str();

	version_no dl_ver, cur_ver;

	if (get_version_no_from_ini(dl_ver, dl_version_ini_path)
		&& get_version_no_from_ini(cur_ver, cur_version_ini_path)
		&& cur_ver < dl_ver) {

		ss.str(""); ss.clear();
		ss << utf8::w2a(get_exe_path()) << "\\update\\" << alarm_center_prefix << dl_ver.to_string() << alarm_center_postfix;
		auto dl_installer_path = ss.str();

		std::ifstream in(dl_installer_path);
		if (!in)return false;
		in.close();

		auto update_msg = get_change_log_by_version(dl_ver);
		if (update_msg.empty()) return false;

		if (!ask_user_to_install_update_or_not(utf8::a2w(update_msg))) {
			g_status = dl_updates;
			return true;
		}

		jlib::daemon(utf8::a2w(dl_installer_path), false);
		DeleteFileA(dl_version_ini_path.c_str()); // clear flag
		g_status = done_update;
		return true;
	}

	return false;
}



void do_daemon_things() {
	auto exe = get_exe_path() + L"\\AlarmCenter.exe";
	DWORD ret = jlib::daemon(exe);
	while (ret == 9958 || ret == 9959) {
		ret = jlib::daemon(exe);
	}
}

}

using namespace detail;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       /*nCmdShow*/)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;

	if (check_running()) {
		return 0;
	} else {

		if (check_if_update_installer_already_ready()) {
			if (g_status == done_update) {
				return 0;
			} 
		} else if (check_update() && check_if_update_installer_already_ready()) {
			if (g_status == done_update) {
				return 0;
			}
		} 
			
		do_daemon_things();
		
		
		return 0;
	}


}



