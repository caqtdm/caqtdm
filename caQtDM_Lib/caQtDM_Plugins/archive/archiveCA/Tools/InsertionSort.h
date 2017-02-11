// -*- c++ -*-

/// \ingroup Index
/// InsertionSort implements a simple, well, insertion sort.
///
/// Yes, of course this sucks in comparison to e.g. quicksort
/// when your mind's set to N*N vs. N*log(N),
/// but it's actually quite fast for N up to maybe 10.
///
/// It requires that the elements of class T
/// can be copied and compared.
///
/// No checks for exceeding N, the max. number of entries.
template<class T, size_t N>
class InsertionSort
{
public:
    /// Constructor
    InsertionSort() : n(0) {}

    /// Sorted insertion of new item into array
    void insert(T item)
    {
        size_t i,j;
        for (i=0; i<n; ++i)
        {
            if (item < a[i])
            {
                for (j=n; j>i; --j)
                    a[j] = a[j-1];
                a[i] = item;
                ++n;
                return;
            }
        }
        a[n] = item;
        ++n;
    }

    /// Set array element.

    /// This allows write access. Feel free to mess up
    /// the sorted array by using this.
    ///
    void set(size_t i, const T &item)
    {   a[i] = item; }

    /// Convenince definition of operator [] (read-only)
    const T &operator [] (size_t i) const
    {   return a[i]; }
    
private:
    T a[N]; // the array of items
    size_t n; // current # of items in array
};

