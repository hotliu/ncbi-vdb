/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#include <kdb/rowset.h>
#include "rowset-priv.h"
#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/out.h>

#include <string.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KRowSet
 */

struct KRowSet
{
    KRefcount refcount;
    KRowSetTreeNode * root_node;
    DLList leaf_nodes;

    uint64_t number_rows;
    size_t number_leaves;
};

KDB_EXTERN rc_t CC KCreateRowSet ( KRowSet ** self )
{
    rc_t rc;
    KRowSet * rowset;
    
    if ( self == NULL )
        rc = RC ( rcDB, rcRowSet, rcConstructing, rcSelf, rcNull );
    else
    {
        rowset = calloc ( 1, sizeof *rowset );
        if ( rowset == NULL )
            rc = RC ( rcDB, rcRowSet, rcConstructing, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( &rowset->refcount, 1, "KRowSet", "new", "" );
            DLListInit ( &rowset->leaf_nodes );

            *self = rowset;
            return 0;
        }
    }
    
    return rc;
}

/**
 * Allocates and initializes single leaf
 */
static
rc_t KRowSetAllocateLeaf ( KRowSetTreeLeafType type, int64_t leaf_id, KRowSetTreeLeaf ** leaf )
{
    KRowSetTreeLeaf * allocated_leaf;
    size_t size = sizeof allocated_leaf->header;

    assert ( leaf != NULL );

    switch ( type )
    {
    case LeafType_Bitmap:
        size += sizeof allocated_leaf->data.bitmap;
        break;
    case LeafType_ArrayRanges:
        size += sizeof allocated_leaf->data.array_ranges;
        break;
    default:
        assert ( false );
    }

    allocated_leaf = calloc ( 1, size );
    if ( allocated_leaf == NULL )
        return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

#if CHECK_NODE_MARKS
    allocated_leaf->header.leaf_mark = LEAF_MARK;
#endif
    allocated_leaf->header.type = type;
    allocated_leaf->header.leaf_id = leaf_id;

    *leaf = allocated_leaf;
    return 0;
}

/* Whack
 */

/*
 * When data is NULL, it means that leaf is freeing during freeing the whole list
 * and DLListUnlink does not need to be called. Otherwise, we are removing a single leaf
 */
static
void KRowSetLeafWhack ( DLNode * node, void * data )
{
    KRowSetTreeLeaf * leaf = (KRowSetTreeLeaf *) node;
#if CHECK_NODE_MARKS
    assert ( leaf->header.leaf_mark == LEAF_MARK );
#endif

    if ( data != NULL )
    {
        DLList * leaf_nodes = data;
        DLListUnlink ( leaf_nodes, node );
    }

    free ( leaf );
}

/**
 * Recursively deallocates all trie nodes and leaves when "free_leaves" is true.
 *  Otherwise only deallocates nodes.
 */
static
void KRowSetNodeWhack ( KRowSet * self, KRowSetTreeNode * node, int depth, bool free_leaves )
{
    int i;

    assert ( self != NULL );
    assert ( depth < LEAF_DEPTH );

#if CHECK_NODE_MARKS
    assert ( node->node_mark == NODE_MARK );
#endif
    assert ( depth < LEAF_DEPTH );

    for ( i = 0; i < 256; ++i )
    {
        assert ( (i & 0xFF) == i );

        if ( node->children[i] )
        {
            int new_depth = depth + node->transitions[i].size + 1;
            assert ( new_depth <= LEAF_DEPTH );

            if ( new_depth < LEAF_DEPTH )
                KRowSetNodeWhack ( self, node->children[i], new_depth, free_leaves );
            else if ( free_leaves )
                KRowSetLeafWhack ( &((KRowSetTreeLeaf*) node->children[i])->header.dad, &self->leaf_nodes );
        }
    }

    free ( node );
}

/**
 * Whack
 *   Frees all leaves first and then walks through the trie
 *   and frees all remaining nodes
 */
