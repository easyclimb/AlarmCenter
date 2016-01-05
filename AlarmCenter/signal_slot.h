#pragma once

#include <assert.h>
#include <mutex>
#include <functional>
#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>

namespace util {
	namespace detail {
		template <typename callback>
		struct slotimpl;

		template <typename callback>
		struct signalimpl : boost::noncopyable
		{
			typedef std::vector<std::weak_ptr<slotimpl<callback>>> slotlist;

			signalimpl() : _slots(std::make_shared<slotlist>()) {}

			void copy_on_write() {
				if (!_slots.unique()) {
					//_slots.reset(std::make_shared<slotlist>(_slots));
					_slots.reset(new slotlist(*_slots));
				}
				assert(_slots.unique());
			}

			void clean() {
				std::lock_guard<std::mutex> lock(_mutex);
				copy_on_write();
				slotlist& list(*_slots);
				auto iter = list.begin();
				while (iter != list.end()) {
					if (iter->expired()) {
						iter = list.erase(iter);
					} else {
						++iter;
					}
				}
			}

			std::mutex _mutex;
			std::shared_ptr<slotlist> _slots;
		};

		template <typename callback>
		struct slotimpl : boost::noncopyable
		{
			typedef signalimpl<callback> data_type;

			slotimpl(const std::shared_ptr<data_type>& data, callback&& cb)
				: _data(data), _cb(cb), _tie(), _tied(false)
			{}

			slotimpl(const std::shared_ptr<data_type>& data, callback&& cb, std::shared_ptr<void>& tie)
				: _data(data), _cb(cb), _tie(tie), _tied(true)
			{}

			~slotimpl() {
				std::shared_ptr<data_type> data(_data.lock());
				if (data) {
					data->clean();
				}
			}

			std::weak_ptr<data_type> _data;
			callback _cb;
			std::weak_ptr<void> _tie;
			bool _tied;
		};

	} // namespace detail end



	typedef std::shared_ptr<void> slot;

	template <typename signature>
	class signal;

	template <typename ret, typename... args>
	class signal<ret(args...)> : boost::noncopyable
	{
	public:
		typedef std::function<void(args...)> callback;
		typedef detail::signalimpl<callback> signalimpl;
		//typedef signalimpl::slotlist slotlist;
		typedef detail::slotimpl<callback> slotimpl;

		signal() : _impl(std::make_shared<signalimpl>()) {}

		~signal() {}

		slot connect(callback&& func) {
			auto slot_impl = std::make_shared<slotimpl>(_impl, std::forward<callback>(func));
			add(slot_impl);
			return slot_impl;
		}

		slot connect(callback&& func, const std::shared_ptr<void>& tie) {
			auto slot_impl = std::make_shared<slotimpl>(_impl, func, tie);
			add(slot_impl);
			return slot_impl;
		}

		void call(args&&... argv) {
			std::shared_ptr<signalimpl::slotlist> slots;
			signalimpl& impl(*_impl);
			{
				std::lock_guard<std::mutex> lock(impl._mutex);
				slots = impl._slots;
			}
			signalimpl::slotlist& s(*slots);
			for (auto slot : s) {
				auto slot_impl = slot.lock();
				if (slot_impl) {
					if (slot_impl->_tied) {
						if (slot_impl->_tie.lock()) {
							slot_impl->_cb(argv...);
						}
					} else {
						slot_impl->_cb(argv...);
					}
				}
			}
		}

	private:
		void add(const std::shared_ptr<slotimpl>& slot) {
			signalimpl& impl(*_impl);
			{
				std::lock_guard<std::mutex> lock(impl._mutex);
				impl.copy_on_write();
				impl._slots->push_back(slot);
			}
		}

		const std::shared_ptr<signalimpl> _impl;
	};
	
}
