
#ifndef HELPER_TEMPLATES_HH
#define HELPER_TEMPLATES_HH

#include <type_traits>

#include <iRRAM/core.h>

namespace iRRAM {

/* helper for reduced overload visibility of binary operator declarations */
template <typename Base,typename EquivBase,typename Compat,typename Ret = Base>
using enable_if_compat = typename std::enable_if<
	std::is_same<typename std::remove_cv<EquivBase>::type,
	             typename std::remove_cv<Base>::type
	            >::value &&
	std::is_convertible<Compat,Base>::value
,Ret>::type;

template <typename Base,typename Ret = bool>
struct conditional_comparison_overloads {
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator<(const A &a, const B &b) { return a<Base(b); }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator<(const B &b, const A &a) { return Base(b)<a; }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator>(const A &a, const B &b) { return a>Base(b); }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator>(const B &b, const A &a) { return Base(b)>a; }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator<=(const A &a, const B &b) { return a<=Base(b); }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator<=(const B &b, const A &a) { return Base(b)<=a; }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator>=(const A &a, const B &b) { return a>=Base(b); }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator>=(const B &b, const A &a) { return Base(b)>=a; }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator==(const A &a, const B &b) { return a==Base(b); }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator==(const B &b, const A &a) { return Base(b)==a; }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator!=(const A &a, const B &b) { return a!=Base(b); }
	template <typename A,typename B> friend enable_if_compat<Base,A,B,Ret> operator!=(const B &b, const A &a) { return Base(b)!=a; }
};

template <typename...> struct disjunction : public std::false_type {};

template <typename C,typename... D> struct disjunction<C,D...> {
	static constexpr bool value = C::value || disjunction<D...>::value;
};

template <typename T> struct negation {
	static constexpr bool value = !T::value;
};

template <typename... T>
using conjunction = negation<disjunction<negation<T>...>>;

template <typename C> struct is_continuous : public std::false_type {};
template <typename C> struct is_continuous<C &> {
	static constexpr bool value = is_continuous<C>::value;
};
template <typename C> struct is_continuous<const C &> {
	static constexpr bool value = is_continuous<C>::value;
};
template <typename C> struct is_continuous<C &&> {
	static constexpr bool value = is_continuous<C>::value;
};

template <> struct is_continuous<REAL> : public std::true_type {};
template <> struct is_continuous<REALMATRIX> : public std::true_type {};
template <> struct is_continuous<SPARSEREALMATRIX> : public std::true_type {};
template <> struct is_continuous<COMPLEX> : public std::true_type {};
template <> struct is_continuous<INTERVAL> : public std::true_type {};
template <typename... Args> struct is_continuous<FUNCTION<REAL,Args...>> : public std::true_type {};

template <typename... T> using any_continuous = disjunction<is_continuous<T>...>;

}

#endif
