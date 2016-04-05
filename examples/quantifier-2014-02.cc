
#include <iRRAM.h>

namespace iRRAM {

namespace logic {

/*************************
 * implementation details
 *************************/
typedef unsigned var_id;

/************
 * interface
 ************/
enum type {
	Z, /* Integer */
	Q, /* Rational */
	D, /* Dyadic */
	R, /* Real */
	C, /* Complex */
};

struct var;
struct domain;

class context {
	//std::shared_ptr<context> parent;

	std::vector<domain> var_domains;

	var_id next_free_var_id(const domain &d)
	{
		var_domains.emplace_back(d);
		return var_domains.size();
	}

	friend var create_var(const std::shared_ptr<context> &ctx, const domain &d);
public:
	context() {}
	context(const context &) = delete;
	context & operator=(const context &) = delete;

	      domain & var_domain(var_id id)       { return var_domains[id]; }
	const domain & var_domain(var_id id) const { return var_domains[id]; }

	//context(const std::shared_ptr<context> &parent) : parent(parent) {}
};

struct var {
	const std::shared_ptr<context> ctx;
	const var_id id;

	var(const var &) = default;
	var & operator=(const var &) = default;
private:
	friend var create_var(const std::shared_ptr<context> &ctx, const domain &d);

	var(const std::shared_ptr<context> &ctx, var_id id)
	: ctx(ctx), id(id) {}
};

var create_var(const std::shared_ptr<context> &ctx, const domain &d)
{
	return var(ctx, ctx->next_free_var_id(d));
}

struct domain {};

template <typename... U>
struct set {
};

template <typename... U>
set<U...> unite(const set<U...> &a, const set<U...> &b);

//static const set<> EMPTY;

template <typename... U>
struct interval : public set<U...> {
};

template <typename R,typename... P>
struct fun_obj {
	virtual R operator()(const P &...) const = 0;
	virtual ~fun_obj() {}
};

template <typename R,typename... P>
struct fun {
	fun(const fun<R,P...> &) = default;
	fun(const std::shared_ptr<fun_obj<R,P...>> &f) : f(f) {}

