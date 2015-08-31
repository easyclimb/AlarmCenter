#pragma once

namespace core {
namespace video {

	typedef enum PRODUCTOR
	{
		UNKNOWN,
		EZVIZ,
		NORMAL,
	}PRODUCTOR;

	class CProductorInfo
	{
	private:
		const PRODUCTOR _productor;
		const std::wstring _name;
		const std::wstring _description;
		const std::string _appKey;

	public:
		DECLARE_GETTER(PRODUCTOR, _productor);
		DECLARE_GETTER(std::wstring, _name);
		DECLARE_GETTER(std::wstring, _description);
		DECLARE_GETTER(std::string, _appKey);

		CProductorInfo(PRODUCTOR productor, 
					   const std::wstring& name,
					   const std::wstring& desc,
					   const std::string& appKey)
					   : _productor(productor)
					   , _name(name)
					   , _description(desc)
					   , _appKey(appKey)
		{}

		CProductorInfo() 
			: _productor(UNKNOWN)
			, _name()
			, _description()
			, _appKey()
		{}

		CProductorInfo(const CProductorInfo& rhs)
			: _productor(rhs._productor)
			, _name(rhs._name)
			, _description(rhs._description)
			, _appKey(rhs._appKey)
		{}

		const CProductorInfo& operator=(const CProductorInfo& rhs)
		{
			new(this) CProductorInfo(rhs);
			return *this;
		}
	};

	static const CProductorInfo ProductorUnknown;
	static const CProductorInfo ProductorEzviz(EZVIZ, L"”© Ø", L"ºÊ»›[∫£øµÕ˛ ”]", "52c8edc727cd4d4a81bb1d6c7e884fb5");
	
	static const CProductorInfo GetProductorInfo(int productor)
	{
		switch (productor) {
			case EZVIZ:
				return ProductorEzviz;
				break;
			default:
				return ProductorUnknown;
				break;

		}
	}













};
};






