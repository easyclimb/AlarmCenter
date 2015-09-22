#pragma once
#include <list>
#include <map>

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
		std::wstring _name;
		std::wstring _description;
		const std::string _appKey;

	public:
		DECLARE_GETTER(PRODUCTOR, _productor);
		DECLARE_GETTER_SETTER(std::wstring, _name);
		DECLARE_GETTER_SETTER(std::wstring, _description);
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

#define SET_DATA_MEMBER_STRING(obj, member) obj->set_##member(W2A(member));
#define SET_DATA_MEMBER_WCSTRING(obj, member) obj->set_##member(member.LockBuffer()); member.UnlockBuffer();
#define SET_DATA_MEMBER_INTEGER(obj, member) obj->set_##member(member);

#define SET_DEVICE_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(member) SET_DATA_MEMBER_WCSTRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(deviceInfo, member);

#define SET_USER_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_WSTRING(member) SET_DATA_MEMBER_WCSTRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(userInfo, member);

	class CVideoUserInfo;
	typedef std::list<CVideoUserInfo*> CVideoUserInfoList;
	//typedef std::list<CVideoUserInfo*>::iterator CVideoUserInfoListIter;

	class CVideoDeviceInfo;
	typedef std::list<CVideoDeviceInfo*> CVideoDeviceInfoList;
	//typedef std::list<CVideoDeviceInfo*>::iterator CVideoDeviceInfoListIter;
	
	namespace ezviz {
		class CVideoUserInfoEzviz;
		typedef std::list<CVideoUserInfoEzviz*> CVideoUserInfoEzvizList;
		//typedef std::list<CVideoUserInfoEzviz*>::iterator CVideoUserInfoEzvizListIter;

		class CVideoDeviceInfoEzviz;
		typedef std::list<CVideoDeviceInfoEzviz*> CVideoDeviceInfoEzvizList;
		//typedef std::list<CVideoDeviceInfoEzviz*>::iterator CVideoDeviceInfoEzvizListIter;
	};

	namespace normal {
		class CVideoUserInfoNormal;
		typedef std::list<CVideoUserInfoNormal*> CVideoUserInfoNormalList;
		//typedef std::list<CVideoUserInfoNormal*>::iterator CVideoUserInfoNormalListIter;

		class CVideoDeviceInfoNormal;
		typedef std::list<CVideoDeviceInfoNormal*> CVideoDeviceInfoNormalList;
		//typedef std::list<CVideoDeviceInfoNormal*>::iterator CVideoDeviceInfoNormalListIter;
	};

	typedef struct ZoneUuid
	{
		int _ademco_id;
		int _zone_value;
		int _gg;
	
	public:
		ZoneUuid(int ademco_id, int zone_value, int gg)
			:_ademco_id(ademco_id), _zone_value(zone_value), _gg(gg)
		{}

		/*bool operator < (const ZoneUuid& rhs) const
		{
			return (_ademco_id < rhs._ademco_id) 
				&& (_zone_value < rhs._ademco_id) 
				&& (_gg < rhs._gg);
		}*/

		ZoneUuid() = default;
		ZoneUuid& operator=(const ZoneUuid& rhs) = default;
		// use default

		bool operator == (const ZoneUuid& rhs)
		{
			return (_ademco_id == rhs._ademco_id)
				&& (_zone_value == rhs._zone_value)
				&& (_gg == rhs._gg);
		}
#ifdef _DEBUG
		std::string toString() const
		{
			char buff[1024] = { 0 };
			sprintf_s(buff, "ademco_id:%d, zone_value:%d, gg:%d", _ademco_id, _zone_value, _gg);
			return std::string(buff);
		}
#endif
	}ZoneUuid;

	struct CmpZoneUuid
	{
		bool operator() (const ZoneUuid& l, const ZoneUuid& r)
		{
			if (l._ademco_id < r._ademco_id) return true;
			if (l._ademco_id > r._ademco_id) return false;

			if (l._zone_value < r._zone_value) return true;
			if (l._zone_value > r._zone_value) return false;

			if (l._gg < r._gg) return true;
			if (l._gg > r._gg) return false;

			return false;
		}
	};

	typedef struct BindInfo
	{
		int _id;
		CVideoDeviceInfo* _device;
		int _auto_play_video;

		BindInfo() = default;

		BindInfo(int id, CVideoDeviceInfo* device, int auto_play_video)
			:_id(id), _device(device), _auto_play_video(auto_play_video)
		{}

		BindInfo& operator=(const BindInfo& rhs) = default;

		/*bool operator==(const DeviceInfo& rhs)
		{
			return (_device == rhs._device)
				&& (_auto_play_video == rhs._auto_play_video);
		}*/

		/*bool operator < (const DeviceInfo& rhs)
		{
			return reinterpret_cast<int>(_device) < reinterpret_cast<int>(rhs._device);
		}*/
#ifdef _DEBUG
		std::string toString() const
		{
			char buff[1024] = { 0 };
			sprintf_s(buff, "_device:%d, _auto_play_video:%d", _device, _auto_play_video);
			return std::string(buff);
		}
#endif
	}BindInfo;

	//typedef std::list<BindInfo*> CBindInfoList;

	typedef std::map<ZoneUuid, BindInfo, CmpZoneUuid> CBindMap;









};






