// TestBoostAsio.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning(disable:4005)

#pragma comment(lib, "C:/dev_libs/boost_1_59_0/stage/lib/libboost_system-vc140-mt-gd-1_59.lib")
#pragma comment(lib, "C:/dev_libs/boost_1_59_0/stage/lib/libboost_date_time-vc140-mt-gd-1_59.lib")
#pragma comment(lib, "C:/dev_libs/boost_1_59_0/stage/lib/libboost_regex-vc140-mt-gd-1_59.lib")
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using namespace boost::asio;

class server {
	typedef server this_type;
	typedef ip::tcp::acceptor acceptor_type;
	typedef ip::tcp::endpoint endpoint_type;
	typedef ip::tcp::socket socket_type;
	typedef boost::shared_ptr<socket_type> sock_ptr;
	typedef std::vector<char> buffer_type;
private:
	io_service io_service_;
	acceptor_type acceptor_;
	buffer_type recv_buffer_;
public:
	server() :acceptor_(io_service_, endpoint_type(ip::tcp::v4(), 8080)) { accept(); }
	void run() { io_service_.run(); }
	void accept() {
		sock_ptr sock(new socket_type(io_service_));
		acceptor_.async_accept(*sock, boost::bind(&this_type::accept_handler, this, boost::asio::placeholders::error, sock));
	}
	void accept_handler(const system::error_code& ec, sock_ptr sock) {
		if (ec) { assert(0); return; }
		std::cout << "client: " << sock->remote_endpoint().address() << std::endl;
		//sock->async_write_some(buffer("hello world"), boost::bind(&this_type::write_handler, this, boost::asio::placeholders::error));
		sock->async_read_some(buffer(recv_buffer_), boost::bind(&this_type::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		accept();
	}
	void write_handler(const system::error_code& ec) {
		std::cout << "send msg complete: ec " << ec << std::endl;
	}
	void write_handler2(const system::error_code& ec, std::size_t n) {
		std::cout << "send msg complete: ec " << ec << ", bytes " << n << std::endl;;
	}
	void read_handler(const system::error_code& ec, size_t n) {
		if (ec) { assert(0); return; }
		if(!recv_buffer_.empty())
			std::cout << &recv_buffer_[0] << std::endl;
	}

};


int main()
{
	server srv;
	srv.run();

    return 0;
}

