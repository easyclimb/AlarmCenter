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

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


namespace detail {

auto MUTEX_NAME = _T("Global//AlarmCenter2015Mutex");

enum status {
	unchecked,
	no_updates,
	dl_updates,

};

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

inline std::wstring get_exe_path()
{
	wchar_t path[1024] = { 0 };
	GetModuleFileName(nullptr, path, 1024);
	std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
	return std::wstring(path).substr(0, pos);
}

inline std::string get_exe_path_a()
{
	char path[1024] = { 0 };
	GetModuleFileNameA(nullptr, path, 1024);
	std::string::size_type pos = std::string(path).find_last_of("\\/");
	return std::string(path).substr(0, pos);
}

DWORD daemon(const std::wstring& path, bool wait_app_exit = true) {
	STARTUPINFO si = { sizeof(si) };
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;
	::SetFocus(GetDesktopWindow());
	BOOL bRet = CreateProcess(NULL, (LPWSTR)(path.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bRet) {
		WaitForSingleObject(pi.hProcess, wait_app_exit ? INFINITE : 0);
		DWORD dwExit;
		::GetExitCodeProcess(pi.hProcess, &dwExit);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return dwExit;
	}
	return 0;
}




struct version_no {
	int major = 0;
	int minor = 0;
	int revision = 0;
	int build = 0;

	version_no& from_string(const std::string& s) {
		std::sscanf(s.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &build);
		if (major < 0) major = 0;
		if (minor < 0) minor = 0;
		if (revision < 0) revision = 0;
		if (build < 0) build = 0;
		return *this;
	}

	std::string to_string() const {
		std::stringstream ss;
		ss << major << "." << minor << "." << revision << "." << build;
		return ss.str();
	}

	bool valid() {
		return !(major == 0 && minor == 0 && revision == 0 && build == 0);
	}

	void reset() {
		major = minor = revision = build = 0;
	}

	bool operator == (const version_no& ver) {
		return major == ver.major
			&& minor == ver.minor
			&& revision == ver.revision
			&& build == ver.build;
	}

	bool operator < (const version_no& ver) {
		return major < ver.major
			|| minor < ver.minor
			|| revision < ver.revision
			|| build < ver.build;
	}


};

bool get_version_no_from_ini(version_no& ver, const std::string& ini_path) {
	std::ifstream in(ini_path);
	if (!in)return false;
	std::stringstream is;
	is << in.rdbuf();
	version_no file_ver;
	file_ver.from_string(is.str());
	if (file_ver.valid()) {
		ver = file_ver;
		return true;
	}
	return false;
}

//HANDLE hEventShutdown = INVALID_HANDLE_VALUE;
//HANDLE hThread = INVALID_HANDLE_VALUE;
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

auto web_site = "http://192.168.168.168/AlarmCenter/";
auto version_ini = "VersionNo.ini";
auto version_ini_dl = "dl_VersionNo.ini";
auto alarm_center_prefix = "AlarmCenter_Setup_V";
auto alarm_center_postfix = ".exe";
auto change_log = "changelog.txt";

bool get_version_no_from_update_server(version_no& ver) {
	std::stringstream ss;
	ss << web_site << version_ini;
	auto url = ss.str();
	auto dl_version_ini_path = get_exe_path_a() + version_ini_dl;
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

bool check_update() {
	version_no local_ver, remote_ver;
	if (!get_version_no_from_ini(local_ver, get_exe_path_a() + "\\" + version_ini)
		|| !get_version_no_from_update_server(remote_ver)) {
		return false;
	}

	if (local_ver < remote_ver) {
		std::stringstream ss;
		ss << web_site << alarm_center_prefix << remote_ver.to_string() << alarm_center_postfix;
		auto url = ss.str();
		ss.clear();
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

void remove_spaces(std::string& str) {
	str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

std::string get_change_log_by_version(const version_no& ver) {
	std::stringstream ss;
	ss << web_site << change_log;
	auto url_change_log = ss.str(); ss.clear();
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
				if (ver_parsed.from_string(content).valid() && ver_parsed == ver) {
					ok = true;
					version_string = content;
					timestamp = is.str();
				} else {
					break; // failed
				}
			}
		}

		if (ok) {
			return version_string + timestamp + "\r\n" + updates;
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

bool check_if_update_installer_already_ready() {
	std::stringstream ss;
	ss << get_exe_path_a() << "\\" << version_ini_dl;
	auto dl_version_ini_path = ss.str();

	ss.clear();
	ss << get_exe_path_a() << "\\" << version_ini;
	auto cur_version_ini_path = ss.str();

	version_no dl_ver, cur_ver;

	if (get_version_no_from_ini(dl_ver, dl_version_ini_path)
		&& get_version_no_from_ini(cur_ver, cur_version_ini_path)
		&& cur_ver < dl_ver) {

		ss.clear();
		ss << utf8::w2a(get_exe_path()) << "\\update\\" << alarm_center_prefix << dl_ver.to_string() << alarm_center_postfix;
		auto dl_installer_path = ss.str();

		std::ifstream in(dl_installer_path);
		if (!in)return false;
		in.close();

		auto update_msg = get_change_log_by_version(dl_ver);
		if (update_msg.empty()) return false;

		if (!ask_user_to_install_update_or_not(utf8::a2w(update_msg))) {
			return false;
		}

		daemon(utf8::a2w(dl_installer_path), false);
		DeleteFileA(dl_version_ini_path.c_str()); // clear flag
		return true;
	}

	return false;
}

void do_update_things() {

}


void do_daemon_things() {
	auto exe = get_exe_path() + L"\\AlarmCenter.exe";
	DWORD ret = daemon(exe);
	while (ret == 9958 || ret == 9959) {
		ret = daemon(exe);
	}
}

}

using namespace detail;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;

	if (check_running()) {
		return 0;
	} else {




		
		return 0;
	}


	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_ALARMCENTERDAEMON, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ALARMCENTERDAEMON));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALARMCENTERDAEMON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ALARMCENTERDAEMON);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		SetTimer(hDlg, 1, 1000, nullptr);
		return (INT_PTR)TRUE;

	case WM_TIMER:
		if (!check_running()) {

		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
