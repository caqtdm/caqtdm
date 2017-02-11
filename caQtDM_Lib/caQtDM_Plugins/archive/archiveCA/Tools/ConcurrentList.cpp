// Tools
#include "MsgLogger.h"
#include "GenericException.h"
#include "ConcurrentList.h"
 
class CPElement
{
public:
    CPElement(void *item) : item(item), next(0) {}

    void *getItem() const                       { return item; }
    
    void setItem(void *i)                       { item = i; }

    CPElement *getNext() const                  { return next; }
    
    void setNext(CPElement *e)                  { next = e; }
    
private:
    void       *item;
    CPElement  *next;
};

// ConcurrentPtrList Implementation:
// A simple linked list,
// where items with value '0' are indicators for deleted elements.
// They are skipped by any (ongoing) iterator,
// and removed later.
ConcurrentPtrList::ConcurrentPtrList()
    : mutex("ConcurrentPtrList", OrderedMutex::ConcurrentList),
      list(0)
{}

ConcurrentPtrList::~ConcurrentPtrList()
{
    CPElement *e = list;
    while (e)
    {
        CPElement *del = e;
        e = e->getNext();
        delete del;
    }
}

bool ConcurrentPtrList::isEmpty(Guard &guard)
{
    guard.check(__FILE__, __LINE__, mutex);
    CPElement *e = list;
    while (e)
    {
        if (e->getItem())
            return false; // There is at least one entry.
        e = e->getNext();
    }    
    return true; // nothing found, must be empty.
}

size_t ConcurrentPtrList::size(Guard &guard)
{
    guard.check(__FILE__, __LINE__, mutex);
    size_t count = 0;
    CPElement *e = list;
    while (e)
    {
        if (e->getItem())
            ++count;
        e = e->getNext();
    }    
    return count;
}

void ConcurrentPtrList::add(Guard &guard, void *item)
{
    guard.check(__FILE__, __LINE__, mutex);
    if (list == 0)
    {   // list was empty
        list = new CPElement(item);
        return;
    }
    // Loop to last element
    CPElement *e = list;
    while (1)
    {
        if (e->getItem() == 0)
        {   // Re-use list item that had been removed.
            e->setItem(item);
            return;
        }
        if (e->getNext() == 0)
        {   // Reached end of list, append new element.
            e->setNext(new CPElement(item));
            return;
        }
        e = e->getNext();
    }
}

bool ConcurrentPtrList::removeIfFound(Guard &guard, void *item)
{
    guard.check(__FILE__, __LINE__, mutex);    
    CPElement *e = list;
    while (e)
    {
        if (e->getItem() == item)
        {
            e->setItem(0);
            return true;
        }
        e = e->getNext();
    }
    return false;    
}

void ConcurrentPtrList::remove(Guard &guard, void *item)
{
    if (removeIfFound(guard, item))
        return;
    throw GenericException(__FILE__, __LINE__, "Unknown item");    
}

ConcurrentPtrListIterator ConcurrentPtrList::iterator(Guard &guard)
{
    guard.check(__FILE__, __LINE__, mutex);    
    return ConcurrentPtrListIterator(guard, this, list);
}

// ConcurrentPtrListIterator Idea:
// item is the item that next() will return.
// It was copied from the 'current' list element
// in case somebody modifies the list between
// calls to hasNext() and next().
// The next_element already points to the
// following list element.
ConcurrentPtrListIterator::ConcurrentPtrListIterator(Guard &guard,
                                                     ConcurrentPtrList *list,
                                                     CPElement *element)
    : list(list), next_element(element), item(0)
{
    getNext(guard);
}

ConcurrentPtrListIterator::ConcurrentPtrListIterator(
    const ConcurrentPtrListIterator &rhs)
    : list(rhs.list), next_element(rhs.next_element), item(rhs.item)
{
}

ConcurrentPtrListIterator & ConcurrentPtrListIterator::operator =
   (const ConcurrentPtrListIterator &rhs)
{
    list = rhs.list;
    next_element = rhs.next_element;
    item = rhs.item;
    return *this;
}

ConcurrentPtrListIterator::~ConcurrentPtrListIterator()
{
    // Could try to use this->list to keep track
    // of number of active iterators,
    // but then also need to implement proper copy constructor etc.
    // For now, that's not done.
}

void ConcurrentPtrListIterator::getNext(Guard &guard)
{
    guard.check(__FILE__, __LINE__, getMutex());    
    if (next_element == 0)
    {
        item = 0;
        return;
    }
    do
    {
        item = next_element->getItem();
        next_element = next_element->getNext();
    }
    while (item == 0  &&  next_element);
}

void *ConcurrentPtrListIterator::next(Guard &guard)
{
    guard.check(__FILE__, __LINE__, getMutex());    
    void *result = item;
    getNext(guard);
    return result;
}
