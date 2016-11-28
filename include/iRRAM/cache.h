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
  state.cache_active->id[state.max_active++]=this;
  current=0;
};

};

class iRRAM_thread_data_class { public:
iRRAM_thread_data_class();
~iRRAM_thread_data_class();
iRRAM_cache<bool> cache_b;
iRRAM_cache<short> cache_sh;
iRRAM_cache<unsigned short> cache_ush;
iRRAM_cache<int> cache_i;
iRRAM_cache<long> cache_l;
iRRAM_cache<unsigned long> cache_ul;
iRRAM_cache<double> cache_d;
iRRAM_cache<long long> cache_ll;
iRRAM_cache<unsigned int> cache_ui;
iRRAM_cache<unsigned long long> cache_ull;
iRRAM_cache<std::string> cache_s;
iRRAM_cache<float> cache_f;
iRRAM_cache<void*> cache_v;
iRRAM_cache<MP_type> cache_mp;
iRRAM_cache<MP_int_type> cache_mpi;
iRRAM_cache<std::ostream*> cache_os;
iRRAM_cache<std::istream*> cache_is;

};

} // namespace iRRAM

#endif //define iRRAM_CACHE_H
