#pragma once
#include <future>

namespace AST
{

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

	class AsyncTimer
	{
	public:
		AsyncTimer() = default;
		~AsyncTimer();

		AsyncTimer(const int timeout);

		template<typename Duration>
		AsyncTimer(Duration timeout);

		template<typename Duration, typename... Args>
		AsyncTimer(Duration timeout, void execute(Args...), Args ...args);

		template<typename Duration>
		void SetTimeout(Duration timeout);

		void SetTimeout(const int timeout);

#ifdef __linux
		// TODO: allow use of timer function with placeholders
		// TODO: enable function return types for handles.
		template<typename... Args>
		void bind(void a(Args...), Args&&... args);
#endif
		// TODO: allow re-execution, and use with placeholders - latter is unlikely.
		// TODO: allow the use of timer event information - which iteration etc.
		template<typename... Args>
		void Run(void execution(Args...), Args&&... args);

		void Stop();
	private:
		bool _end;
		std::chrono::milliseconds _timeout;
		std::mutex _muStop;
		std::future<int> _fut; // type required, int is only a placement type.
#ifdef __linux
		struct ParameterNest _boundFunction;
#endif
	};
}