static
rc_t KRowSetWhack ( KRowSet * self )
{
    assert ( self != NULL );

    if ( self->root_node )
    {
        KRowSetNodeWhack ( self, self->root_node, 0, false );
        DLListWhack ( &self->leaf_nodes, KRowSetLeafWhack, NULL );
    }
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  ignores NULL references
 */
KDB_EXTERN rc_t CC KRowSetAddRef ( const KRowSet *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KRowSet" ) )
    {
        case krefOkay:
            break;
        default:
            return RC ( rcDB, rcRowSet, rcAttaching, rcConstraint, rcViolated );
    }
    return 0;
}

KDB_EXTERN rc_t CC KRowSetRelease ( const KRowSet *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KRowSet" ) )
    {
        case krefOkay:
            break;
        case krefWhack:
            return KRowSetWhack ( ( KRowSet* ) self );
        default:
            return RC ( rcDB, rcRowSet, rcReleasing, rcConstraint, rcViolated );
    }
    return 0;
}

/**
 * Get byte of "leaf_id" which represents node's child at "depth"
 */
static
inline uint8_t KRowSetGetLeafIdByte ( int64_t leaf_id, int depth )
{
    return leaf_id >> (LEAF_DEPTH - depth - 1) * 8 & 0xFF;
}

/**
 * Searches a nearest neighbor of just inserted "leaf",
 *  by walking through nodes_stack from the most deep node to the root one.
 *
 * Is used to inserter newly created "leaf" to a linked list.
 */
static
void KRowSetFindNearestLeaf ( KRowSet * self, KRowSetTreeNode * nodes_stack[], int nodes_depth[], KRowSetTreeLeaf * leaf, KRowSetTreeLeaf ** result, bool * result_left )
{
    assert ( self != NULL );
    assert ( nodes_stack != NULL );
    assert ( leaf != NULL );
    assert ( result != NULL );
    assert ( result_left != NULL );

    *result = NULL;

    if ( self->number_leaves == 0 )
        return;

    if ( self->number_leaves == 1 )
    {
        *result = (KRowSetTreeLeaf *) DLListHead ( &self->leaf_nodes );
        *result_left = (*result)->header.leaf_id < leaf->header.leaf_id;
        return;
    }

    {
        int i;
        int64_t leaf_id = leaf->header.leaf_id;
        KRowSetTreeLeaf * nearest_leaf = NULL;
        bool nearest_leaf_left;
        for ( i = LEAF_DEPTH - 1; i >= 0; --i )
        {
            int depth;
            int j;
            int j_max;
            uint8_t bt;
            KRowSetTreeNode * current_node = nodes_stack[i];
            void * nearest_subtree = NULL;
            bool nearest_subtre_left;
            int nearest_subtree_depth;

            // determine nodes stack depth, since some of the nodes may have been compressed
            if ( current_node == NULL )
                continue;

            depth = nodes_depth[i];
            bt = KRowSetGetLeafIdByte ( leaf_id, depth );

            assert ( current_node->children[bt] != NULL );

            if ( 255 - bt > bt )
                j_max = 255 - bt;
            else
                j_max = bt;

            for (j = 1; j <= j_max; ++j)
            {
                if ( bt + j <= 255 && current_node->children[bt + j] != NULL )
                {
                    nearest_subtre_left = false;
                    nearest_subtree = current_node->children[bt + j];
                    nearest_subtree_depth = depth + 1 + current_node->transitions[bt + j].size;
                    break;
                }

                if ( bt - j >= 0 && current_node->children[bt - j] != NULL )
                {
                    nearest_subtre_left = true;
                    nearest_subtree = current_node->children[bt - j];
                    nearest_subtree_depth = depth + 1 + current_node->transitions[bt - j].size;
                    break;
                }
            }

            if ( nearest_subtree != NULL )
            {
                for ( j = nearest_subtree_depth; j < LEAF_DEPTH; ++j )
                {
                    int search_i;
                    int search_start = nearest_subtre_left ? 255 : 0;
                    int search_stop = nearest_subtre_left ? 0 : 255;
                    int search_step = nearest_subtre_left ? -1 : 1;
                    void * nearest_subtree_next = NULL;
                    KRowSetTreeNodeTransition * nearest_subtree_next_tr;

#if CHECK_NODE_MARKS
                    assert ( ((KRowSetTreeNode *) nearest_subtree)->node_mark == NODE_MARK );
#endif

                    for ( search_i = search_start; search_step > 0 ? search_i <= search_stop : search_i >= search_stop; search_i += search_step )
                    {
                        if ( ((KRowSetTreeNode *) nearest_subtree)->children[search_i] != NULL )
                        {
                            nearest_subtree_next = ((KRowSetTreeNode *) nearest_subtree)->children[search_i];
                            nearest_subtree_next_tr = &((KRowSetTreeNode *) nearest_subtree)->transitions[search_i];
                            break;
                        }
                    }

                    assert ( nearest_subtree_next != NULL );
                    nearest_subtree = nearest_subtree_next;
                    j += nearest_subtree_next_tr->size;
                }

                assert ( j == LEAF_DEPTH );

                nearest_leaf = nearest_subtree;
                nearest_leaf_left = nearest_subtre_left;

                break;
            }
        }

        // empty tree is handled by previous "if"s
        assert ( nearest_leaf != NULL );

#if CHECK_NODE_MARKS
        assert ( ((KRowSetTreeLeaf *) nearest_leaf)->header.leaf_mark == LEAF_MARK );
#endif
        *result = nearest_leaf;
        *result_left = nearest_leaf_left;
    }

    return;
}

/**
 * Checks if node's child pointed by "node_depth"'s "leaf_id" byte matches "leaf_id".
 *  It is possible that transition uses more than one byte of "leaf_id".
 *  In such case, we have to make sure that those bytes match.
 */
static
bool KRowSetNodeIsTransitionMatches ( int64_t leaf_id, int node_depth, const KRowSetTreeNodeTransition * tr, int * size_matched )
{
    int matched;

    assert ( node_depth + tr->size < LEAF_DEPTH );

    for ( matched = 0; matched < tr->size; ++matched )
    {
        if ( KRowSetGetLeafIdByte ( leaf_id, matched + node_depth + 1 ) != tr->data[matched] )
            break;
    }

    if ( size_matched != NULL )
        *size_matched = matched;

    return matched == tr->size;
}

/**
 * Assigns a new child to a "node".
 *  It will note create any extra node's even if child_depth - node_depth > 1
 *  (because of radix trie compression)
 */
static
void KRowSetNodeSetChild ( KRowSetTreeNode * node, void * child, int64_t leaf_id, int node_depth, int child_depth )
{
    uint8_t bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );
    int i;

    node->children[bt] = child;
    node->transitions[bt].size = child_depth - 1 - node_depth;

    for ( i = node_depth; i < child_depth - 1; ++i )
    {
        node->transitions[bt].data[i - node_depth] = KRowSetGetLeafIdByte ( leaf_id, i + 1 );
    }
}

