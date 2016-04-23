
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






#define iRRAM_PIPE_DEBUG(fmt, ...) \
	iRRAM_DEBUG2(0,"[%08x] " fmt,std::this_thread::get_id(),__VA_ARGS__)

#define iRRAM_SOCK_DEBUG(fmt, ...) \
	iRRAM_PIPE_DEBUG("Sock[out <- %s]" fmt,process->id.c_str(),__VA_ARGS__)






class Process : public std::enable_shared_from_this<Process> {

	struct BaseSock {
		const   std::shared_ptr<Process>    process;
		explicit BaseSock(Process_t src) noexcept;
		virtual ~BaseSock() noexcept { iRRAM_SOCK_DEBUG("%s","::~BaseSock()\n"); }
	};

	typedef std::shared_ptr<BaseSock> BaseSock_t;

	template <typename T>
	class Sock : public BaseSock, public OSock<T>, public ISock<T> {
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
	iRRAM_SOCK_DEBUG("::get w/ effort %u\n", e);
	std::shared_lock<decltype(mtx)> lock(mtx);
	process->run(e);
	cond.wait(lock, [&]{return e <= effort;});
	return data;
}

template <typename T>
void Process::Sock<T>::put(T v, effort_t e)
{
	iRRAM_SOCK_DEBUG("::put w/ effort %u\n",e);
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

#define iRRAM_PROCESS_DEBUG(fmt, ...) \
	iRRAM_PIPE_DEBUG("Process(%s)" fmt,id.c_str(),__VA_ARGS__)

template <typename Func,typename... Args>
void Process::exec(int argc, const char *const *argv, Func &&f, Args &&... args)
{
	iRRAM_PROCESS_DEBUG("::exec(%d,%p,...)\n", argc, (void *)argv);
	std::thread([g = std::bind(f, std::forward<Args>(args)...),this]
	            (int argc, const char *const *argv) {
		iRRAM_PROCESS_DEBUG("::exec(%d,%p,...) -> thread %08x\n",
		                    argc, (void *)argv, std::this_thread::get_id());
		iRRAM_initialize(argc, argv);
		try {
		iRRAM_exec([&]{
			iRRAM_PROCESS_DEBUG(" iterating w/ effort %u...\n",
			                    (effort_t)ACTUAL_STACK.prec_step);
			try {
				std::vector<BaseSock_t> locked_outputs(outputs.size());
				for (auto &s : outputs)
					locked_outputs.push_back(s.lock());
				g();
			} catch (const Iteration &) {
				iRRAM_PROCESS_DEBUG("%s", " caught exception, reiterating...\n");
				throw;
			}
			iRRAM_PROCESS_DEBUG("%s", " finished computation g()\n");
			std::unique_lock<decltype(mtx_outputs)> lock(mtx_outputs);
			output_requested.wait(lock, [&]{
				return cancelled ||
				       (effort_t)ACTUAL_STACK.prec_step < max_effort_requested;
			});
			iRRAM_infinite = !cancelled;
			return 0;
		});
		} catch (const char *msg) {
			iRRAM_PROCESS_DEBUG(" exception: %s\n", msg);
		}
		iRRAM_PROCESS_DEBUG("%s", " thread finishes\n");
	}, argc, argv).detach();
}

/******************************************************************************
 * static functions
 ******************************************************************************/

Process_t make_process(std::string id);

} /* end namespace iRRAM */

#endif
