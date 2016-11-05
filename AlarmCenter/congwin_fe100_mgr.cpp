#include "stdafx.h"
#include "congwin_fe100_mgr.h"

namespace core {

congwin_fe100_mgr::congwin_fe100_mgr()
{
}

congwin_fe100_mgr::~congwin_fe100_mgr()
{
	Close();
}

BOOL congwin_fe100_mgr::Open(int port)
{
	AUTO_LOG_FUNCTION;
	if (!m_bOpened) {
		m_bOpened = InitPort(nullptr, port, 9600);
		if (m_bOpened) {
			last_time_send_ = std::chrono::steady_clock::now();
			StartMonitoring();

			running_ = true;
			thread_ = std::thread(&congwin_fe100_mgr::worker, this);

		}
	}
	return m_bOpened;
}


void congwin_fe100_mgr::Close()
{
	AUTO_LOG_FUNCTION;
	if (m_bOpened) {
		m_bOpened = FALSE;
		ClosePort();
		ReleasePort();

		running_ = false;
		thread_.join();
	}

	//m_taskList.clear();
	data_buffer_.clear();
}

void congwin_fe100_mgr::add_event(const ademco::AdemcoDataSegment * data)
{
	if (!data || !data->_valid || data->_ademco_event == 0) {
		return;
	}

	std::lock_guard<std::mutex> lg(m_lock);
	auto now = std::chrono::steady_clock::now();

	auto iter = data_buffer_filter_.find(data->_ademco_id);
	if (iter == data_buffer_filter_.end()) {
		auto dt = std::make_shared<data_times>();
		auto pd = std::make_shared<ademco::AdemcoDataSegment>(*data);
		dt->push_back(data_time_pair(now, pd));
		data_buffer_filter_.insert(std::pair<int, data_times_ptr>(data->_ademco_id, dt));
		data_buffer_.push_back(pd);
	} else {
		
		auto dt = iter->second;
		auto dt_iter = dt->begin();
		while (dt_iter != dt->end()) {
			auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - dt_iter->first);
			if (diff.count() >= 6) {
				dt_iter = dt->erase(dt_iter);
				continue;
			} else {
				if (dt_iter->second->operator==(*data)) {
					dt_iter->first = now;
					return;
				}
			}
			dt_iter++;
		}

		auto pd = std::make_shared<ademco::AdemcoDataSegment>(*data);
		dt->push_back(data_time_pair(now, pd));
		data_buffer_.push_back(pd);
	}


}

BOOL congwin_fe100_mgr::OnRecv(const char * cmd, WORD wLen)
{
	m_recvBuff.Write(cmd, wLen);
	return TRUE;
}

BOOL congwin_fe100_mgr::OnSend(IN char * cmd, IN WORD wLen, OUT WORD & wRealLen)
{
	if (!data_buffer_.empty()) {
		std::lock_guard<std::mutex> lg(m_lock);
		
		/*auto& iter = data_buffer_.begin();
		auto& dp = iter->second;
		if (dp->empty()) {
			return 0;
		}*/

		auto pd = data_buffer_.front();
		data_buffer_.pop_front();
		/*if (dp.empty()) {
			data_buffer_.erase(iter);
		}*/

		ademco::congwin_fe100_packet fe;
		fe.from_ademco_data_segment(pd.get());
		std::copy(fe.data_, fe.data_ + fe.len_, (cmd));
		wLen = fe.len_;
		wRealLen = fe.len_;
		last_time_send_ = std::chrono::steady_clock::now();
		return TRUE;
	} else {
		auto now = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - last_time_send_);
		if (diff.count() > 1) {
			//memcpy(cmd, "hello\n", 6);
			//wLen = wRealLen = 6;

			ademco::AdemcoDataSegment data;
			data.Make();
			ademco::congwin_fe100_packet fe;
			fe.from_ademco_data_segment(&data);
			std::copy(fe.data_, fe.data_ + fe.len_, (cmd));
			wLen = fe.len_;
			wRealLen = fe.len_;

			last_time_send_ = std::chrono::steady_clock::now();
			return TRUE;
		}
	}
	return 0;
}

void congwin_fe100_mgr::worker()
{
	last_time_send_ = std::chrono::steady_clock::now();

	while (true) {
		if (!running_) {
			break;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}





}

