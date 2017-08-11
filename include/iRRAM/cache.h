/*

iRRAM_cache.h -- caching routines for the iterative structure of the iRRAM library
 
Copyright (C) 2001-2003 Norbert Mueller
 
This file is part of the iRRAM Library.
 
The iRRAM Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.
 
The iRRAM Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.
 
You should have received a copy of the GNU Library General Public License
along with the iRRAM Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. 
*/

#ifndef iRRAM_CACHE_H
#define iRRAM_CACHE_H

#include <string>
#include <vector>

namespace iRRAM {

template <typename T,void (*clearfct)(T &)>
struct wrap_type {
	T v;
	wrap_type() noexcept(noexcept(T())) : v(T()) {}
	wrap_type(const T &v) noexcept(noexcept(T(v))) : v(v) {}
	wrap_type(wrap_type &&o) noexcept(noexcept(std::swap(v, o.v))) : wrap_type() { using std::swap; swap(v, o.v); }
	~wrap_type() noexcept { clearfct(v); }
	wrap_type & operator=(const T &_v) noexcept(noexcept(const_cast<T &>(_v)=_v)) { v = _v; return *this; }
	wrap_type & operator=(wrap_type o) noexcept(noexcept(std::swap(v, o.v))) { using std::swap; swap(v, o.v); return *this; }
	operator T() const noexcept { return v; }
};

template <typename T> struct get_type {
	typedef T type;
};

template <> struct get_type<bool> {
	static void clear(bool &){}
	typedef wrap_type<bool,clear> type;
};

template <> struct get_type<MP_type> {
	static void clear(MP_type &t){ if(t) MP_clear(t); }
	typedef wrap_type<MP_type,clear> type;
};

template <> struct get_type<MP_int_type> {
	static void clear(MP_int_type &t){ if (t) MP_int_clear(t); }
	typedef wrap_type<MP_int_type,clear> type;
};


class iRRAM_cache_type{
public:
virtual void clear()=0;
virtual void rewind() noexcept =0;
};

class cachelist{public:
iRRAM_cache_type* id[50];
};

template <class DATA> class iRRAM_cache : public iRRAM_cache_type
{
public:
std::vector<typename get_type<DATA>::type> data;
unsigned int current = 0;
bool active = false;

void put(const DATA& x){
  if (iRRAM_unlikely(!active)){activate();}
  data.emplace_back(x);
  current++;
};

bool get(DATA& x) noexcept(noexcept(x=x)) {
  if (current>=data.size())return false;
    x=data[current++];
  return true; 
}

void modify(const DATA& x) noexcept(noexcept(const_cast<DATA &>(x)=x)) {
  data[current-1]=x;
}

void rewind() noexcept {
  current=0;
};

void clear(){
  data.clear();
  active=false;
  current=0;
};

void activate(){
  data.clear();
  active=true;
  state->cache_active->id[state->max_active++]=this;
  current=0;
};

};

template <typename T> struct is_cacheable : std::false_type {};
template <> struct is_cacheable<bool> : std::true_type {};
template <> struct is_cacheable<short> : std::true_type {};
template <> struct is_cacheable<unsigned short> : std::true_type {};
template <> struct is_cacheable<int> : std::true_type {};
template <> struct is_cacheable<unsigned int> : std::true_type {};
template <> struct is_cacheable<long> : std::true_type {};
template <> struct is_cacheable<unsigned long> : std::true_type {};
template <> struct is_cacheable<long long> : std::true_type {};
template <> struct is_cacheable<unsigned long long> : std::true_type {};
template <> struct is_cacheable<float> : std::true_type {};
template <> struct is_cacheable<double> : std::true_type {};
template <> struct is_cacheable<void *> : std::true_type {};
template <> struct is_cacheable<MP_type> : std::true_type {};
template <> struct is_cacheable<MP_int_type> : std::true_type {};
template <> struct is_cacheable<std::string> : std::true_type {};
template <> struct is_cacheable<std::ostream *> : std::true_type {};
template <> struct is_cacheable<std::istream *> : std::true_type {};

struct iRRAM_thread_data_class final
: iRRAM_cache<bool>
, iRRAM_cache<short> // unused
, iRRAM_cache<unsigned short> // unused
, iRRAM_cache<int>
, iRRAM_cache<unsigned int> // unused
, iRRAM_cache<long> // unusused
, iRRAM_cache<unsigned long> // unused
, iRRAM_cache<long long> // unused
, iRRAM_cache<unsigned long long> // unused
, iRRAM_cache<float> // unused
, iRRAM_cache<double> // unused
, iRRAM_cache<void*> // unused
, iRRAM_cache<MP_type>
, iRRAM_cache<MP_int_type>
, iRRAM_cache<std::string>
, iRRAM_cache<std::ostream*>
, iRRAM_cache<std::istream*>
{
iRRAM_thread_data_class();
~iRRAM_thread_data_class();
};

} // namespace iRRAM

#endif //define iRRAM_CACHE_H
