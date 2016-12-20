#ifndef NOCOPY_H_
#define NOCOPY_H_

// Helper macro for declaring the (private) copy constructor
// and copy operator,
// with the intent of never defining it,
// to prevent usage of the default copy operators.
// See Meyer Item 27.
// Suggested for all classes with members that
// point to allocated memory.
#define PROHIBIT_DEFAULT_COPY(CLASS)    \
    CLASS(const CLASS &);               \
    CLASS & operator = (const CLASS &)

#endif /*NOCOPY_H_*/
