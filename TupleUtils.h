#pragma once
#include <fstream>
#include <tuple>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

struct callback
{
    template<typename T>
    void operator()(T&& t, ostream& stream)
    {
        stream << t << " ";
    }
};

template<int index, typename Callback, typename... Args>
struct iterate
{
    static void next(tuple<Args...>& t, Callback callback, ostream& stream)
    {
        iterate<index - 1, Callback, Args...>::next(t, callback, stream);
        callback(get<index>(t), stream);
    }
};

template<typename Callback, typename... Args>
struct iterate<0, Callback, Args...>
{
    static void next(tuple<Args...>& t, Callback callback, ostream& stream)
    {
        callback(get<0>(t), stream);
    }
};

template<typename Callback, typename... Args>
struct iterate<-1, Callback, Args...>
{
    static void next(tuple<Args...>& t, Callback callback, ostream& stream) {}
};


template<typename Callback, typename... Args>
void forEach(tuple<Args...>& t, Callback callback, ostream& stream)
{
    const int size = tuple_size<tuple<Args...>>::value;
    iterate<size - 1, Callback, Args...>::next(t, callback, stream);
}

template<typename _CharT, typename _Traits, typename... Args>
basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& stream, tuple<Args...>& t)
{
    forEach(t, callback(), stream);

    return stream;
}