/**
 * Splits the node, by inserting "allocated_interm_node" between "node" and it's child
 *  Corrects transitions after insertion.
 *
 *  NB - To ease error handling and deallocation of resources in case of errors,
 *       "allocated_interm_node" has to be allocated before calling this function.
 */
static
void KRowSetSplitNode ( KRowSetTreeNode * node, int node_depth, KRowSetTreeNode * allocated_interm_node, int interm_node_depth, int64_t leaf_id )
{
    uint8_t bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );
    uint8_t interm_bt;
    void * child = node->children[bt];
    int first_tr_size = interm_node_depth - node_depth - 1;

#if CHECK_NODE_MARKS
    allocated_interm_node->node_mark = NODE_MARK;
#endif

    assert ( allocated_interm_node != NULL );
    assert ( node->children[bt] != NULL );

    interm_bt = node->transitions[bt].data[first_tr_size];
    allocated_interm_node->transitions[interm_bt].size = node->transitions[bt].size - first_tr_size - 1;
    for ( int i = 0; i < allocated_interm_node->transitions[interm_bt].size; ++i )
    {
        allocated_interm_node->transitions[interm_bt].data[i] = node->transitions[bt].data[first_tr_size + i + 1];
    }
    allocated_interm_node->children[interm_bt] = child;

    KRowSetNodeSetChild ( node, allocated_interm_node, leaf_id, node_depth, interm_node_depth );
}

