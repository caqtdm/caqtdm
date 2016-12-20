// -*- c++ -*-

#ifndef _AUTO_PTR_H_
#define _AUTO_PTR_H_

// System
#include <stdlib.h> // size_t
// Tools
#include <NoCopy.h>

/// \ingroup Tools

/// AutoPtr: Holds pointer, deletes in destructor.
///
/// This AutoPtr is meant for holding a pointer to
/// one instance of something.
/// The instance is removed via delete.
///
/// For arrays that need to be removed via delete[],
/// see AutoArrayPtr.
///
/// @see AutoArrayPtr
template<class T>class AutoPtr
{
public:
    AutoPtr() : ptr(0) {};

    /// Assign pointer to AutoPtr.
    AutoPtr(T *in) : ptr(in) {};

    /// Copying from other AutoPtr causes rhs to release ownership.
    AutoPtr(AutoPtr &rhs)
    {
        ptr = rhs.release();
    }

    /// Destructor deletes owned pointer.
    ~AutoPtr()
    {
        assign(0);
    }

    /// Copying from other AutoPtr causes rhs to release ownership.
    AutoPtr & operator = (AutoPtr &rhs)
    {
        assign(rhs.release());
        return *this;
    }

    AutoPtr & operator = (T *p)
    {
        assign(p);
        return *this;
    }
    
    operator bool () const
    {
        return ptr != 0;
    }
    
    /// Allow access just like ordinary pointer.
    T &operator *() const
    {
        return *ptr;
    }

    /// Allow access just like ordinary pointer.
    T *operator ->() const
    {
        return ptr;
    }

    /// Allow access just like ordinary pointer.
    operator T * () const
    {
        return ptr;
    }

    /// Assign a new pointer, deleting existing one.
    void assign(T *new_ptr)
    {
        if (ptr)
            delete ptr;
        ptr = new_ptr;
    }

    /// Release ownership.
    ///
    /// The AutoPtr is set to 0, and the pointer that
    /// used to be handled by the AutoPtr is returned.
    ///
    /// @return Returns the original value.
    T * release()
    {
        T *tmp = ptr;
        ptr = 0;
        return tmp;
    }
    
private:
    PROHIBIT_DEFAULT_COPY(AutoPtr);
    T *ptr;
};

/// An auto-pointer for arrays.
///
/// The data must be allocates with new[],
/// and it will be released via delete [].
///
/// @see AutoPtr
template<class T>class AutoArrayPtr
{
public:
    /// Create emptry AutoArrayPtr.
    AutoArrayPtr() : arr(0) {};

    /// Assign pointer to AutoArrayPtr.
    AutoArrayPtr(T *in) : arr(in) {};

    /// Copying from other AutoArrayPtr causes rhs to release ownership.
    AutoArrayPtr(AutoArrayPtr &rhs)
    {
        arr = rhs.release();
    }

    /// Destructor deletes owned pointer.
    ~AutoArrayPtr()
    {
        assign(0);
    }

    /// Copying from other AutoArrayPtr causes rhs to release ownership.
    AutoArrayPtr & operator = (AutoArrayPtr &rhs)
    {
        assign(rhs.release());
        return *this;
    }

    /// Assign new pointer, delete the pointer currently held.
    AutoArrayPtr & operator = (T *p)
    {
        assign(p);
        return *this;
    }
    
    /// @return Returns true if pointer is valid.
    operator bool () const
    {
        return arr != 0;
    }

    /// @return Returns the current value.
    T * get() const
    {
        return arr;
    }
    
    /// Access one array element.
    T & operator [] (size_t i) const
    {
        return arr[i];
    }

    /// Assign a new pointer, deleting existing one.
    void assign(T *new_arr)
    {
        if (arr)
            delete [] arr;
        arr = new_arr;
    }

    /// Release ownership.
    ///
    /// The AutoArrayPtr is set to 0, and the pointer that
    /// used to be handled by the AutoArrayPtr is returned.
    ///
    /// @return Returns the original value.
    T * release()
    {
        T *tmp = arr;
        arr = 0;
        return tmp;
    }
    
private:
    PROHIBIT_DEFAULT_COPY(AutoArrayPtr);
    T *arr;
};

#endif
