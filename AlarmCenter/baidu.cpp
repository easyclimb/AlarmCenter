#include "stdafx.h"
#include "baidu.h"

/*******************************/
// http://p-nand-q.com/python/building-python-33-with-vs2013.html
// blablabla\python-3.3.2\include\pymath.h(22): warning C4273: 'round' : inconsistent dll linkage
// This is so because HAVE_ROUND is not defined on Windows; your most simple option is to modify pymath.h to define HAVE_ROUND right before the #ifdef.
// Jack: it seems like useful with python2.7
#define HAVE_ROUND
/*******************************/

#include "F:/Python27/include/Python.h"

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

	IMPLEMENT_SINGLETON(CBaiduService);

	CBaiduService::CBaiduService()
	{

	}

	CBaiduService::~CBaiduService()
	{

	}

	bool CBaiduService::locate(std::wstring& addr, int& city_code, double& x, double& y)
	{
		AUTO_LOG_FUNCTION;
		try {
			do {
				Py_Initialize();
				if (!Py_IsInitialized()) {
					LOG(L"Py_IsInitialized failed.\n");
					break;
				}
				LOG(L"Py_IsInitialized success.\n");
				typedef PyObject* pyobj;
				pyobj module = NULL, func_locate_by_ip = NULL;

				std::wstring path = GetModuleFilePath();
				path += L"\\python";
				//std::string apath;
				//utf8::utf16to8(path.begin(), path.end(), std::back_inserter(apath));
				PyRun_SimpleString("import sys");
				LOG(L"import sys success.\n");
				USES_CONVERSION;
				const char* apath = W2A(path.c_str());
				std::string string = "sys.path.append('";
				string += apath;
				string += "')";
				PyRun_SimpleString(string.c_str());
				module = PyImport_ImportModule("locate_by_ip");
				if (NULL == module) {
					LOG(L"PyImport_ImportModule failed.\n");
					break;
				}

				func_locate_by_ip = PyObject_GetAttrString(module, "func_locate_by_ip");
				if (NULL == func_locate_by_ip) {
					LOG(L"PyObject_GetAttrString failed.\n");
					break;
				}

				pyobj ret = PyEval_CallObject(func_locate_by_ip, NULL);
				if (NULL == ret) {
					LOG(L"PyEval_CallObject failed.\n");
					break;
				}

				int ok = 0;
				int size = PyTuple_Size(ret);
				if (size != 5) {
					LOG(L"PyTuple_Size(ret) != 5.\n");
					break;
				}

				pyobj pystatus = PyTuple_GetItem(ret, 0);
				int status = _PyInt_AsInt(pystatus);
				if (status != 0) {
					LOG(L"_PyInt_AsInt status != 0, %d.\n", status);
					pyobj err = PyTuple_GetItem(ret, 1);
					wchar_t* error = NULL;
					ok = PyArg_Parse(err, "u", &error);
					if (0 == ok) {
						LOG(L"PyArg_Parse addr failed.\n");
						break;
					}
					LOG(L"%s\n", error);
					break;
				}

				pyobj pyaddr = PyTuple_GetItem(ret, 1);
				wchar_t* address = NULL;
				ok = PyArg_Parse(pyaddr, "u", &address);
				if (0 == ok) {
					LOG(L"PyArg_Parse addr failed.\n");
					break;
				}
				addr = address;
				LOG(L"%s\n", addr.c_str());

				pyobj code = PyTuple_GetItem(ret, 2);
				ok = PyArg_Parse(code, "i", &city_code);
				if (0 == ok) {
					LOG(L"PyArg_Parse city_code failed.\n");
					break;
				}

				pyobj px = PyTuple_GetItem(ret, 3);
				pyobj py = PyTuple_GetItem(ret, 4);
				x = PyFloat_AsDouble(px);
				y = PyFloat_AsDouble(py);

				Py_Finalize();
				LOG(L"locate success! addr:%s, city_code:%d, x:%f, y:%f\n",
					address, city_code, x, y);
				return true;
			} while (0);

			if (Py_IsInitialized())
				Py_Finalize();
		} catch (...) {
			AfxMessageBox(L"locate fail!");
		}
		return false;
	}
};