/**
 * Sets "allocated_leaf" members and inserts it as a child to a previously found "node" that fits leaf_id.
 *  Also inserts "allocated_leaf" to leaves linked list.
 *
 *  NB - To ease error handling and deallocation of resources in case of errors,
 *       "allocated_leaf" has to be allocated before calling this function.
 */
static
void KRowSetInsertLeaf ( KRowSet * self, int64_t leaf_id, KRowSetTreeNode * node, int node_depth, KRowSetTreeNode * nodes_stack[], int nodes_depth[], KRowSetTreeLeaf * allocated_leaf )
{
    KRowSetTreeLeaf * nearest_leaf;
    bool nearest_leaf_left;
    uint8_t bt;
    bt = KRowSetGetLeafIdByte ( leaf_id, node_depth );

    assert ( allocated_leaf != NULL );
    assert ( node->children[bt] == NULL );

    KRowSetNodeSetChild ( node, allocated_leaf, leaf_id, node_depth, LEAF_DEPTH );
    node->children[bt] = allocated_leaf;

    KRowSetFindNearestLeaf ( self, nodes_stack, nodes_depth, allocated_leaf, &nearest_leaf, &nearest_leaf_left );
    if ( nearest_leaf == NULL )
        DLListPushTail ( &self->leaf_nodes, &allocated_leaf->header.dad );
    else if (nearest_leaf_left)
        DLListInsertNodeAfter ( &self->leaf_nodes, &nearest_leaf->header.dad, &allocated_leaf->header.dad );
    else
        DLListInsertNodeBefore ( &self->leaf_nodes, &nearest_leaf->header.dad, &allocated_leaf->header.dad );

    ++self->number_leaves;
    assert ( KRowSetNodeIsTransitionMatches ( leaf_id, node_depth, &node->transitions[bt], NULL ) );
}

/**
 * Searches for a leaf by "row_id".
 *  Returns rcNotFound when leaf is not in the tree and "insert_when_needed" is false,
 *  otherwise inserts a new leaf and returns it in "leaf_found".
 *
 *  NB - This function preallocates all required resources
 * 	     and only after that changes the RowSet data structure.
 *
 * 	     Using this approach we can simply return from the function,
 * 	     since all the resources are being allocated once.
 */
static
rc_t KRowSetGetLeaf ( KRowSet * self, int64_t row_id, bool insert_when_needed, KRowSetTreeLeaf ** leaf_found, KRowSetTreeNode ** leaf_parent, int * leaf_parent_depth )
{
    rc_t rc;
    int64_t leaf_id = row_id >> 16;
    KRowSetTreeNode * node;
    KRowSetTreeLeaf * new_leaf;
    uint8_t bt;
    int depth = 0;
    int nodes_stack_size = 0;
    KRowSetTreeNode * nodes_stack[LEAF_DEPTH] = { NULL }; // initialize array with NULLs
    int nodes_depth[LEAF_DEPTH];

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcSelf, rcNull );

    if ( leaf_found == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

    if ( leaf_parent != NULL && leaf_parent_depth == NULL )
        return RC ( rcDB, rcRowSet, rcSelecting, rcParam, rcNull );

    // empty tree
    if ( self->root_node == NULL )
    {
        KRowSetTreeNode * root;
        if ( !insert_when_needed )
            return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

        root = calloc ( 1, sizeof ( KRowSetTreeNode ) );
        if (root == NULL)
            return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

        // pre-allocate leaf here
        rc = KRowSetAllocateLeaf ( LeafType_ArrayRanges, leaf_id, &new_leaf );
        if ( rc != 0 )
        {
            free ( root );
            return rc;
        }

#if CHECK_NODE_MARKS
        root->node_mark = NODE_MARK;
#endif

        self->root_node = root;

        node = self->root_node;
        nodes_stack[nodes_stack_size] = node;
        nodes_depth[nodes_stack_size++] = depth;

        KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, new_leaf );
        *leaf_found = new_leaf;
        if ( leaf_parent != NULL )
        {
            *leaf_parent = node;
            *leaf_parent_depth = nodes_depth[nodes_stack_size - 1];
        }
        return 0;
    }

    node = self->root_node;
    nodes_stack[nodes_stack_size] = node;
    nodes_depth[nodes_stack_size++] = depth;
