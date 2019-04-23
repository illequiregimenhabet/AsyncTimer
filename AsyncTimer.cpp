#include "pch.h"
#include "AsyncTimer.h"

namespace AST
{

	AsyncTimer::~AsyncTimer()
	{
		_end = true;
	}

	AsyncTimer::AsyncTimer(const int timeout)
		: _end(false),
		_timeout(std::chrono::milliseconds(timeout))
	{
	}

	template<typename Duration>
	AsyncTimer::AsyncTimer(Duration timeout)
		: _end(false),
		_timeout(std::chrono::duration_cast<decltype(_timeout)>(timeout))
	{
		_timeout = timeout;
	}

	template<typename Duration, typename... Args>
	AsyncTimer::AsyncTimer(Duration timeout, void execute(Args...), Args ...args)
		: _end(false),
		_timeout(std::chrono::duration_cast<decltype(_timeout)>(timeout))
	{
		Run(execute, std::forward<Args>(args)...);
	}

	template<typename Duration>
	void AsyncTimer::SetTimeout(Duration timeout)
	{
		_timeout = std::chrono::duration_cast<decltype(_timeout)>(timeout);
	}
	void AsyncTimer::SetTimeout(const int timeout)
	{
		_timeout = std::chrono::milliseconds(timeout);
	}


#ifdef __linux
	// TODO: allow use of timer function with placeholders
	// TODO: enable function return types for handles.
	template<typename... Args>
	void AsyncTimer::bind(void a(Args...), Args&&... args)
	{
		_boundFunction.Setup(a, std::forward<Args>(args)...);
	}
#endif


	// TODO: allow re-execution, and use with placeholders - latter is unlikely.
	// TODO: allow the use of timer event information - which iteration etc.
	template<typename... Args>
	void AsyncTimer::Run(void execution(Args...), Args&&... args)
	{
		{
			std::lock_guard<std::mutex> lock(_muStop);
			if (!_end)
				_end = true;
		}
		_fut = std::async(std::launch::async,
			[&] {
#ifdef __linux
			_boundFunction.Setup(execution, std::forward<Args>(args)...);
#else
			auto f = std::bind(execution, std::forward<Args>(args)...);
#endif
			while (true)
			{
				// check whether the main thread has been stopped
				{
					std::lock_guard<std::mutex> lock(_muStop);
					if (_end)
						break;
				}
#ifdef __linux
				_boundFunction.Process();
#else
				f();
#endif
				std::this_thread::sleep_for(_timeout);
			}
			return 0;
		}
		);
	}

	void AsyncTimer::Stop()
	{
		{ // todo checking?
			std::lock_guard<std::mutex> lock(_muStop);
			_end = true;
		}
	}

}
