#include "stdafx.h"
#include "baidu.h"

/*******************************/
// http://p-nand-q.com/python/building-python-33-with-vs2013.html
// blablabla\python-3.3.2\include\pymath.h(22): warning C4273: 'round' : inconsistent dll linkage
// This is so because HAVE_ROUND is not defined on Windows; your most simple option is to modify pymath.h to define HAVE_ROUND right before the #ifdef.
// Jack: it seems like useful with python2.7
//#define HAVE_ROUND // python2.7.10 defined this micro
/*******************************/

#include "C:/Python27/include/Python.h"

//#ifdef _DEBUG 
////#pragma comment(lib, "F:/Python27/libs/python27_d.lib")
//#else 
//#pragma comment(lib, "F:/Python27/libs/python27.lib")
//#endif 

namespace web
{
	/*namespace baidu
	{
		bool get_my_ip(wchar_t* dst, size_t dst_size)
		{
			do {
				WSAData	wsData;
				int nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
				if (nRet < 0) {
					CLog::WriteLog(L"Can't load winsock.dll.\n");
					break;;
				}




				return true;
			} while (0);
			return false;
		}
	};*/
	//
	//IMPLEMENT_SINGLETON(CBaiduService);

	//CBaiduService::CBaiduService()
	//{

	//}

	//CBaiduService::~CBaiduService()
	//{

	//}

	//bool CBaiduService::locate(std::wstring& addr, int& city_code, BaiduCoordinate& coor)
	//{
	//	AUTO_LOG_FUNCTION;
	//	try {
	//		do {
	//			Py_Initialize();
	//			if (!Py_IsInitialized()) {
	//				JLOG(L"Py_IsInitialized failed.\n");
	//				break;
	//			}
	//			JLOG(L"Py_IsInitialized success.\n");
	//			typedef PyObject* pyobj;
	//			pyobj module = nullptr, func_locate_by_ip = nullptr;

	//			std::wstring path = GetModuleFilePath();
	//			path += L"\\python";
	//			//std::string apath;
	//			//utf8::utf16to8(path.begin(), path.end(), std::back_inserter(apath));
	//			PyRun_SimpleString("import sys");
	//			JLOG(L"import sys success.\n");
	//			USES_CONVERSION;
	//			const char* apath = W2A(path.c_str());
	//			std::string string = "sys.path.append('";
	//			string += apath;
	//			string += "')";
	//			//JLOGA(string.c_str());
	//			for (size_t i = 0; i < string.size(); i++) {
	//				if (string[i] == '\\') {
	//					string[i] = '/';
	//				}
	//			}
	//			JLOG(L"%s\n", A2W(string.c_str()));
	//			PyRun_SimpleString(string.c_str());
	//			module = PyImport_ImportModule("locate_by_ip");
	//			if (nullptr == module) {
	//				JLOG(L"PyImport_ImportModule failed.\n");
	//				break;
	//			}

	//			func_locate_by_ip = PyObject_GetAttrString(module, "func_locate_by_ip");
	//			if (nullptr == func_locate_by_ip) {
	//				JLOG(L"PyObject_GetAttrString failed.\n");
	//				break;
	//			}

	//			pyobj ret = PyEval_CallObject(func_locate_by_ip, nullptr);
	//			if (nullptr == ret) {
	//				JLOG(L"PyEval_CallObject failed.\n");
	//				break;
	//			}

	//			int ok = 0;
	//			int size = PyTuple_Size(ret);
	//			if (size != 5) {
	//				JLOG(L"PyTuple_Size(ret) != 5.\n");
	//				break;
	//			}

	//			pyobj pystatus = PyTuple_GetItem(ret, 0);
	//			int status = _PyInt_AsInt(pystatus);
	//			if (status != 0) {
	//				JLOG(L"_PyInt_AsInt status != 0, %d.\n", status);
	//				pyobj err = PyTuple_GetItem(ret, 1);
	//				wchar_t* error = nullptr;
	//				ok = PyArg_Parse(err, "u", &error);
	//				if (0 == ok) {
	//					JLOG(L"PyArg_Parse addr failed.\n");
	//					break;
	//				}
	//				JLOG(L"%s\n", error);
	//				break;
	//			}

	//			pyobj pyaddr = PyTuple_GetItem(ret, 1);
	//			wchar_t* address = nullptr;
	//			ok = PyArg_Parse(pyaddr, "u", &address);
	//			if (0 == ok) {
	//				JLOG(L"PyArg_Parse addr failed.\n");
	//				break;
	//			}
	//			addr = address;
	//			JLOG(L"%s\n", addr.c_str());

	//			pyobj code = PyTuple_GetItem(ret, 2);
	//			ok = PyArg_Parse(code, "i", &city_code);
	//			if (0 == ok) {
	//				JLOG(L"PyArg_Parse city_code failed.\n");
	//				break;
	//			}

	//			pyobj px = PyTuple_GetItem(ret, 3);
	//			pyobj py = PyTuple_GetItem(ret, 4);
	//			coor.x = PyFloat_AsDouble(px);
	//			coor.y = PyFloat_AsDouble(py);

	//			Py_Finalize();
	//			JLOG(L"locate success! addr:%s, city_code:%d, x:%f, y:%f\n",
	//				address, city_code, coor.x, coor.y);
	//			return true;
	//		} while (0);

	//		if (Py_IsInitialized())
	//			Py_Finalize();
	//	} catch (...) {
	//	}
	//	return false;
	//}
};







