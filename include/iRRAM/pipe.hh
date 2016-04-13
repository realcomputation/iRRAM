
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

template <typename T>
struct numeric_limits {
	static constexpr bool is_specialized = false;
	static constexpr bool is_continuous  = false;
};

template <>
struct numeric_limits<INTEGER> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = true;
	static constexpr bool is_continuous  = false;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<DYADIC> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = true;
	static constexpr bool is_continuous  = false;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<RATIONAL> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = true;
	static constexpr bool is_continuous  = false;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<LAZY_BOOLEAN> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = false;
	static constexpr bool is_continuous  = true;
	static constexpr bool is_bounded     = true;
};

template <>
struct numeric_limits<REAL> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = true;
	static constexpr bool is_continuous  = true;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<TM> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = true;
	static constexpr bool is_continuous  = true;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<COMPLEX> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = true;
	static constexpr bool is_continuous  = true;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<REALMATRIX> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = false;
	static constexpr bool is_continuous  = true;
	static constexpr bool is_bounded     = false;
};

template <>
struct numeric_limits<SPARSEREALMATRIX> {
	static constexpr bool is_specialized = true;
	static constexpr bool is_numeric     = false;
	static constexpr bool is_continuous  = true;
	static constexpr bool is_bounded     = false;
};







typedef ::iRRAM_effort_t effort_t;

template <typename T>
struct OSock {
	virtual ~OSock() { printf("~OSock()\n"); }
	virtual void put(T, effort_t=ACTUAL_STACK.prec_step) = 0;
};

template <typename T>
struct ISock {
	virtual ~ISock() { printf("~ISock()\n"); }
	virtual T get(effort_t=ACTUAL_STACK.prec_step) const = 0;
};

class Process;

typedef std::shared_ptr<Process> Process_t;
template <typename T> using OSock_t = std::shared_ptr<OSock<T>>;
template <typename T> using ISock_t = std::shared_ptr<ISock<T>>;











class Process : public std::enable_shared_from_this<Process> {

	struct BaseSock {
		const   std::shared_ptr<Process>    process;
		BaseSock(Process_t src) noexcept;
		virtual ~BaseSock() noexcept {}
	};

	template <typename T>
	class Sock : public OSock<T>, public ISock<T>, public BaseSock {
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

	std::vector<std::weak_ptr<BaseSock>> outputs;
	std::vector<std::shared_ptr<BaseSock>> inputs;
	std::thread thr;
	std::atomic<bool> cancelled;
	std::string id;

	effort_t max_effort_requested;
	std::condition_variable output_requested;
	std::mutex mtx_outputs; /* protects max_effort_requested and output_requested */

	struct hide_constructor;

	friend Process_t make_process(std::string);
	void run(effort_t e);

	friend int ::iRRAM_osock_get(iRRAM_simple_t *result, iRRAM_osocket_t *s, iRRAM_effort_t effort);

public:
	Process(std::string id, hide_constructor); /* prevent instantiation by users */
	~Process();

	template <typename T> OSock_t<T> out_sock();
	template <typename T> ISock_t<T> connect(const OSock_t<T> &s);

	template <typename Func,typename... Args>
	void exec(int argc, const char *const *argv, Func &&f, Args &&... args);
};

/* required for C support */
extern template class Process::Sock<int>;
extern template class Process::Sock<double>;
extern template class Process::Sock<std::string>;
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
 * BaseSock
 ******************************************************************************/

Process::BaseSock::BaseSock(Process_t src) noexcept : process(src) {}

/******************************************************************************
 * Sock<T>
 ******************************************************************************/

template <typename T>
Process::Sock<T>::Sock(Process_t src) noexcept : BaseSock(src), effort(0) {}

template <typename T>
T Process::Sock<T>::get(effort_t e) const
{
	if (e <= effort)
		return data;
	if (auto p = process) {
		printf("[%08x] Sock[out <- %s]::get w/ effort %u\n", std::this_thread::get_id(), p->id.c_str(), e);
		std::shared_lock<decltype(mtx)> lock(mtx);
		p->run(e);
		cond.wait(lock, [&]{return e <= effort;});
		return data;
	}
	throw "Sock::get: not connected: src Process gone";
}

template <typename T>
void Process::Sock<T>::put(T v, effort_t e)
{
	printf("[%08x] Sock[out <- %s]::put w/ effort %u\n", std::this_thread::get_id(),
	       [p = process]{ return p ? p->id.c_str(): "<disconn>"; }(),
	       e);
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
	auto s = std::make_shared<Sock<T>>(shared_from_this());
	outputs.emplace_back(s);
	return s;
}

template <typename T>
ISock_t<T> Process::connect(const OSock_t<T> &s)
{
	auto ss = std::static_pointer_cast<Sock<T>>(s);
	if (auto p = ss->process) {
		if (p == shared_from_this())
			throw "error: socket connect circular";
		inputs.push_back(ss);
		return ss;
	}
	throw "connect: not connected: src Process gone";
}

template <typename Func,typename... Args>
void Process::exec(int argc, const char *const *argv, Func &&f, Args &&... args)
{
	printf("[%08x] Process(%s)::exec(%d,%p,...)\n", std::this_thread::get_id(), id.c_str(), argc, (void *)argv);
	thr = std::thread([g = std::bind(f, std::forward<Args>(args)...),this]
	                  (int argc, const char *const *argv) {
		printf("[%08x] Process(%s)::exec(%d,%p,...) -> thread %08x\n", std::this_thread::get_id(), id.c_str(), argc, (void *)argv, std::this_thread::get_id());
		iRRAM_initialize(argc, argv);
		try {
		iRRAM_exec([&]{
			printf("[%08x] Process(%s) iterating w/ effort %u...\n", std::this_thread::get_id(), id.c_str(), (effort_t)ACTUAL_STACK.prec_step);
			try {
				std::vector<std::shared_ptr<BaseSock>> locked_outputs(outputs.size());
				for (auto &s : outputs)
					locked_outputs.push_back(s.lock());
				g();
			} catch (const Iteration &) {
				printf("[%08x] Process(%s) caught exception, reiterating...\n", std::this_thread::get_id(), id.c_str());
				throw;
			}
			printf("[%08x] Process(%s) finished computation g()\n", std::this_thread::get_id(), id.c_str());
			std::unique_lock<decltype(mtx_outputs)> lock(mtx_outputs);
			output_requested.wait(lock, [&]{
				return cancelled ||
				       (effort_t)ACTUAL_STACK.prec_step < max_effort_requested;
			});
			iRRAM_infinite = !cancelled;
			return 0;
		});
		} catch (const char *msg) { printf("[%08x] Process(%s) exception: %s\n", std::this_thread::get_id(), id.c_str(), msg); }
		printf("[%08x] Process(%s) thread finishes\n", std::this_thread::get_id(), id.c_str());
	}, argc, argv);
}

/******************************************************************************
 * static functions
 ******************************************************************************/

Process_t make_process(std::string id);

} /* end namespace iRRAM */

#endif
