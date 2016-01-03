#pragma once
#include <list>
#include <map>
#include <memory>

namespace video {

	typedef enum _MsgType
	{
		TYPE_GET = 1,
		TYPE_HD = 2,
	}MsgType;

	typedef enum PRODUCTOR
	{
		UNKNOWN,
		EZVIZ,
		NORMAL,
	}PRODUCTOR;

	class CProductorInfo
	{
	private:
		PRODUCTOR _productor;
		std::wstring _name;
		std::wstring _description;
		std::string _appKey;

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
			_productor = rhs._productor;
			_name = rhs._name;
			_description = rhs._description;
			_appKey = rhs._appKey;

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
	typedef std::shared_ptr<CVideoUserInfo> CVideoUserInfoPtr;
	typedef std::weak_ptr<CVideoUserInfo> CVideoUserInfoWeakPtr;
	typedef std::list<CVideoUserInfoPtr> CVideoUserInfoList;

	class CVideoDeviceInfo;
	typedef std::shared_ptr<CVideoDeviceInfo> CVideoDeviceInfoPtr;
	//typedef std::weak_ptr<CVideoDeviceInfo> CVideoDeviceInfoWeakPtr;
	typedef std::list<CVideoDeviceInfoPtr> CVideoDeviceInfoList;
	//typedef std::list<CVideoDeviceInfoWeakPtr> CVideoDeviceInfoWeakList;
	
	namespace ezviz {
		class CVideoUserInfoEzviz;
		typedef std::shared_ptr<CVideoUserInfoEzviz> CVideoUserInfoEzvizPtr;
		typedef std::list<CVideoUserInfoEzvizPtr> CVideoUserInfoEzvizList;

		class CVideoDeviceInfoEzviz;
		typedef std::shared_ptr<CVideoDeviceInfoEzviz> CVideoDeviceInfoEzvizPtr;
		typedef std::list<CVideoDeviceInfoEzvizPtr> CVideoDeviceInfoEzvizList;
	};

	namespace normal {
		class CVideoUserInfoNormal;
		typedef std::shared_ptr<CVideoUserInfoNormal> CVideoUserInfoNormalPtr;
		typedef std::list<CVideoUserInfoNormalPtr> CVideoUserInfoNormalList;

		class CVideoDeviceInfoNormal;
		typedef std::shared_ptr<CVideoDeviceInfoNormal> CVideoDeviceInfoNormalPtr;
		typedef std::list<CVideoDeviceInfoNormalPtr> CVideoDeviceInfoNormalList;
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
		bool operator() (const ZoneUuid& l, const ZoneUuid& r) const
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
		CVideoDeviceInfoPtr _device;
		int _auto_play_video;

		BindInfo() = default;

		BindInfo(int id, CVideoDeviceInfoPtr device, int auto_play_video)
			:_id(id), _device(device), _auto_play_video(auto_play_video)
		{}

		BindInfo& operator=(const BindInfo& rhs) = default;

#ifdef _DEBUG
		std::string toString() const
		{
			char buff[1024] = { 0 };
			sprintf_s(buff, "_device:%p, _auto_play_video:%d", _device.get(), _auto_play_video);
			return std::string(buff);
		}
#endif
	}BindInfo;

	typedef std::map<ZoneUuid, BindInfo, CmpZoneUuid> CBindMap;


};


