#pragma once

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>

class CallBackTimer
{
public:
	CallBackTimer()
		:_execute(false)
	{}

	~CallBackTimer() {
		if (_execute.load(std::memory_order_acquire)) {
			stop();
		};
	}

	void stop()
	{
		_execute.store(false, std::memory_order_release);
		if (_thd.joinable())
			_thd.join();
	}

	void start(double interval, std::function<void(void)> func)
	{
		if (_execute.load(std::memory_order_acquire)) {
			stop();
		};
		_execute.store(true, std::memory_order_release);
		_thd = std::thread([this, interval, func]()
		{
			while (_execute.load(std::memory_order_acquire)) {
				func();
				std::this_thread::sleep_for(
					std::chrono::nanoseconds((int)(interval * 1000000.0)));
			}
		});
	}

	bool is_running() const noexcept {
		return (_execute.load(std::memory_order_acquire) &&
			_thd.joinable());
	}

private:
	std::atomic<bool> _execute;
	std::thread _thd;
};
