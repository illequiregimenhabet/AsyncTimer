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


#include "pch.h"
#include <functional>
#include <future>
#include <thread>
#include <mutex>

#define __linux
#include "AsyncTimer.h"

#ifdef __linux

struct ParameterNest
{
	std::function<void()> CallBack;

	template<typename Func, typename... Args>
	void Setup(Func func1, Args&&... args)
	{
		auto f = std::bind(func1, std::forward<Args>(args)...);
		CallBack = [=]()
		{
			f();
		};
	}

	void Process()
	{
		CallBack();
	}
};
#endif

// TODO: refactor into different files.
class AsyncTimer
{
public:

	AsyncTimer() = default;

	AsyncTimer(const int timeout)
		: _end(false),
		  _timeout(std::chrono::milliseconds(timeout))
	{
	}
	template<typename Duration>
	AsyncTimer(Duration timeout)
		: _end(false),
		  _timeout(std::chrono::duration_cast<decltype(_timeout)>(timeout))
	{
		_timeout = timeout;
	}

	template<typename Duration, typename... Args>
	AsyncTimer(Duration timeout, void execute(Args...), Args ...args)
		: _end(false),
		  _timeout(std::chrono::duration_cast<decltype(_timeout)>(timeout))
	{
		Run(execute, std::forward<Args>(args)...);
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

#ifdef __linux
    // TODO: allow use of timer function with placeholders
    // TODO: enable function return types for handles.
    template<typename... Args>
    void bind(void a(Args...), Args&&... args)
    {
		_boundFunction.Setup(a, std::forward<Args>(args)...);
    }
#endif
    // TODO: allow re-execution, and use with placeholders - latter is unlikely.
    // TODO: allow the use of timer event information - which iteration etc.
    template<typename... Args>
    void Run(void execution(Args...), Args&&... args)
    {
	  {
	    std::lock_guard<std::mutex> lock(_muStop);
		if (!_end)
			_end = true;
	  }
      _fut = std::async(std::launch::async,
      [&]{
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
    void Stop()
    {
        { // todo checking?
            std::lock_guard<std::mutex> lock(_muStop);
            _end = true;
        }
    }
private:
	bool _end;
    std::chrono::milliseconds _timeout;
    std::mutex _muStop; 
    std::future<int> _fut; // type required, int is only a placement type.
#ifdef __linux
    struct ParameterNest _boundFunction;
#endif
} ;

#include <iostream>

void print(int a)
{
  std::cout << a << std::endl;
}

int main() {
  AsyncTimer t;
  t.SetTimeout(300);
  t.Run(&print, 1);
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  std::cout << "Hello World!\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}