#if CHECK_NODE_MARKS
        assert ( node->node_mark == NODE_MARK );
#endif

    for ( ; depth < LEAF_DEPTH; )
    {
        int tr_size_matched;
        bt = KRowSetGetLeafIdByte ( leaf_id, depth );
        // no child at a given transition, let's insert leaf here
        if ( node->children[bt] == NULL )
        {
            if ( !insert_when_needed )
                return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

            // pre-allocate leaf here
            rc = KRowSetAllocateLeaf ( LeafType_ArrayRanges, leaf_id, &new_leaf );
            if ( rc != 0 )
                return rc;

            KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, new_leaf );
            *leaf_found = new_leaf;
            if ( leaf_parent != NULL )
            {
                *leaf_parent = node;
                *leaf_parent_depth = nodes_depth[nodes_stack_size - 1];
            }
            return 0;
        }

        // transition does not match, split the node
        if ( !KRowSetNodeIsTransitionMatches ( leaf_id, depth, &node->transitions[bt], &tr_size_matched ) )
        {
            KRowSetTreeNode * interm_node;
            int interm_node_depth = depth + 1 + tr_size_matched;

            if ( !insert_when_needed )
                return RC ( rcDB, rcRowSet, rcSelecting, rcItem, rcNotFound );

            // pre-allocate node and leaf here
            interm_node = calloc ( 1, sizeof ( KRowSetTreeNode ) );
            if ( interm_node == NULL )
                return RC ( rcDB, rcRowSet, rcInserting, rcMemory, rcExhausted );

            rc = KRowSetAllocateLeaf ( LeafType_ArrayRanges, leaf_id, &new_leaf );
            if ( rc != 0 )
            {
                free ( interm_node );
                return rc;
            }

            KRowSetSplitNode ( node, depth, interm_node, interm_node_depth, leaf_id );

            node = interm_node;
            depth = interm_node_depth;
#if CHECK_NODE_MARKS
            assert ( node->node_mark == NODE_MARK );
#endif
            assert ( depth < sizeof nodes_stack / sizeof nodes_stack[0] );
            nodes_stack[nodes_stack_size] = node;
            nodes_depth[nodes_stack_size++] = depth;

            KRowSetInsertLeaf ( self, leaf_id, node, depth, nodes_stack, nodes_depth, new_leaf );
            *leaf_found = new_leaf;
            if ( leaf_parent != NULL )
            {
                *leaf_parent = node;
                *leaf_parent_depth = nodes_depth[nodes_stack_size - 1];
            }
            return 0;

        }

        // check if transition leads to a leaf or follow the transition
        if ( depth + 1 + tr_size_matched == LEAF_DEPTH )
        {
            *leaf_found = node->children[bt];
            if ( leaf_parent != NULL )
            {
                *leaf_parent = node;
                *leaf_parent_depth = nodes_depth[nodes_stack_size - 1];
            }
            return 0;
        }

        node = node->children[bt];
        depth += 1 + tr_size_matched;
#if CHECK_NODE_MARKS
        assert ( node->node_mark == NODE_MARK );
#endif
        assert ( depth < sizeof nodes_stack / sizeof nodes_stack[0] );
        nodes_stack[nodes_stack_size] = node;
        nodes_depth[nodes_stack_size++] = depth;
    }

    // "for" loop iterates up to a maximum possible depth,
    // so it must come to a leaf
    assert ( false );
    return 0;
}

/**
 * Transforms leaf from LeafType_ArrayRanges to LeafType_Bitmap
 */
