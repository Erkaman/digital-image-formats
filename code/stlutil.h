#ifndef _STLUTIL_H_
#define _STLUTIL_H_

#include <vector>
#include <string>

template<typename T> typename std::vector<T>::size_type findIndex(
    std::vector<T> strs,
    const T & str)
{
    typename std::vector<T>::iterator foundIter = find(strs.begin(),strs.end(), str);
    return std::distance(strs.begin(), foundIter);
}

template<typename T> std::vector<T> singleVector(T b)
{
    std::vector<T> s;

    s.push_back(b);

    return s;
}


#endif /* _STLUTIL_H_ */
