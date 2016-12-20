
#include "AutoPtr.h"
#include "UnitTest.h"

class X
{
public:
    static int deletes;
    X()
    {
        printf("New X.\n");
    }

    ~X()
    {
        ++deletes;
        printf("X deleted.\n");
    }

    int val;
};

int X::deletes = 0;

// AutoPtr tests: Pointer to one instance of something.
TEST_CASE test_autoptr()
{
    X::deletes = 0;
    {
        AutoPtr<int> i(new int);
        *i = 42;
        TEST(*i == 42);
    }
    {
        AutoPtr<X> x(new X);
        x->val = 42;
        (*x).val = 43;

        AutoPtr<X> x2(x);
        TEST(x2->val == 43);
    }
    {
        X x;
        AutoPtr<X> p(&x);
        p.release();
        p.assign(&x);
        p.release();
    }
    {
        int i;
        for (i=0; i<5; ++i)
        {
            AutoPtr<X> x(new X);
            x->val = i;
        }
    }
    TEST_MSG(X::deletes == 7, "All classes were auto-deleted");
    TEST_OK;
}

// AutoArrayPtr tests: Pointer to array of something.
TEST_CASE test_autoarrayptr()
{
    X::deletes = 0;
    {
        AutoArrayPtr<X> copy;
        int i;
        {
            AutoArrayPtr<X> array_of_X(new X[5]);
            for (i=0; i<5; ++i)
            {
                array_of_X[i].val = i;
            }
            for (i=0; i<5; ++i)
            {
                TEST(array_of_X[i].val == i);
            }
            copy = array_of_X;
        }
        for (i=0; i<5; ++i)
        {
            TEST(copy[i].val == i);
        }
    }

    TEST_MSG(X::deletes == 5, "All classes were auto-deleted");
    TEST_OK;
}

// Combined test: Array of Auto-Ptr.
TEST_CASE test_autoarray_autoptr()
{
    X::deletes = 0;
    {
        AutoArrayPtr< AutoPtr<X> > copy;
        int i;
        {
            AutoArrayPtr< AutoPtr<X> > array_of_X_ptr(new AutoPtr<X>[5]);
            for (i=0; i<5; ++i)
            {
                array_of_X_ptr[i] = new X;
                array_of_X_ptr[i]->val = i;
            }
            for (i=0; i<5; ++i)
            {
                TEST(array_of_X_ptr[i]->val == i);
            }
            copy = array_of_X_ptr;
        }
        for (i=0; i<5; ++i)
        {
            TEST(copy[i]->val == i);
        }
    }

    TEST_MSG(X::deletes == 5, "All classes were auto-deleted");
    TEST_OK;
}

// Combined test: Array of ordinary pointers.
// The ordinary pointers are of course not managed.
TEST_CASE test_autoarray_ordptr()
{
    X::deletes = 0;
    {
        AutoArrayPtr< X * > array(new X * [5]);
        int i;
        for (i=0; i<5; ++i)
        {
            array[i] = new X;
            array[i]->val = i;
        }
        for (i=0; i<5; ++i)
        {
           TEST(array[i]->val == i);
           delete array[i];
        }
    }
    TEST_MSG(X::deletes == 5, "All classes were auto-deleted");
    TEST_OK;
}

