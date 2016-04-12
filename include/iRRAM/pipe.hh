
#ifndef PIPE_HH
#define PIPE_HH

#include <cstdio>

#include <vector>
#include <memory>
#include <shared_mutex>
#include <atomic>

#include <iRRAM/core.h>
#include <iRRAM/pipe.h>
#include <iRRAM/TaylorModel.hh>

namespace iRRAM {

typedef unsigned effort_t;

template <typename T>
struct OSock {
	virtual ~OSock() {}
	virtual void put(T, effort_t=ACTUAL_STACK.prec_step) = 0;
};

template <typename T>
struct ISock {
	virtual ~ISock() {}
	virtual T get(effort_t=ACTUAL_STACK.prec_step) const = 0;
};

class Process;

typedef std::shared_ptr<Process> Process_t;
template <typename T> using OSock_t = std::shared_ptr<OSock<T>>;
template <typename T> using ISock_t = std::shared_ptr<ISock<T>>;

class Process : public std::enable_shared_from_this<Process> {

	struct BaseSock { virtual ~BaseSock() noexcept {} };

	template <typename T>
	class Sock : public OSock<T>, public ISock<T>, public BaseSock {
		const   std::weak_ptr<Process>      process;
		mutable std::shared_timed_mutex     mtx;
		mutable std::condition_variable_any cond;
		        effort_t                    effort;
		        T                           data;

		friend class Process;
	public:
		explicit Sock(Process_t src) noexcept;

		Sock(const Sock &)             = delete;
		Sock & operator=(const Sock &) = delete;

		T    get(     effort_t e) const override;
		void put(T v, effort_t e)       override;
	};

	std::vector<std::shared_ptr<BaseSock>> outputs;
	std::vector<std::shared_ptr<BaseSock>> inputs;
	std::thread thr;
	std::atomic<bool> cancelled;

	effort_t max_effort_requested;
	std::condition_variable output_requested;
	std::mutex mtx_outputs; /* protects max_effort_requested and output_requested */

	struct hide_constructor;

	friend Process_t make_process();
	void run(effort_t e);

public:
	Process(hide_constructor); /* prevent instantiation by users */
	~Process();

	template <typename T> OSock_t<T> out_sock();
	template <typename T> ISock_t<T> connect(const OSock_t<T> &s);

	template <typename Func,typename... Args>
	void exec(int argc, const char *const *argv, Func &&f, Args &&... args);
};

/* required for C support */
extern template class Process::Sock<int>;
extern template class Process::Sock<double>;
extern template class Process::Sock<INTEGER>;
extern template class Process::Sock<RATIONAL>;
extern template class Process::Sock<DYADIC>;
extern template class Process::Sock<LAZY_BOOLEAN>;
extern template class Process::Sock<REAL>;
extern template class Process::Sock<COMPLEX>;
extern template class Process::Sock<REALMATRIX>;
extern template class Process::Sock<SPARSEREALMATRIX>;
extern template class Process::Sock<TM>;

/******************************************************************************
 * Sock<T>
 ******************************************************************************/

template <typename T>
Process::Sock<T>::Sock(Process_t src) noexcept : process(src) {}

template <typename T>
T Process::Sock<T>::get(effort_t e) const
{
	printf("Sock::get w/ effort %u\n", e);
	if (auto p = process.lock()) {
		std::shared_lock<decltype(mtx)> lock(mtx);
		p->run(e);
		cond.wait(lock, [&]{return e <= effort;});
		return data;
	}
	throw "not connected: src Process gone";
}

template <typename T>
void Process::Sock<T>::put(T v, effort_t e)
{
	printf("Sock::put w/ effort %u\n", e);
	std::unique_lock<decltype(mtx)> lock(mtx);
	data = std::move(v);
	effort = e;
	cond.notify_all();
}

/******************************************************************************
 * Process
 ******************************************************************************/

template <typename T>
OSock_t<T> Process::out_sock()
{
	auto r = std::make_shared<Sock<T>>(shared_from_this());
	outputs.push_back(r);
	return r;
}

template <typename T>
ISock_t<T> Process::connect(const OSock_t<T> &s)
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

template <typename Func,typename... Args>
void Process::exec(int argc, const char *const *argv, Func &&f, Args &&... args)
{
	printf("Process::exec(%d,%p,...)\n", argc, (void *)argv);
	thr = std::thread([&f,&args...,this](int argc, const char *const *argv) -> void {
		printf("Process::exec(%d,%p,...) -> thread\n", argc, (void *)argv);
		iRRAM_initialize(argc, argv);
		iRRAM_exec([&]{
			printf("Process iterating...\n");
			f(args...);
			std::unique_lock<decltype(mtx_outputs)> lock(mtx_outputs);
			output_requested.wait(lock, [&]{
				return cancelled ||
				       ACTUAL_STACK.prec_step < max_effort_requested;
			});
			iRRAM_infinite = !cancelled;
			return 0;
		});
	}, argc, argv);
}

/******************************************************************************
 * static functions
 ******************************************************************************/

Process_t make_process();

} /* end namespace iRRAM */

#endif
