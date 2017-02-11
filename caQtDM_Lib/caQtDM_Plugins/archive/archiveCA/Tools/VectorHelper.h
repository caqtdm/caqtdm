#ifndef __VECTORHELPER_H__
#define __VECTORHELPER_H__

// Helper methods for Standard C++ vector template

// Add src vector to head/tail of dest vector
template <class T>
inline void vec_add_head (vector<T> &dest, const vector<T> &src)
{	dest.insert (dest.begin(), src.begin(), src.end());	}

template <class T>
inline void vec_add_tail (vector<T> &dest, const vector<T> &src)
{	dest.insert (dest.end(), src.begin(), src.end());	}

#endif //__VECTORHELPER_H__