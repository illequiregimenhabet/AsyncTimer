/*
 * This program is designed to create a background timer that calls a function
 * provided by the user.
	Copyright(C) 2019 'illequiregimenhabet' - GitHub

	This program is free software : you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once
#include <future>
#include <functional>
#include <chrono>
#include <mutex>

namespace AST
{
	struct ParameterNest
	{
		std::function<void()> CallBack;

		template<typename Func, typename... Args>
		void Setup(Func func1, Args&&... args)
		{
			auto f = std::bind(func1, std::forward<Args>(args)...);
			CallBack = [=]()
			{
				f(); // goes out of scope?
			};
		}

		void Process()
		{
			CallBack();
		}
	};

	// TODO: refactor into different files.
	// please note that this class does not make any attempt to avoid
	// data-races and you must implement this functionality within the
	// functions passed to it. This is due to the impossibility of us
	// knowing what is in your function. If c++ has a way, make an issue.
	class AsyncTimer
	{
	public:

		AsyncTimer() = default;

		AsyncTimer(const int timeout)
			: _timeout(std::chrono::milliseconds(timeout))
		{
		}
		template<typename Duration>
		AsyncTimer(Duration timeout)
			: _timeout(std::chrono::duration_cast<decltype(_timeout)>(timeout))
		{
			_timeout = timeout;
		}

		template<typename Duration, typename... Args>
		AsyncTimer(Duration timeout, void execute(Args...), Args ...args)
			: _timeout(std::chrono::duration_cast<decltype(_timeout)>(timeout))
		{
			// store param-pack

		}

		~AsyncTimer()
		{
			_end = true; // stop the while(true) loop
		}

		template<typename Duration>
		void SetTimeout(Duration timeout)
		{
			_timeout = std::chrono::duration_cast<decltype(_timeout)>(timeout);
		}

		void SetTimeout(const int timeout)
		{
			_timeout = std::chrono::milliseconds(timeout);
	}

		// TODO: allow use of timer function with placeholders
		// TODO: enable function return types for handles.
		template<typename... Args>
		void bind(void a(Args...), Args&&... args)
		{
			_boundFunction.Setup(a, std::forward<Args>(args)...);
		}

		void Run()
		{
			_fut = std::async(std::launch::async,
				[&] {
				while (true)
				{
					// check whether the main thread has been stopped
					{
						std::lock_guard<std::mutex> lock(_muStop);
						if (_end)
							break;
					}
					_boundFunction.Process();
					std::this_thread::sleep_for(_timeout);
				}
				return 0;
			}
			);
		}

		// TODO: allow re-execution, and use with placeholders - latter is unlikely.
		// TODO: allow the use of timer event information - which iteration etc.
		template<typename... Args>
		void Run(void execution(Args...), Args&&... args)
		{
			_fut = std::async(std::launch::async,
				[&] {
				_boundFunction.Setup(execution, std::forward<Args>(args)...);
				while (true)
				{
					// check whether the main thread has been stopped
					{
						std::lock_guard<std::mutex> lock(_muStop);
						if (_end)
							break;
					}
					_boundFunction.Process();
					std::this_thread::sleep_for(_timeout);
				}
				return 0;
			}
			);
		}
		void Stop()
		{
			{ // todo checking?
				std::lock_guard<std::mutex> lock(_muStop);
				_end = true;
			}
		}
	private:
		std::chrono::milliseconds _timeout;
		std::mutex _muStop;
		bool _end = false;
		std::future<int> _fut; // type required, int is only a placement type.
		struct ParameterNest _boundFunction;
	};
}
