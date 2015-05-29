// TestPython.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#define HAVE_ROUND
#include "F:/Python27/include/Python.h"
//#pragma comment(lib, "F:/Python27/libs/python27.lib")

#include "../AlarmCenter/baidu.h"

void test()
{
	do {
		Py_Initialize();
		if (!Py_IsInitialized())
			break;

		typedef PyObject* pyobj;
		pyobj module = NULL, func_locate_by_ip = NULL, func_get_addr = NULL,
			func_get_x = NULL, func_get_y = NULL;

		PyRun_SimpleString("import sys");
		PyRun_SimpleString("sys.path.append('C:/PythonWorkspace/testBaiduMap/src')");
		module = PyImport_ImportModule("locate_by_ip");
		if (NULL == module)
			break;


		func_locate_by_ip = PyObject_GetAttrString(module, "locate_by_ip");
		if (NULL == func_locate_by_ip)
			break;

		pyobj ret = PyEval_CallObject(func_locate_by_ip, NULL);

		int ok = 0;
		int size = PyTuple_Size(ret);
		pyobj addr = PyTuple_GetItem(ret, 0);
		//std::string address;
		wchar_t* address = NULL;
		ok = PyArg_Parse(addr, "s", &address);
		int city_code;
		//float x, y;
		//char* err = NULL;
		//if (PyUnicode_Check(addr)) {
		//	int i = 0;
		//	PyObject * temp_bytes = PyUnicode_AsUTF8String(addr); // Owned reference
		//	if (temp_bytes != NULL) {
		//		char* my_result = PyBytes_AS_STRING(temp_bytes); // Borrowed pointer
		//		
		//		my_result = _strdup(my_result);
		//		int i = 0;
		//		//Py_DECREF(temp_bytes);
		//	} else {
		//		// TODO: Handle encoding error.
		//	}
		//} else if (PyBytes_Check(addr)) {
		//	int i = 0;
		//} else {
		//	int i = 0;
		//}
		//char* s = PyString_AsDecodedString(addr, "utf-8", err);
		//ok = PyArg_Parse(addr, "s", &address);

		pyobj code = PyTuple_GetItem(ret, 1);
		ok = PyArg_Parse(code, "i", &city_code);
		//PyArg_ParseTuple(ret, "i", &ok);

		pyobj px = PyTuple_GetItem(ret, 2);
		pyobj py = PyTuple_GetItem(ret, 3);
		double x = PyFloat_AsDouble(px);
		double y = PyFloat_AsDouble(py);
		if (size) {

			//ok = PyArg_ParseTuple(ret, "s|i|f|f", &address, &city_code, &x, &y);
		}
		wchar_t* sss = L"陕西省西安市";
		wprintf(L"%s, %d, %f, %f\n", sss, city_code, x, y);
		wprintf(L"%s, %d, %f, %f", address, city_code, x, y);
		int i = 0;
	} while (0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring addr;
	int city_code;
	double x, y;
	//if (web::CBaiduService::GetInstance()->locate(addr, city_code, x, y)) {
	//	int i = 0;
	//}
	return 0;
}