static
rc_t KRowSetTreeLeafTransform ( KRowSet * self, KRowSetTreeLeaf ** p_leaf, KRowSetTreeNode * parent_node, int parent_depth )
{
    rc_t rc;
    int i;
    int j;
    int len;
    uint8_t parent_bt;
    uint16_t leaf_bt;
    KRowSetTreeLeaf * leaf;
    KRowSetTreeLeaf * new_leaf;

    assert ( p_leaf != NULL );
    leaf = *p_leaf;
    assert ( leaf != NULL );

    parent_bt = KRowSetGetLeafIdByte ( leaf->header.leaf_id, parent_depth );
    assert ( KRowSetNodeIsTransitionMatches ( leaf->header.leaf_id, parent_depth, &parent_node->transitions[parent_bt], NULL ) );

    rc = KRowSetAllocateLeaf( LeafType_Bitmap, leaf->header.leaf_id, &new_leaf );
    if ( rc != 0 )
        return rc;

    // copy rows to a new leaf
    len = leaf->data.array_ranges.len;
    for ( i = 0; i < len; ++i )
    {
        struct KRowSetTreeLeafArrayRange * range = &leaf->data.array_ranges.ranges[i];
        for ( j = range->start; j <= range->end; ++j )
        {
            leaf_bt = j;
            new_leaf->data.bitmap[leaf_bt >> 3] |= 1 << (leaf_bt & 7);
        }
    }

    DLListInsertNodeBefore ( &self->leaf_nodes, &leaf->header.dad, &new_leaf->header.dad );
    KRowSetLeafWhack ( &leaf->header.dad, &self->leaf_nodes );
    leaf = new_leaf;
    parent_node->children[parent_bt] = leaf;
    *p_leaf = leaf;

    return 0;
}

/**
 * NB- it may reallocate leaf in case when leaf is transformed,
 *     so leaf pointer may change as a result of this function
 */
static
rc_t KRowSetTreeLeafInsertRow ( KRowSet * self, KRowSetTreeLeaf ** p_leaf, uint16_t leaf_row_start, uint16_t leaf_row_end, KRowSetTreeNode * parent_node, int parent_depth )
{
    KRowSetTreeLeaf * leaf = *p_leaf;
    int i;
    uint16_t leaf_row;

    assert ( leaf_row_end >= leaf_row_start );

    switch ( leaf->header.type )
    {
    case LeafType_Bitmap:
        // TODO: optimize by assigning several bytes at a time when possible
        for ( i = leaf_row_start; i <= leaf_row_end; ++i )
        {
            leaf_row = i;
            if ( ( leaf->data.bitmap[leaf_row >> 3] & (1 << (leaf_row & 7)) ) != 0 )
                return RC ( rcDB, rcRowSet, rcInserting, rcId, rcDuplicate );

            leaf->data.bitmap[leaf_row >> 3] |= 1 << (leaf_row & 7);
        }
        break;
    case LeafType_ArrayRanges:
    {
        int j;
        int insert_i;
        int len = leaf->data.array_ranges.len;
        int max_len = 8;
        for ( i = 0; i < len; ++i )
        {
            struct KRowSetTreeLeafArrayRange * range = &leaf->data.array_ranges.ranges[i];

            if ( leaf_row_end >= range->start && leaf_row_start <= range->end )
                return RC ( rcDB, rcRowSet, rcInserting, rcId, rcDuplicate );

            // we can just increase existing range to the left
            if ( range->start > 0 && leaf_row_end == range->start - 1 )
            {
                range->start = leaf_row_start;
                return 0;
            }
            if ( leaf_row_start > 0 && leaf_row_start - 1 == range->end )
            {
                if ( i + 1 < len)
                {
                    if ( (range + 1)->start <= leaf_row_end )
                        return RC ( rcDB, rcRowSet, rcInserting, rcId, rcDuplicate );
                    else if ( (range + 1)->start - 1 == leaf_row_end )
                    {
                        // just merge two ranges
                        range->end = (range+1)->end;

                        --len;
                        --leaf->data.array_ranges.len;
                        for ( j = i + 1; j < len; --j )
                        {
                            leaf->data.array_ranges.ranges[i] = leaf->data.array_ranges.ranges[i + 1];
                        }

                        return 0;
                    }
                }

                range->end = leaf_row_end;
                return 0;
            }

            // not found - insert new range
            if ( leaf_row_end < range->start )
            {
                break;
            }
        }

        // came here because we need to insert a new range
        // (or transform a leaf to a bitmap)
        insert_i = i;

        if ( len == max_len )
        {
            rc_t rc;
            rc = KRowSetTreeLeafTransform ( self, p_leaf, parent_node, parent_depth );
            if ( rc != 0 )
                return rc;

            return KRowSetTreeLeafInsertRow ( self, p_leaf, leaf_row_start, leaf_row_end, parent_node, parent_depth );
        }

        ++len;
        ++leaf->data.array_ranges.len;
        for ( i = len - 1; i > insert_i; --i )
        {
            leaf->data.array_ranges.ranges[i] = leaf->data.array_ranges.ranges[i - 1];
        }

        leaf->data.array_ranges.ranges[insert_i].start = leaf_row_start;
        leaf->data.array_ranges.ranges[insert_i].end = leaf_row_end;

        break;
    }
    default:
        assert ( false );
    }

    return 0;
}

