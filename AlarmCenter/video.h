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
	static const CProductorInfo ProductorEzviz(EZVIZ, L"өʯ", L"����[��������]", "52c8edc727cd4d4a81bb1d6c7e884fb5");
	
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

#define SET_DATA_MEMBER_STRING(obj, member) obj->set_##member(W2A(member));
#define SET_DATA_MEMBER_WCSTRING(obj, member) obj->set_##member(member.LockBuffer()); member.UnlockBuffer();
#define SET_DATA_MEMBER_INTEGER(obj, member) obj->set_##member(member);

#define SET_DEVICE_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(member) SET_DATA_MEMBER_WCSTRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(deviceInfo, member);

#define SET_USER_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_WSTRING(member) SET_DATA_MEMBER_WCSTRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(userInfo, member);












};
};






