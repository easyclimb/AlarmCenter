#pragma once

#include "chat_message.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace ipc {

class talk_to_client : public std::enable_shared_from_this<talk_to_client>, boost::noncopyable
{
	typedef talk_to_client self_type;
	talk_to_client(boost::asio::io_service& service) :sock_(service), timer_(service) {}

public:
	typedef boost::system::error_code bec;
	typedef std::shared_ptr<self_type> self_ptr;

	~talk_to_client() {}

private:
	boost::asio::ip::tcp::socket sock_;
	enum { max_msg = 1024 };
	char read_buffer_[max_msg] = { 0 };
	char write_buffer_[max_msg] = { 0 };
	bool started_ = false;
	
	std::string user_name_ = "";
	boost::asio::deadline_timer timer_;
	std::chrono::steady_clock::time_point last_ping_;

public:

};

class ipc_server : dp::singleton<ipc_server>
{
protected:
	ipc_server();
	std::shared_ptr<boost::asio::io_service> service_ = {};


public:
	virtual ~ipc_server();






};







}