KDB_EXTERN rc_t CC KRowSetInsertRowRange ( KRowSet * self, int64_t row_id_start, uint64_t row_id_count )
{
    rc_t rc = 0;
    KRowSetTreeLeaf * leaf;
    KRowSetTreeNode * leaf_parent;
    int leaf_parent_depth;
    int64_t current_range_start = row_id_start;
    uint64_t row_id_remaining = row_id_count;
    uint16_t leaf_row_start;
    uint16_t leaf_row_end;
    int leaf_row_count;

    if ( row_id_start < 0 || row_id_count <= 0 )
        return RC ( rcDB, rcRowSet, rcInserting, rcParam, rcInvalid );

    if ( row_id_start + row_id_count < row_id_start )
        return RC ( rcDB, rcRowSet, rcInserting, rcParam, rcOutofrange );


    while ( rc == 0 && row_id_remaining > 0 )
    {
        leaf_row_start = current_range_start & ROW_LEAF_MAX;
        leaf_row_count = ROW_LEAF_MAX - leaf_row_start + 1;
        if ( leaf_row_count > row_id_remaining )
            leaf_row_count = row_id_remaining;

        assert ( leaf_row_start + leaf_row_count - 1 <= ROW_LEAF_MAX );
        leaf_row_end = leaf_row_start + leaf_row_count - 1;

        rc = KRowSetGetLeaf ( self, current_range_start, true, &leaf, &leaf_parent, &leaf_parent_depth );
        if ( rc == 0 )
        {
            rc = KRowSetTreeLeafInsertRow ( self, &leaf, leaf_row_start, leaf_row_end, leaf_parent, leaf_parent_depth );
        }

        current_range_start += leaf_row_count;
        row_id_remaining -= leaf_row_count;
    }

    self->number_rows += row_id_count - row_id_remaining;

    return rc;
}

KDB_EXTERN rc_t CC KRowSetGetNumRows ( const KRowSet * self, uint64_t * num_rows )
{
    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcSelf, rcNull );

    if ( num_rows == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcParam, rcNull );

    *num_rows = self->number_rows;
    return 0;
}

KDB_EXTERN rc_t CC KRowSetWalkRows ( const KRowSet * self, bool reverse,
        void ( CC * cb ) ( int64_t row_id, void * data ), void * data )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcRowSet, rcAccessing, rcSelf, rcNull );

    KRowSetIterator * it;
    rc = KRowSetCreateIterator ( self, reverse, &it );
    if ( rc != 0 )
        return rc;

    while ( KRowSetIteratorNext ( it ) )
    {
        int64_t row_id = KRowSetIteratorRowId ( it );
        cb ( row_id, data );
    }

    KRowSetIteratorRelease ( it );

    return 0;
}

const KRowSetTreeLeaf * KRowSetTreeGetFirstLeaf ( const KRowSet * self )
{
    assert ( self != NULL );
    return (const KRowSetTreeLeaf * )DLListHead ( &self->leaf_nodes );
}

const KRowSetTreeLeaf * KRowSetTreeGetLastLeaf ( const KRowSet * self )
{
    assert ( self != NULL );
    return (const KRowSetTreeLeaf * )DLListTail ( &self->leaf_nodes );
}
