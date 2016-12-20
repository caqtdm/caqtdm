// ------------------ -*- c++ -*- -------------------------
// $Id: AVLTree.h,v 1.8 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

#ifndef __AVL_TREE_H
#define __AVL_TREE_H

// Tools
#include <GenericException.h>
#include <AutoFilePtr.h>

// Tree Item:
// Holds the full Item as well as left/right pointers
template<class Item> class AVLItem
{
public:
    AVLItem(const Item &i)
            : item(i), left(0), right(0), balance(0)
    {}

    Item    item;
    AVLItem *left, *right;
    short   balance;
private:
    PROHIBIT_DEFAULT_COPY(AVLItem);
};

/// \ingroup Tools

/// AVL-type balanced tree

/// Sorted binary tree for Items that support
/// the following comparison member function:
/// - int sort_compare(const item &a, const item &b);
///   sort_compare should return "b-a":
///   - <0 if a > b
///   -  0 if a== b
///   - >0 if a < b                         
template<class Item> class AVLTree
{
public:
    AVLTree()
    {   root = 0; }

    ~AVLTree()
    {   clear(); }

    /// Add (copy) Item into the tree.
    /// @exception GenericException
    void add(const Item &item)
    {
        try
        {
            AVLItem<Item> *n = new AVLItem<Item>(item);
            insert(n, &root);
        }
        catch (...)
        {
            throw GenericException(__FILE__, __LINE__, "AVLTree::add failed");
        }
    }

    /// Try to find item in tree.
    
    /// Result: true if found, in which case
    ///         the item argument is set to the one found.
    ///
    bool find(Item &item)
    {
        AVLItem<Item> *n = root;
        int comp;
        while (n)
        {
            comp = sort_compare(item, n->item);
            if (comp == 0)
            {
                item = n->item;
                return true;
            }
            else if (comp >0)
                n = n->left;
            else
                n = n->right;
        }
        return false;
    }

    /// Sorted (inorder) traverse, calling visitor routine on each item.
    void traverse(void (*visit) (const Item &, void *), void *arg=0)
    {   visit_inorder (visit, root, arg); }

    /// Delete all tree entries.
    void clear()
    {
        cleanup(root);
        root = 0;
    }
    
    /// Generates a graphviz 'dot' file.

    /// Requires the Item to support
    /// const char *toString(const Item &).
    ///
    void make_dotfile(const char *name);    

    /// Tests if the tree is AVL-balanced.
    bool selftest()
    {   return check_balance(root); }
    
private:
    PROHIBIT_DEFAULT_COPY(AVLTree);
    AVLItem<Item>   *root;

    void rotate_right(AVLItem<Item> **node)
    {
        if ((*node)->left->balance == 1)
            rotate_left(&(*node)->left);            
        AVLItem<Item> *l = (*node)->left;
        (*node)->left = l->right;
        l->right = *node;
        // Stole the balance recalc from Brad Appleton, http://www.bradapp.net
        if (l->balance == -1)
            (*node)->balance += 2;
        else
            (*node)->balance += 1;

        if ((*node)->balance == 1)
            l->balance += 2;
        else
            l->balance += 1;        
        *node = l;
    }    

    void rotate_left(AVLItem<Item> **node)
    {
        if ((*node)->right->balance == -1)
            rotate_right(&(*node)->right);
        AVLItem<Item> *r = (*node)->right;
        (*node)->right = r->left;
        r->left = *node;

        if (r->balance == 1)
            (*node)->balance -= 2;
        else
            (*node)->balance -= 1;

        if ((*node)->balance == -1)
            r->balance -= 2;
        else
            r->balance -= 1;        
        *node = r;
    }    

    // Result: Did we grow the tree from *node on down?
    bool insert(AVLItem<Item> *new_node, AVLItem<Item> **node)
    {
        if (*node == 0)
        {
            *node = new_node;
            return true;
        }
        int comp = sort_compare(new_node->item, (*node)->item);
        if (comp == 0)
        {   // Don't insert new node, only update data item
            (*node)->item = new_node->item;
            delete new_node;
            return false;
        }
        if (comp > 0)
        {
            if (insert(new_node, & ((*node)->left)))
            {
                --(*node)->balance;
                if ((*node)->balance < -1)
                {
                    rotate_right(node);
                    return false;
                }
                return (*node)->balance < 0;
            }
        }
        else if (comp < 0)
        {
            if (insert(new_node, & ((*node)->right)))
            {
                ++(*node)->balance;
                if ((*node)->balance > 1)
                {
                    rotate_left(node);
                    return false;
                }
                return (*node)->balance > 0;
            }
        }
        return false;
    }

    void visit_inorder(void (*visit) (const Item &, void *),
                       AVLItem<Item> *node, void *arg)
    {
        if (node)
        {
            visit_inorder(visit, node->left, arg);
            visit(node->item, arg);
            visit_inorder(visit, node->right, arg);
        }
    }

    void print_dot_node(FILE *f, AVLItem<Item> *node, int &i)
    {
        ++i;
        int me = i;
        fprintf(f, "n%d [ label=\"%s (%d)\" ];\n",
                me, toString(node->item), (int)node->balance);
        if (node->left)
        {
            fprintf(f, "n%d -> n%d\n", me, i+1);
            print_dot_node(f, node->left, i);
        }
        if (node->right)
        {
            fprintf(f, "n%d -> n%d\n", me, i+1);
            print_dot_node(f, node->right, i);
        }
    }    

    void cleanup(AVLItem<Item> *node)
    {
        if (node)
        {
            cleanup(node->left);
            cleanup(node->right);
            delete node;
        }
    }

    int height(AVLItem<Item> *node)
    {
        if (node == 0)
            return 0;
        int l = height(node->left)+1;
        int r = height(node->right)+1;
        return (l>r) ? l : r;
    }

    // balance should be -1, 0, 1
    // and match the actual sub-tree-height difference 
    bool check_balance(AVLItem<Item> *node)
    {
        if (node == 0)
            return true;
        if (node->balance < -1 ||
            node->balance > 1)
            return false;
        if (node->balance !=
            height(node->right) - height(node->left))
            return false;
        return check_balance(node->left) &&
            check_balance(node->right);
    }
};

template<class Item>
void AVLTree<Item>::make_dotfile(const char *name)
{
    char filename[200];
    int i;
    sprintf(filename, "%s.dot", name);
    try
    {
        AutoFilePtr f(filename, "wt");
        if (!f)
            return;
        fprintf(f, "# dot -Tpng %s.dot -o %s.png && eog %s.png\n",
                name, name, name);
        fprintf(f, "\n");
        fprintf(f, "digraph AVLTree\n");
        fprintf(f, "{\n");
        i=0;
        print_dot_node(f, root, i);
        fprintf(f, "}\n");
    }
    catch (...) {}
}

#endif

