#ifndef _EMILIA_ENDIAN_H_
#define _EMILIA_ENDIAN_H_

#define EMILIA_LITTLE_ENDIAN    1
#define EMILIA_BIG_ENDIAN       2

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

namespace emilia
{

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T v)
{
    return (T)bswap_16((uint16_t)v);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T v)
{
    return (T)bswap_32((uint32_t)v);
}


template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T v)
{
    return (T)bswap_64((uint64_t)v);
}

#if BYTE_ORDER == BIG_ENDIAN
#define EMILIA_ORDER EMILIA_BIG_ENDIAN
#else
#define EMILIA_ORDER EMILIA_LITTLE_ENDIAN
#endif

#if EMILIA_ORDER == EMILIA_BIG_ENDIAN
template <class T>
T byteswapOnLittleEndian(T v)
{
    return v;
}

template <class T>
T byteswapOnBigEndian(T v)
{
    return byteswap(v);
}
#else

template <class T>
T byteswapOnLittleEndian(T v)
{
    return byteswap(v);
}

template <class T>
T byteswapOnBigEndian(T v)
{
    return v;
}
#endif

}

#endif