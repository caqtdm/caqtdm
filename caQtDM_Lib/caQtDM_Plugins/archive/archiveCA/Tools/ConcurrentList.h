#ifndef __CONCURRENTLIST_H__
#define __CONCURRENTLIST_H__

// Tools
#include <Guard.h>
#include <NoCopy.h>

/** \ingroup Tools
 *  A List that allows concurrent access.
 *  <p>
 *  One can add or remove elements while
 *  someone else is traversing the list.
 *  Of course the list is locked in add(),
 *  next(), .., but not for the full duration
 *  of traversal.
 */
class ConcurrentPtrList
{
public:
    /** Construct a new list. */
    ConcurrentPtrList();

    /** Delete list and all its elements. */
    virtual ~ConcurrentPtrList();
    
    /** @return Returns the mutex for the Guard passed to other methods. */
    OrderedMutex &getMutex() { return mutex; }
    
    /** @return Returns true if the list is empty. */
    bool isEmpty(Guard &guard);

    /** @return Returns number of entries. */
    size_t size(Guard &guard);
    
    /** Add an item to the list.
     *  <p>
     *  The position of that item on the list is
     *  not necessarily predicatable.
     *  Typically at the end, but may be at a 'reused' location.
     */
    void add(Guard &guard, void *item);

    /** Remove an item from the list.
     *  <p>
     *  @return Returns true if item was found and removed.
     */
    bool removeIfFound(Guard &guard, void *item);

    /** Remove an item from the list.
     *  <p>
     *  @exception GenericException if item is not found on the list.
     */
    void remove(Guard &guard, void *item);
    
    /** Obtain iterator, positioned at the start of the list. */
    class ConcurrentPtrListIterator iterator(Guard &guard);
    
private:
    PROHIBIT_DEFAULT_COPY(ConcurrentPtrList);
    OrderedMutex     mutex;
    class CPElement *list;
};

/** \ingroup Tools
 *  Iterator for ConcurrentPtrList.
 *  @see ConcurrentPtrList
 */
class ConcurrentPtrListIterator
{
public:
    /** Constructor. Users should use ConcurrentPtrList::iterator().
     *  @see ConcurrentPtrList::iterator()
     */
    ConcurrentPtrListIterator(Guard &guard,
                              ConcurrentPtrList *list,
                              class CPElement *element);

    /** Copy Constructor. */
    ConcurrentPtrListIterator(const ConcurrentPtrListIterator &rhs);

    /** Copy operator. */
    ConcurrentPtrListIterator & operator = (const ConcurrentPtrListIterator &);

    /** Destructor. */
    virtual ~ConcurrentPtrListIterator();

    /** @return Returns the mutex for the Guard passed to other methods. */
    OrderedMutex &getMutex() { return list->getMutex(); }
    
    /** @return Returns the next element or 0 if there is none. */
    void *next(Guard &guard);
private:
    ConcurrentPtrList *list;
    class CPElement   *next_element;
    void              *item;

    void getNext(Guard &guard);
};
 
/** \ingroup Tools
 *  Type-save wrapper for ConcurrentPtrListIterator.
 *  @see ConcurrentPtrListIterator
 */
template<class T> class ConcurrentListIterator
{
public:
    ConcurrentListIterator(ConcurrentPtrListIterator iter) : iter(iter) {}

    ConcurrentListIterator(const ConcurrentListIterator &rhs) : iter(rhs.iter) { }
 
    ConcurrentListIterator & operator = (const ConcurrentListIterator &rhs)
    {
        iter = rhs.iter;
        return *this;
    }
        
    /** @see ConcurrentPtrListIterator::next */
    T *next()
    {
        Guard guard(__FILE__, __LINE__, iter.getMutex());
        return (T *) iter.next(guard);
    }
private:
    ConcurrentPtrListIterator iter;
};

/** \ingroup Tools
 *  Type-save wrapper for ConcurrentPtrList.
 *  @see ConcurrentPtrList
 */
template<class T> class ConcurrentList
    : public Guardable, private ConcurrentPtrList
{
public:
    /** Constructor */
    ConcurrentList()
    {}

    /** @return Returns the list mutex. */
    OrderedMutex &getMutex()
    {
        return ConcurrentPtrList::getMutex();
    }
    
    /** @see ConcurrentPtrList::isEmpty */
    bool isEmpty()
    {
        Guard guard(__FILE__, __LINE__, ConcurrentPtrList::getMutex());
        return ConcurrentPtrList::isEmpty(guard);
    }
    
    /** @see ConcurrentPtrList::size */
    size_t size()
    {
        Guard guard(__FILE__, __LINE__, ConcurrentPtrList::getMutex());
        return ConcurrentPtrList::size(guard);
    }

    /** @see ConcurrentPtrList::add */
    void add(T *item)
    {
        Guard guard(__FILE__, __LINE__, ConcurrentPtrList::getMutex());
        ConcurrentPtrList::add(guard, item);
    }
    
    /** @see ConcurrentPtrList::removeIfFound */
    bool removeIfFound(void *item)
    {
        Guard guard(__FILE__, __LINE__, ConcurrentPtrList::getMutex());
        return ConcurrentPtrList::removeIfFound(guard, item);
    }
    
    /** @see ConcurrentPtrList::remove */
    void remove(T *item)
    {
        Guard guard(__FILE__, __LINE__, ConcurrentPtrList::getMutex());
        ConcurrentPtrList::remove(guard, item);
    }

    /** @see ConcurrentPtrList::iterator */
    ConcurrentListIterator<T> iterator()
    {
        Guard guard(__FILE__, __LINE__, ConcurrentPtrList::getMutex());
        return ConcurrentListIterator<T>(ConcurrentPtrList::iterator(guard));
    }
private:
    PROHIBIT_DEFAULT_COPY(ConcurrentList);
};

#endif 
