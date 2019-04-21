#include <functional>
#include <future>
#include <thread>
#include <mutex>
#include <exception>

// TODO: refactor into different files.
// please note that this class does not make any attempt to avoid
// data-races and you must implement this functionality within the
// functions passed to it. This is due to the impossibility of us
// knowing what is in your function. If c++ has a way, make an issue.
class AsyncTimer
{
public:
    AsyncTimer() = default;
    // TODO: set execution information for storage..
    AsyncTimer(int timeout)
    {
      _timeout = timeout;
    }
    ~AsyncTimer()
    {
        _end = true;
        // TODO: necessity?
        if (_fut.get() != 0)
        {
            throw std::runtime_error("Something went drastically wrong...");
        }
    }
    void SetTimeout(int timeout)
    {
      _timeout = timeout;
    }

    // TODO: allow use of timer function with placeholders
    // TODO: enable function return types for handles.
    template<typename... Args>
    void bind(void a(Args...), Args&&... args)
    {
      // TODO: 'store' our parameter pack rather than binding it to an un-'storable' function
      auto f = std::bind(a, std::forward<Args>(args)...);
    }
    // TODO: allow re-execution, and use with placeholders
    // TODO: allow the use of timer event information - which iteration etc.
    template<typename... Args>
    void Run(void execution(Args...), Args&&... args)
    {
      _fut = std::async(std::launch::async,
      [&]{
        auto f = std::bind(execution, std::forward<Args>(args)...);
        while (true)
        {
          // check whether the main thread has been stopped
          {
            std::lock_guard<std::mutex> lock(_muStop);
            if (_end == true)
              break;
          }
          f();
          std::this_thread::sleep_for(std::chrono::milliseconds(_timeout));
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
  int _timeout;
  std::mutex _muStop; 
  bool _end = false;
  std::future<int> _fut;
  //auto _bound_function;
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
