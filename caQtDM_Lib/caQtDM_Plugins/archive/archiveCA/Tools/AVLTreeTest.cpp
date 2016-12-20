
#include "AVLTree.h"
#include "UnitTest.h"
#include <stdlib.h>

static int sort_compare(const int &a, const int &b)
{   return b-a; }

static int avl_last_number;
static bool tree_is_ordered;

static void avl_order_test(const int &i, void *)
{
    if (i < avl_last_number)
        tree_is_ordered = false;
    avl_last_number = i;
}

static const char *toString(const int &i)
{
    static char txt[10];
    sprintf(txt, "%d", i);
    return txt;
}

TEST_CASE avl_test()
{
    AVLTree<int> tree;

    tree.add(RAND_MAX/2);
    int i, num;

    for (i=0; i<100; ++i)
    {
        num = rand();
        if (tree.find(i))
            continue;
        tree.add(num);
        if (!tree.find(num))
        {
            FAIL("Tree cannot find a number just entered");
        }
        if (!tree.selftest())
        {
            FAIL("Tree fails selftest");
        }
    }
    tree.make_dotfile("avl");
    avl_last_number = -10000;
    tree_is_ordered = true;
    tree.traverse(avl_order_test, 0);
    TEST(tree_is_ordered);
    tree.make_dotfile("avl");
    TEST("Check avl.dot if you care");
    TEST_OK;
}

