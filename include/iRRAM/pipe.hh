
#ifndef PIPE_HH
#define PIPE_HH

#include <cstdio>

#include <future>
#include <vector>
#include <memory>
#include <shared_mutex>

#include <iRRAM/core.h>

namespace iRRAM {

typedef unsigned effort_t;

class Process;

template <typename T>
struct OSock {
	virtual ~OSock() noexcept {}
	virtual void put(T, effort_t) = 0;
};
template <typename T>
struct ISock {
	virtual ~ISock() noexcept {}
	virtual T get(effort_t) const = 0;
};

class Process : public std::enable_shared_from_this<Process> {

	struct BaseSock { virtual ~BaseSock() noexcept {} };

	template <typename T>
	class Sock : public OSock<T>, public ISock<T>, public BaseSock {
		const   std::weak_ptr<Process>      process;
		mutable std::shared_timed_mutex     mtx;
		mutable std::condition_variable_any cond;
			effort_t                    effort;
			std::atomic<bool>           valid_flag;
			T                           data;

		friend class Process;
	public:
		explicit Sock(std::shared_ptr<Process> src) noexcept : process(src) {}

		Sock(const Sock &) = delete;
		Sock & operator=(const Sock &) = delete;

		T    get(effort_t e) const;

		void put(T v, effort_t e)
		{
			std::unique_lock<decltype(mtx)> lock(mtx);
			data = std::move(v);
			effort = e;
			valid_flag = true;
			cond.notify_all();
		}
	};

	effort_t current_effort_;
	std::vector<std::shared_ptr<BaseSock>> outputs;
	std::vector<std::shared_ptr<BaseSock>> inputs;
	std::thread thr;
	std::atomic<bool> cancelled;

	/* called by the iRRAM thread this process represents I/Os for */
	void commencing(effort_t current_effort)
	{
		/* TODO: sync */
		current_effort_ = current_effort;
		/* TODO: create promises */
	}
public:
	~Process()
	{
		cancelled = true;
		if (thr.joinable())
			thr.join();
	}

	template <typename T>
	std::shared_ptr<OSock<T>> out_sock()
	{
		auto r = std::make_shared<Sock<T>>(shared_from_this());
		outputs.push_back(r);
		return r;
	}

	template <typename T>
	std::shared_ptr<ISock<T>> connect(const std::shared_ptr<OSock<T>> &s)
	{
		auto ss = std::static_pointer_cast<Sock<T>>(s);
		if (auto p = ss->process.lock()) {
			if (p == shared_from_this())
				throw "error: socket connect circular";
			inputs.push_back(ss);
			return ss;
		}
		throw "not connected: src Process gone";
	}

	void run(effort_t e) { printf("Process::run(%u)\n", e); }

	template <typename Func,typename... Args>
	void exec(int argc, char **argv, Func &&f, Args &&... args)
	{
		thr = std::thread([&]{
			iRRAM_initialize(argc, argv);
			while (!cancelled) {
				iRRAM_exec([&]{f(std::forward<Args>(args)...); return 0;});
			}
		});
	}

	/* general getter */
	effort_t current_effort() { /* TODO: sync */ return current_effort_; }
};

template <typename T> T Process::Sock<T>::get(effort_t e) const
{
	if (auto p = process.lock()) {
		std::shared_lock<decltype(mtx)> lock(mtx);
		p->run(e);
		cond.wait(lock, [&]{return valid_flag && e <= effort;});
		return data;
	}
	throw "not connected: src Process gone";
}

}

#endif
