#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <NoCopy.h>

//CLASS Histogram
//
// ... has not been tested extensively
class Histogram
{
public:
    //* Create Histogram for values <I>start</I>, <I>end</I>, 
    // divided into <I>num</I> buckets
    Histogram(double start, double end, size_t num);
    ~Histogram();

    //* Reset hit counts
    void clear();

    //* Count one additional hit for <I>value</I>
    void hit(double value);

    void dump(FILE *f);
private:
    PROHIBIT_DEFAULT_COPY(Histogram);
    double _start, _width;
    size_t _num, _under, _over;
    size_t *_bucket;
};

inline Histogram::Histogram(double start, double end, size_t num)
{
    _num = num;
    _width = (end - start) / num;
    _bucket = new size_t[num];
    _under = _over = 0;
    clear ();
}

inline Histogram::~Histogram()
{
    delete [] _bucket;
}

inline void Histogram::clear()
{
    for (size_t i=0; i<_num; ++i)
        _bucket[i] = 0;
}

inline void Histogram::hit(double value)
{
    if (value < _start)
    {
        ++_under;
        return;
    }
    size_t index = size_t((value - _start) / _width);
    if (index >= _num)
    {
        ++_over;
        return;
    }
    ++_bucket[index];
}

inline void Histogram::dump(FILE *f)
{
    size_t i;
    for (i=0; i<h._num; ++i)
        fprintf(f, "|%5d ",  _bucket[i]);
    fprintf(f, "|\n");
    for (i=0; i<h._num; ++i)
        fprintf(f, "|______");
    fprintf(f, "|\n");
    double v = h._start + h._width/2;
    for (i=0; i<h._num; ++i)
    {
        fprintf(f, "|%5.2f ", v);
        v += h._width;
    }
    fprintf(f, "|\n");

    fprintf(f, "Below: %d\n", _under);
    fprintf(f, "Above: %d\n", _over);
}

#endif //__HISTOGRAM_H__



