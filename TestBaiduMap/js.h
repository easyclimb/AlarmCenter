#pragma once

typedef void __stdcall JsFunction_Callback(LPVOID pParam);

class JsFunction :public IDispatch
{
	long _refNum;
	JsFunction_Callback *m_pCallback;
	LPVOID m_pParam;
public:
	JsFunction(JsFunction_Callback *pCallback, LPVOID pParam)
	{
		_refNum = 1;
		m_pCallback = pCallback;
		m_pParam = pParam;
	}
	~JsFunction(void)
	{}
public:

	// IUnknown Methods

	STDMETHODIMP QueryInterface(REFIID iid, void**ppvObject)
	{
		*ppvObject = NULL;

		if (iid == IID_IUnknown)	*ppvObject = this;
		else if (iid == IID_IDispatch)	*ppvObject = (IDispatch*)this;

		if (*ppvObject) {
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return ::InterlockedIncrement(&_refNum);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		::InterlockedDecrement(&_refNum);
		if (_refNum == 0) {
			delete this;
		}
		return _refNum;
	}

	// IDispatch Methods

	HRESULT __stdcall GetTypeInfoCount(
		unsigned int * pctinfo)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall GetTypeInfo(
		unsigned int iTInfo,
		LCID lcid,
		ITypeInfo FAR* FAR* ppTInfo)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall GetIDsOfNames(
		REFIID riid,
		OLECHAR FAR* FAR* rgszNames,
		unsigned int cNames,
		LCID lcid,
		DISPID FAR* rgDispId
		)
	{
		//令人费解的是，网页调用函数的call方法时，没有调用GetIDsOfNames获取call的ID，而是直接调用Invoke
		return E_NOTIMPL;
	}

	HRESULT __stdcall Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS* pDispParams,
		VARIANT* pVarResult,
		EXCEPINFO* pExcepInfo,
		unsigned int* puArgErr
		)
	{
		m_pCallback(m_pParam);
		return S_OK;
	}
};






