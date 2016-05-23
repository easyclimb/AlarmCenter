#pragma once
#include <list>
#include <map>
#include <memory>

namespace video {

	typedef enum msg_type
	{
		TYPE_GET = 1,
		TYPE_HD = 2,
	}msg_type;

	typedef enum productor
	{
		UNKNOWN,
		EZVIZ,
		JOVISION,
	}productor;

	class productor_info
	{
	private:
		productor _productor;
		std::wstring _name;
		std::wstring _description;
		std::string _appKey;

	public:
		DECLARE_GETTER(productor, _productor);
		DECLARE_GETTER_SETTER(std::wstring, _name);
		DECLARE_GETTER_SETTER(std::wstring, _description);

		productor_info(productor productor, 
					   const std::wstring& name,
					   const std::wstring& desc)
					   : _productor(productor)
					   , _name(name)
					   , _description(desc)
		{}

		productor_info() 
			: _productor(UNKNOWN)
			, _name()
			, _description()
		{}

		productor_info(const productor_info& rhs)
			: _productor(rhs._productor)
			, _name(rhs._name)
			, _description(rhs._description)
		{}

		const productor_info& operator=(const productor_info& rhs)
		{
			_productor = rhs._productor;
			_name = rhs._name;
			_description = rhs._description;

			return *this;
		}
	};

	static const productor_info ProductorUnknown;

#define SET_DATA_MEMBER_STRING(obj, member) obj->set_##member(W2A(member));
#define SET_DATA_MEMBER_WCSTRING(obj, member) obj->set_##member(member.LockBuffer()); member.UnlockBuffer();
#define SET_DATA_MEMBER_INTEGER(obj, member) obj->set_##member(member);

#define SET_DEVICE_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(member) SET_DATA_MEMBER_WCSTRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(deviceInfo, member);

#define SET_USER_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_WSTRING(member) SET_DATA_MEMBER_WCSTRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(userInfo, member);

	class video_user_info;
	typedef std::shared_ptr<video_user_info> video_user_info_ptr;
	typedef std::weak_ptr<video_user_info> video_user_info_weak_ptr;
	typedef std::list<video_user_info_ptr> video_user_info_list;

	class video_device_info;
	typedef std::shared_ptr<video_device_info> video_device_info_ptr;
	typedef std::list<video_device_info_ptr> video_device_info_list;
	
	namespace ezviz {
		class video_user_info_ezviz;
		typedef std::shared_ptr<video_user_info_ezviz> video_user_info_ezviz_ptr;
		typedef std::list<video_user_info_ezviz_ptr> video_user_info_ezviz_list;

		class video_device_info_ezviz;
		typedef std::shared_ptr<video_device_info_ezviz> video_device_info_ezviz_ptr;
		typedef std::list<video_device_info_ezviz_ptr> video_device_info_ezviz_list;
	};

	namespace jovision {
		class video_user_info_jovision;
		typedef std::shared_ptr<video_user_info_jovision> video_user_info_jovision_ptr;
		typedef std::list<video_user_info_jovision_ptr> video_user_info_jovision_list;

		class video_device_info_jovision;
		typedef std::shared_ptr<video_device_info_jovision> video_device_info_jovision_ptr;
		typedef std::list<video_device_info_jovision_ptr> video_device_info_jovision_list;
	};

	typedef struct zone_uuid
	{
		int _ademco_id;
		int _zone_value;
		int _gg;
	
	public:
		zone_uuid(int ademco_id, int zone_value, int gg)
			:_ademco_id(ademco_id), _zone_value(zone_value), _gg(gg)
		{}

		zone_uuid() = default;
		zone_uuid& operator=(const zone_uuid& rhs) = default;
		// use default

		bool operator == (const zone_uuid& rhs)
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
	}zone_uuid;

	struct cmp_zone_uuid
	{
		bool operator() (const zone_uuid& l, const zone_uuid& r) const
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

	typedef struct bind_info
	{
		int _id;
		video_device_info_ptr _device;
		int _auto_play_video;

		bind_info() = default;

		bind_info(int id, video_device_info_ptr device, int auto_play_video)
			:_id(id), _device(device), _auto_play_video(auto_play_video)
		{}

		bind_info& operator=(const bind_info& rhs) = default;

#ifdef _DEBUG
		std::string toString() const
		{
			char buff[1024] = { 0 };
			sprintf_s(buff, "_device:%p, _auto_play_video:%d", _device.get(), _auto_play_video);
			return std::string(buff);
		}
#endif
	}bind_info;

	typedef std::map<zone_uuid, bind_info, cmp_zone_uuid> bind_map;


};