	R           operator()(const P &... p) const { return (*f)(p...); }
	//set<R>      operator()(const set<P...> &) const;
	template <typename... Q>
	fun<R,Q...> operator()(const fun<P,Q...> &...) const;
private:
	std::shared_ptr<fun_obj<R,P...>> f;
};

namespace _impl {
template <typename R,typename... P>
struct fun_obj_funptr : public fun_obj<R,P...> {
	fun_obj_funptr(R (*f)(const P &...)) : f(f) {}
	R operator()(const P &... p) const { return f(p...); }
private:
	R (*f)(const P &...);
};
template <typename R,typename... P>
struct fun_obj_stdfun : public fun_obj<R,P...> {
	fun_obj_stdfun(const std::function<R(const P &...)> &f) : f(f) {}
	R operator()(const P &... p) const { return f(p...); }
private:
	std::function<R(const P &...)> f;
};
}

template <typename R,typename... P>
fun<R,P...> from_algorithm(R (*f)(const P &...))
{ return { std::make_shared<_impl::fun_obj_funptr<R,P...>>(f), }; }

template <typename R,typename... P>
fun<R,P...> from_algorithm(const std::function<R(const P &...)> &f)
{ return { std::make_shared<_impl::fun_obj_stdfun<R,P...>>(f), }; }

namespace _impl {
template <typename R,typename... P>
struct fun_add : public fun<R,P...> {
	fun<R,P...> a, b;
	fun_add(const fun<R,P...> &a, const fun<R,P...> &b) : a(a), b(b) {}
	R           operator()(const P &... p) { return a(p...)+b(p...); }
	set<R>      operator()(const set<P...> &s) { return a(s)+b(s); }
};
template <typename R>
struct fun_add<R> : public fun<R> {
	fun_add(const fun<R> &a, const fun<R> &b) : fun<R>(a() + b()) {}
};
template <typename R,typename... P>
struct fun_sub : public fun<R,P...> {
	fun<R,P...> a, b;
	fun_sub(const fun<R,P...> &a, const fun<R,P...> &b) : a(a), b(b) {}
	R           operator()(const P &... p) { return a(p...)-b(p...); }
	set<R>      operator()(const set<P...> &s) { return a(s)-b(s); }
};
template <typename R>
struct fun_sub<R> : public fun<R> {
	fun_sub(const fun<R> &a, const fun<R> &b) : fun<R>(a() - b()) {}
};
template <typename R,typename... P>
struct fun_mul : public fun<R,P...> {
	fun<R,P...> a, b;
	fun_mul(const fun<R,P...> &a, const fun<R,P...> &b) : a(a), b(b) {}
	R           operator()(const P &... p) { return a(p...)*b(p...); }
	set<R>      operator()(const set<P...> &s) { return a(s)*b(s); }
};
template <typename R>
struct fun_mul<R> : public fun<R> {
	fun_mul(const fun<R> &a, const fun<R> &b) : fun<R>(a() * b()) {}
};
template <typename R,typename... P>
struct fun_div : public fun<R,P...> {
	fun<R,P...> a, b;
	fun_div(const fun<R,P...> &a, const fun<R,P...> &b) : a(a), b(b) {}
	R           operator()(const P &... p) { return a(p...)/b(p...); }
	set<R>      operator()(const set<P...> &s) { return a(s)/b(s); }
};
template <typename R>
struct fun_div<R> : public fun<R> {
	fun_div(const fun<R> &a, const fun<R> &b) : fun<R>(a() / b()) {}
};
template <typename R,typename... P>
struct fun_mod : public fun<R,P...> {
	fun<R,P...> a, b;
	fun_mod(const fun<R,P...> &a, const fun<R,P...> &b) : a(a), b(b) {}
	R           operator()(const P &... p) { return a(p...)%b(p...); }
	set<R>      operator()(const set<P...> &s) { return a(s)%b(s); }
};
template <typename R>
struct fun_mod<R> : public fun<R> {
	fun_mod(const fun<R> &a, const fun<R> &b) : fun<R>(a() % b()) {}
};
template <typename R,typename P,typename... Q>
struct fun_comp : public fun<R,Q...> {
	fun<R,P> outer;
	fun<P,Q...> inner;
	fun_comp(const fun<R,P> &outer, const fun<P,Q...> &inner)
	: outer(outer), inner(inner) {}
	R           operator()(const Q &... q) { return outer(inner(q...)); }
	set<R>      operator()(const set<Q...> &s) { return outer(inner(s)); }
};
template <typename R,typename P>
struct fun_comp<R,P> : public fun<R> {
	fun_comp(const fun<R,P> &a, const fun<P> &b) : fun<R>(a(b())) {}
};
} /* end namespace _impl */

template <typename R,typename... P>
fun<R,P...> operator+(const fun<R,P...> &a, const fun<R,P...> &b)
{ return _impl::fun_add<R,P...>(a,b); }

template <typename R,typename... P>
fun<R,P...> operator-(const fun<R,P...> &a, const fun<R,P...> &b)
{ return _impl::fun_sub<R,P...>(a,b); }

template <typename R,typename... P>
fun<R,P...> operator*(const fun<R,P...> &a, const fun<R,P...> &b)
{ return _impl::fun_mul<R,P...>(a,b); }

template <typename R,typename... P>
fun<R,P...> operator/(const fun<R,P...> &a, const fun<R,P...> &b)
{ return _impl::fun_div<R,P...>(a,b); }

template <typename R,typename... P>
fun<R,P...> operator%(const fun<R,P...> &a, const fun<R,P...> &b)
{ return _impl::fun_mod<R,P...>(a,b); }

template <typename R,typename... P>
template <typename... Q>
fun<R,Q...> fun<R,P...>::operator()(const fun<P,Q...> &... b) const
{
	
	return {
		std::make_shared<_impl::fun_obj_stdfun<R,Q...>>(
			[=](const Q &... q) { return (*this)(b(q...)...); })
	};
}



template <typename T>
struct fun<T> {
	typedef T type;
	T value;

	fun(const T &v) : value(v) {}
	T operator()() const { return value; }

	operator T() const { return value; }
};

template <typename T> using constant = fun<T>;

} /* end namespace logic */
} /* end namespace iRRAM */

void compute()
{
	using namespace iRRAM;
	using namespace iRRAM::logic;
/*
	std::shared_ptr<context> ctx = std::make_shared<context>();

	var X = create_var(ctx, domain());*/
#if 1
	constant<REAL> A = pi();
	constant<REAL> B = REAL(42);

	fun<REAL> f = A * B;
	REAL x = f();

	cout << x / pi() << "\n";


	REAL (*_id)(const REAL &) = [](const REAL &r) { return r; };
	REAL (*_plus)(const REAL &, const REAL &) = [](const REAL &a, const REAL &b) { return a+b; };
	fun<REAL,REAL>       id_REAL = logic::from_algorithm(_id);
	fun<REAL,REAL,REAL>  plus = logic::from_algorithm(_plus);

	fun<REAL,REAL,REAL> plus42 =
		//logic::from_algorithm(std::function<REAL(const REAL &)>(std::bind(plus,A,std::placeholders::_1)));
		plus.operator()<REAL,REAL>(id_REAL,id_REAL);

	//fun<REAL,REAL,REAL> id_plus = id_REAL(plus);

	// cout << id_plus(A,B) << "\n";
	cout << (id_REAL(plus))(A,B) << "\n";

#else
	constant<double> A = pi().as_double();
	constant<double> B = 42;

	fun<double> f = A * B;
	double x = f();

	cout << x / pi().as_double() << "\n";
#endif
}
