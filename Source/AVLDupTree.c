/******************************************************************************
 * $Header: /boot/home/agmsmith/Programming/AVLTree/RCS/AVLDupTree.c,v 1.11 2001/03/28 22:17:44 agmsmith Exp $
 *
 * This set of C subroutines is useful for indexing a set of key/value pairs,
 * using the key to find a matching value.  As an enhancement, multiple values
 * for the same key are supported efficiently.
 *
 * Copyright © 2001 by Alexander G. M. Smith.  Look a bit furthur down in
 * this file for licensing details (GNU Lesser General Public License).
 *
 * It's implemented with a balanced binary search tree, which lets you store
 * and find things in optimal O(log n) time.  A binary search tree (BST) has
 * the property that all nodes in the left sub-tree have keys less than the
 * given node, and all nodes in the right sub-tree have keys greater than the
 * given node.  Searching is done by just traversing down and choosing left or
 * right depending if the key you are looking for is less than or greater than
 * the current node's key.  The balancing is done by using the AVL algorithm,
 * which enforces the condition that left and right sub-trees differ in height
 * by at most one.  If an insert or delete operation (done much like a search -
 * except that you add the new node where the search said it should be) breaks
 * that rule, the subtrees are fixed up with a "BST rotation" operation, with
 * recursive fixups applied from the point of change all the way to the root if
 * necessary.  Since finding a key/value pair and the number of fixups for
 * insert/delete both depend on the height of the tree (which is O(log n)),
 * and a single fixup or search comparision takes constant time, the overall
 * speed of the basic find/insert/delete operations is O(log n).
 *
 * The AVL tree algorithms were invented by G. M. Adel'son-Velskii and
 * Y. M. Landis, who wrote about it first in 1962 in Soviet Math Dockl., titled
 * "An Algorithm for the Organization of Information".  I made use of a nice
 * article written by Timothy Rolfe on AVL trees in the December 2000 issue of
 * Dr. Dobb's Journal, pages 149-152.  I also dug up an alternate description
 * of AVL trees using "left heavy" and "right heavy" concepts in an old
 * textbook called "An Introduction to Data Structures with Applications", by
 * Jean-Paul Tremblay and P. G. Sorenson, 1976, McGraw-Hill computer science
 * series, pages 489-500.  They have a nice reference on page 519 to
 * Adel'son-Vel'skii, G. M. and E. M. Landis, same title as before, in
 * "Dokl. Akad. Nauk SSSR, Mathemat., vol. 146, no. 2, pp. 263-266, 1962."
 *
 * It's called AVLDupTree since it has been altered by Alexander G. M. Smith
 * to handle duplicate keys efficiently.  That was originally done by making a
 * subtree under the node with the common key to store the values.  But that
 * turned out to be unnecessarily complicated, just changing the key comparison
 * function solves the same problem - it now looks at the values if the keys
 * were equal.
 *
 * Since this is designed for use as the index for a BeOS inspired file system,
 * the keys can be the typical BeOS keys: string, int32, int64, float, double.
 *
 * It is also designed to work in a multitasking environment - allowing access
 * by multiple readers or a single writer (no, you can't have your reader's
 * callback function do a write operation - the write will deadlock waiting
 * for the number of readers to fall to zero).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Send comments and wishes to agmsmith@achilles.net (or do a web search for
 * "agmsmith" to find a more recent e-mail address and web pages).
 *
 * $Log: AVLDupTree.c,v $
 * Revision 1.11  2001/03/28 22:17:44  agmsmith
 * Prepared for 1.0 release.
 *
 * Revision 1.10  2001/03/23 02:41:30  agmsmith
 * Oops...
 *
 * Revision 1.9  2001/03/22 22:58:13  agmsmith
 * Added double bounded iteration function, compiles but not tested yet.
 *
 * Revision 1.8  2001/03/19 01:43:51  agmsmith
 * Pass largely unchanging arguments using a per-thread structure,
 * rather than having separate arguments in each recursive call.
 *
 * Revision 1.7  2001/03/18 17:53:23  agmsmith
 * Added semaphore for multitasking safety.
 *
 * Revision 1.6  2001/03/17 21:09:01  agmsmith
 * Printed it out, read it over, corrected a few mistakes.
 *
 * Revision 1.5  2001/03/16 22:31:30  agmsmith
 * Added AVL balancing, and implemented delete.
 *
 * Revision 1.4  2001/03/15 18:02:13  agmsmith
 * Removed duplicates tree, partially working now without it.
 *
 * Revision 1.3  2001/03/12 22:38:56  agmsmith
 * Just before removing duplicates subtree, but it does work a bit.
 *
 * Revision 1.2  2001/03/10 17:59:19  agmsmith
 * Under construction.
 *
 * Revision 1.1  2001/03/08 20:36:31  agmsmith
 * Initial revision
 */

#include <OS.h>
#include <TypeConstants.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AVLDupTree.h"


/* This structure is a node in the AVL tree.  The keys in the sub-tree at
smallerChildPntr are all less than the node's key.  Keys in the sub-tree
rooted at largerChildPntr are all larger than the node's key.  The height is
the maximum of the height of the smaller and greater children, plus 1.  A node
with no children has a height of 1, similarly a NULL pointer (empty tree) has a
height of 0.  To keep the tree balanced, nodes are moved around so that the
heights of the smaller and larger children differ by at most 1. */

typedef struct AVLDupNodeStruct AVLDupNodeRecord, *AVLDupNodePointer;

struct AVLDupNodeStruct
{
  AVLDupThingRecord key;
  AVLDupThingRecord value;
  AVLDupNodePointer smallerChildPntr;
  AVLDupNodePointer largerChildPntr;
  unsigned int      height; /* Only needs to be uint8, rest is for padding. */
};


/* Comparison functions for the various different data types use this function
prototype.  It returns A - B in effect, so the result is >0 for A > B,
<0 for A < B and 0 for A == B. */

typedef int (* AVLDupComparisonFunctionPointer) (
  AVLDupThingPointer A, AVLDupThingPointer B);


/* The header for the AVLDupTree itself.  Besides pointing out the root node,
it contains auxiliary information needed for comparing the associated data
types, for collecting statistics, multitasking access, and for doing pool
memory allocation. */

struct AVLDupTreeStruct
{
  char *indexName; /* Copy of the user's name for this index. */
  type_code keyType;
  AVLDupComparisonFunctionPointer keyComparisonFunctionPntr;
  type_code valueType;
  AVLDupComparisonFunctionPointer valueComparisonFunctionPntr;
  AVLDupNodePointer rootPntr; /* Root node of the tree or NULL. */
  unsigned int count; /* Counts user provided key/value pairs in tree. */
  sem_id accessSemaphoreID; /* Negative if no semaphore is being used. */
  uint32 maxSimultaneousReaders;
  /* Future work: add a memory pool for nodes and another for strings. */
};


/* This structure is used for keeping common data around during recursive
function calls.  It is faster than the simple technique of having a separate
argument for everything the recursive function might need (like the key and
value you are searching for) - only a single pointer to this structure is
passed into the recursive routines rather than many parameters.  It's also
safer than using global variables, which limit multitasking access to the tree
to 1 thread at a time (so by having one of these structures allocated by each
calling thread, you can have several tree searches going on in parallel). */

typedef struct NonRecursiveArgumentsStruct
{
  type_code keyType;
  AVLDupComparisonFunctionPointer keyComparisonFunctionPntr;
  AVLDupThingRecord userKey1;
  AVLDupThingRecord userKey2;
  type_code valueType;
  AVLDupComparisonFunctionPointer valueComparisonFunctionPntr;
  AVLDupThingRecord userValue1;
  AVLDupThingRecord userValue2;
  bool userValue1WasNULL;
  bool userValue2WasNULL;
  bool includeThingEqualToStart;
  bool includeThingEqualToEnd;
  AVLDupIterationCallbackFunctionPointer iterationCallback;
  void *extraUserData;
} NonRecursiveArgumentsRecord, *NonRecursiveArgumentsPointer;


/* Copy an array of things contents from one thing to another, allocating
memory as needed (the caller provides both thing arrays).  If memory runs
out while copying an array of things, the partial allocations will be
undone, this function will return FALSE and the contents of the destination
will be garbage.  Assumes that the contents of the destination on entry are
uninitialised (so free any strings in the destination yourself if you were
using it to hold strings before calling this function). */

bool AVLDupCopyThingArray (
  AVLDupThingPointer DestThingPntr,
  AVLDupThingPointer SourceThingPntr,
  type_code ThingType,
  int NumberOfThings)
{
  int   i;
  char *SourceStringPntr;
  int   StringLength;
  char *TempStringPntr;

  if (DestThingPntr == NULL || SourceThingPntr == NULL || NumberOfThings <= 0)
    return true; /* Nothing to do. */

  /* Most simple things can be copied directly. */

  if (ThingType != B_STRING_TYPE)
  {
    while (NumberOfThings-- > 0)
      *DestThingPntr++ = *SourceThingPntr++;
    return true;
  }

  /* Strings need extra storage allocated for long strings, but just get
  copied for short strings, and NULL strings get converted to zero length
  strings and stored as short strings. */

  TempStringPntr = (char *) 42; /* Initialise to non-NULL, NULL means error. */

  for (i = 0; i < NumberOfThings; i++, DestThingPntr++, SourceThingPntr++)
  {
    SourceStringPntr = AVLDupGetStringPntrFromThing (*SourceThingPntr);

    /* Clear out the Thing to entirely zero.  This makes it a short string,
    containing a zero length string. */

    DestThingPntr->int64Thing = 0;

    if (SourceStringPntr == NULL)
    {
      /*
      DestThingPntr->shortStringThing[0] = 0; / * Empty string. * /
      DestThingPntr->shortStringThing[7] = 0; / * Marks it as short string. * /
      */
    }
    else
    {
      StringLength = strlen (SourceStringPntr);

      if (StringLength <= 7)
      {
        /* Copy a short string or convert a long string to a short string. */

        strcpy (DestThingPntr->shortStringThing, SourceStringPntr);
        /* DestThingPntr->shortStringThing[7] = 0; Marks it as short string. */
      }
      else /* Long string, need to allocate memory. */
      {
        TempStringPntr = malloc (StringLength + 1);
        if (TempStringPntr == NULL)
          break; /* Oops, out of memory, abort to cleanup! */
        strcpy (TempStringPntr, SourceStringPntr);
        DestThingPntr->longStringThing.stringPntr = TempStringPntr;
        DestThingPntr->longStringThing.isLongString = true;
      }
    }
  }

  /* If we ran out of memory, undo all the allocations for all the previous
  long strings in the array. */

  if (TempStringPntr != NULL)
    return true; /* Successfully did all string copies. */

  for (i--, DestThingPntr--; i >= 0; i--, DestThingPntr--)
  {
    if (DestThingPntr->longStringThing.isLongString)
      free (DestThingPntr->longStringThing.stringPntr);
  }

  return false;
}



/* This utility function just goes through the given number of array elements
and frees the long strings, does nothing for most other data types, the result
is an array of zeroes. */

void AVLDupFreeThingArray (
  AVLDupThingPointer ThingPntr,
  type_code          ThingType,
  int                NumberOfThings)
{
  AVLDupThingPointer FreePntr;
  uint32             i;

  if (ThingPntr == NULL || NumberOfThings <= 0)
    return; /* Nothing to do. */

  if (ThingType == B_STRING_TYPE)
  {
    for (i = NumberOfThings, FreePntr = ThingPntr;
    i > 0;
    i--, FreePntr++)
    {
      if (FreePntr->longStringThing.isLongString)
        free (FreePntr->longStringThing.stringPntr);
    }
  }

  if (NumberOfThings == 1)
    ThingPntr->int64Thing = 0;
  else
    memset (ThingPntr, 0, NumberOfThings * sizeof (AVLDupThingRecord));
}



/* A utility function for converting a Human readable string to a Thing.
Returns TRUE if successful, FALSE if it ran out of memory or something else
went wrong. */

bool AVLDupConvertStringToThing (
  const char *StringPntr,
  AVLDupThingPointer ThingPntr,
  type_code ThingType)
{
  AVLDupThingRecord TempThing;

  if (ThingPntr == NULL)
    return false;

  switch (ThingType)
  {
    case B_INT32_TYPE:
      ThingPntr->int32Thing = atol (StringPntr);
      return true;

    case B_INT64_TYPE:
      ThingPntr->int64Thing = atoll (StringPntr);
      return true;

    case B_FLOAT_TYPE:
      ThingPntr->floatThing = atof (StringPntr); /* atof returns a double. */
      return true;

    case B_DOUBLE_TYPE:
      ThingPntr->doubleThing = atof (StringPntr); /* atof returns a double. */
      return true;

    case B_STRING_TYPE:
      TempThing.longStringThing.stringPntr = (char *) StringPntr;
      TempThing.longStringThing.isLongString = true;
      return AVLDupCopyThingArray (ThingPntr, &TempThing, ThingType, 1);
  }

  return false;
}



/* Convert a Thing into a Human readable string.  Returns TRUE if successful,
FALSE if your buffer is too small or something else is wrong. */

bool AVLDupConvertThingToString (
  AVLDupThingPointer ThingPntr,
  type_code ThingType,
  char *StringPntr,
  int StringSize)
{
  char *TempStringPntr;

  if (ThingPntr == NULL || StringPntr == NULL)
    return false;

  switch (ThingType)
  {
    case B_INT32_TYPE:
      if (StringSize < 12) return false;
      sprintf (StringPntr, "%ld", ThingPntr->int32Thing);
      return true;

    case B_INT64_TYPE:
      if (StringSize < 24) return false;
      sprintf (StringPntr, "%qd", ThingPntr->int64Thing);
      return true;

    case B_FLOAT_TYPE:
      if (StringSize < 24) return false;
      sprintf (StringPntr, "%g", (double) ThingPntr->floatThing);
      return true;

    case B_DOUBLE_TYPE:
      if (StringSize < 24) return false;
      sprintf (StringPntr, "%g", ThingPntr->doubleThing);
      return true;

    case B_STRING_TYPE:
      if (StringSize < 4) return false;
      TempStringPntr = AVLDupGetStringPntrFromThing (*ThingPntr);
      if (TempStringPntr == NULL)
        *StringPntr = 0; /* Return an empty string instead of NULL. */
      else if (strlen (TempStringPntr) < StringSize)
        strcpy (StringPntr, TempStringPntr);
      else /* Destination string is too short. */
      {
        strncpy (StringPntr, TempStringPntr, StringSize);
        strcpy (StringPntr + (StringSize-4), "...");
      }
      return true;
  }

  return false;
}



/* A series of value comparison functions for internal use.  They essentially
return the sign of the operation A - B, so the result is >0 for A > B,
<0 for A < B and 0 for A equals B. */

static int CompareInt32 (AVLDupThingPointer A, AVLDupThingPointer B)
{
  int32 Delta;

  Delta = A->int32Thing - B->int32Thing;
  if (Delta < 0)
    return -1;
  else if (Delta > 0)
    return 1;
  return 0;
}


static int CompareInt64 (AVLDupThingPointer A, AVLDupThingPointer B)
{
  int64 Delta;

  Delta = A->int64Thing - B->int64Thing;
  if (Delta < 0)
    return -1;
  else if (Delta > 0)
    return 1;
  return 0;
}


static int CompareFloat (AVLDupThingPointer A, AVLDupThingPointer B)
{
  float Delta;

  Delta = A->floatThing - B->floatThing;
  if (Delta < 0)
    return -1;
  else if (Delta > 0)
    return 1;
  return 0;
}


static int CompareDouble (AVLDupThingPointer A, AVLDupThingPointer B)
{
  double Delta;

  Delta = A->doubleThing - B->doubleThing;
  if (Delta < 0)
    return -1;
  else if (Delta > 0)
    return 1;
  return 0;
}


static int CompareString (AVLDupThingPointer A, AVLDupThingPointer B)
{
  const char *StringAPntr;
  const char *StringBPntr;

  StringAPntr = AVLDupGetStringPntrFromThing (*A);
  StringBPntr = AVLDupGetStringPntrFromThing (*B);

  /* Handle NULL pointer - represents a string smaller than all others,
  though you shouldn't see it in practice (normally it gets converted to
  an empty string in short string format). */

  if (StringAPntr == NULL && StringBPntr == NULL)
    return 0;

  if (StringAPntr == NULL)
    return -1;

  if (StringBPntr == NULL)
    return 1;

  /* Use an ordinary strcmp for the rest of the comparison.  We are comparing
  UTF-8 strings, so the ordering of multibyte characters could be different
  than what strcmp returns, but we'll leave it as it is for now.  Also, we
  could implement a comparison which ignores case. */

  return strcmp (StringAPntr, StringBPntr);
}



/* Internal utility to find a comparison function which is appropriate for the
specified type of data.  Returns NULL if none exists. */

static AVLDupComparisonFunctionPointer GetComparisonFunctionForType (
  type_code KeyType)
{
  switch (KeyType)
  {
    case B_INT32_TYPE:
      return CompareInt32;

    case B_INT64_TYPE:
      return CompareInt64;

    case B_FLOAT_TYPE:
      return CompareFloat;

    case B_DOUBLE_TYPE:
      return CompareDouble;

    case B_STRING_TYPE:
      return CompareString;
  }

  return NULL;
}



/* Create a new empty AVLDupTree.  Returns the new initialised tree header or
NULL if out of memory or any of the key/value data types are unsupported.  You
can specify NULL for the IndexName if you don't want to waste space for the
name.  MaxSimultaneousReaders is the number of simultaneous readers allowed
for multitasking access, use a large number like a billion if you want
essentially any number of readers.  Only one writer is allowed at a time.  If
you specify zero, then multitasking protections are disabled and you get a
slight speed improvement. */

AVLDupTreePointer AVLDupAllocTree (
  type_code   KeyType,
  type_code   ValueType,
  const char *IndexName,
  uint32      MaxSimultaneousReaders)
{
  int                NameLength;
  AVLDupTreePointer  NewTree;

  NewTree = malloc (sizeof (AVLDupTreeRecord));
  if (NewTree == NULL) goto ErrorExit;

  /* Set up some initial values so that cleanup of
  further errors won't cause a crash. */

  NewTree->rootPntr = NULL;
  NewTree->count = 0;
  NewTree->accessSemaphoreID = -1;
  NewTree->maxSimultaneousReaders = MaxSimultaneousReaders;

  /* Copy the user provided title string, if provided. */

  if (IndexName != NULL)
  {
    NameLength = strlen (IndexName);
    NewTree->indexName = malloc (NameLength + 1);
    if (NewTree->indexName == NULL) goto ErrorExit;
    strcpy (NewTree->indexName, IndexName);
  }
  else
    NewTree->indexName = NULL;

  /* Create the multitasking access protection semaphore, if desired. */

  if (MaxSimultaneousReaders > 0)
  {
    NewTree->accessSemaphoreID = create_sem (MaxSimultaneousReaders,
      "AVLDupTree Access");
    if (NewTree->accessSemaphoreID < 0) goto ErrorExit;
  }

  /* Set up the data types and corresponding comparison functions. */

  NewTree->keyType = KeyType;
  NewTree->keyComparisonFunctionPntr =
    GetComparisonFunctionForType (KeyType);
  if (NewTree->keyComparisonFunctionPntr == NULL) goto ErrorExit;

  NewTree->valueType = ValueType;
  NewTree->valueComparisonFunctionPntr =
    GetComparisonFunctionForType (ValueType);
  if (NewTree->valueComparisonFunctionPntr == NULL) goto ErrorExit;

  return NewTree;


ErrorExit: /* Deallocate partial allocations and return NULL. */
  AVLDupFreeTree (NewTree);
  return NULL;
}



/* Internal function for removing nodes.  Deallocates the node's children,
the node's key and value, and the node itself.  It needs the tree only for
the types of the key and value.  It does not update the tree record at all,
nor rebalance the tree, nor fix up parent nodes of the deleted nodes. */

static void AVLDupRecursiveDeallocateNodes (
  AVLDupTreePointer  TreePntr,
  AVLDupNodePointer  CurrentNode)
{
  if (CurrentNode == NULL)
    return;

  /* Delete child nodes. */

  AVLDupRecursiveDeallocateNodes (TreePntr, CurrentNode->smallerChildPntr);
  AVLDupRecursiveDeallocateNodes (TreePntr, CurrentNode->largerChildPntr);

  /* Deallocate associated data. */

  AVLDupFreeThingArray (&CurrentNode->key, TreePntr->keyType, 1);
  AVLDupFreeThingArray (&CurrentNode->value, TreePntr->valueType, 1);

  /* Finally get rid of the node itself. */

  memset (CurrentNode, 0, sizeof (AVLDupNodeRecord));
  free (CurrentNode);
}



/* Deallocate an AVLDupTree.  Deallocates all the nodes and associated strings
if any, then frees the tree header record.  Safe to pass in NULL.  Should be
called by same team of threads as the one which allocated the tree. */

void AVLDupFreeTree (AVLDupTreePointer TreePntr)
{
  if (TreePntr != NULL)
  {
    if (TreePntr->accessSemaphoreID >= 0)
    {
      /* Wait for all readers and writers to leave the premises. */

      acquire_sem_etc (TreePntr->accessSemaphoreID,
        TreePntr->maxSimultaneousReaders /* we are writer, grab all */, 0, 0);

      /* Delete the semaphore.  All the waiting threads (readers and writers)
      will wake up and find an error code returned from acquire_sem_etc, and
      return immediately rather than doing any operations on the tree. */

      delete_sem (TreePntr->accessSemaphoreID);
    }

    if (TreePntr->rootPntr != NULL)
      AVLDupRecursiveDeallocateNodes (TreePntr, TreePntr->rootPntr);

    if (TreePntr->indexName != NULL)
      free (TreePntr->indexName);

    memset (TreePntr, 0, sizeof (AVLDupTreeRecord));
    free (TreePntr);
  }
}



/* Returns the number of key/value pairs in the tree. */

unsigned int AVLDupGetTreeCount (AVLDupTreePointer TreePntr)
{
  if (TreePntr != NULL)
    return TreePntr->count;

  return 0;
}



/* Returns the name of the tree, or NULL if it isn't named.  You can look
at the string, but don't change it as it belongs to the internal tree code. */

const char *AVLDupGetTreeName (AVLDupTreePointer TreePntr)
{
  if (TreePntr != NULL)
    return TreePntr->indexName;

  return NULL;
}



/* Internal function for setting the height of a node to be the larger of
the child nodes' heights, plus one. */

static void AVLDupRecalculateNodeHeight (AVLDupNodePointer CurrentNode)
{
  unsigned int LeftHeight;
  unsigned int RightHeight;

  if (CurrentNode->smallerChildPntr == NULL)
    LeftHeight = 0;
  else
    LeftHeight = CurrentNode->smallerChildPntr->height;

  if (CurrentNode->largerChildPntr == NULL)
    RightHeight = 0;
  else
    RightHeight = CurrentNode->largerChildPntr->height;

  if (LeftHeight < RightHeight)
    CurrentNode->height = RightHeight + 1;
  else
    CurrentNode->height = LeftHeight + 1;
}



/* Internal function for making the left child into the root of the (sub)tree,
pushing the former root down to the right.  The Dr. Dobbs article calls this
a Right Rotation.  It's useful for balancing when the left subtree is deeper
than the right subtree.  It maintains the binary tree property (left is less,
right is greater) but doesn't maintain the AVL properties and may change the
height of the (sub)tree (the parent will need to recalculate its height;
however the heights of the nodes involved will be updated by this function).

The shuffle works like this: the left child becomes the root of the tree,
and since the old root was larger than the left child, it can be safely added
as the new right child of the new root.  So, what happens to the old right
child of the new root?  We know it is less than the old root, so we add it
as a left child of the old root (conveniently free because the left child
was moved up and become the new root).  The left child of the old left child
is still attached where it was, so it is now the left child of the new root.
The same goes for the right child of the old root, which remains attached to
that node and is thus now the right child of the new right child.

Here's an ASCII picture, 1 and 2 are nodes (1 has a smaller key than 2), and
A, B, C are subtrees (which can be NULL):

  Before        After

     2           1
    / \         / \
   1   C       A   2
  / \             / \
 A   B           B   C

*/

static void AVLDupRaiseLeftChild (AVLDupNodePointer *ParentNodePntrPntr)
{
  AVLDupNodePointer  Node1Pntr;
  AVLDupNodePointer  Node2Pntr;

  Node2Pntr = *ParentNodePntrPntr;
  if (Node2Pntr == NULL) return; /* Nothing to do. */

  Node1Pntr = Node2Pntr->smallerChildPntr;
  if (Node1Pntr == NULL) return; /* Oops, can't raise "nothing" up. */

  /* Do the BST rotation. */

  *ParentNodePntrPntr = Node1Pntr;
  Node2Pntr->smallerChildPntr = Node1Pntr->largerChildPntr;
  Node1Pntr->largerChildPntr = Node2Pntr;

  /* Fix up the heights of the nodes which have been changed, except for
  the parent node, which the caller should fix up.  Do it in order of
  increasing tree hierarchy, from lower nodes to higher nodes so that it
  gets the right value! */

  AVLDupRecalculateNodeHeight (Node2Pntr);
  AVLDupRecalculateNodeHeight (Node1Pntr);
}



/* Internal function for making the right child into the root of the
(sub)tree, pushing the former root down to the left.

Here's an ASCII picture, 1 and 2 are nodes (1 has a smaller key than 2), and
A, B, C are subtrees (which can be NULL):

 Before        After

   1              2
  / \            / \
 A   2          1   C
    / \        / \
   B   C      A   B

*/

static void AVLDupRaiseRightChild (AVLDupNodePointer *ParentNodePntrPntr)
{
  AVLDupNodePointer  Node1Pntr;
  AVLDupNodePointer  Node2Pntr;

  Node1Pntr = *ParentNodePntrPntr;
  if (Node1Pntr == NULL) return; /* Nothing to do. */

  Node2Pntr = Node1Pntr->largerChildPntr;
  if (Node2Pntr == NULL) return; /* Oops, can't raise "nothing" up. */

  /* Do the BST rotation. */

  *ParentNodePntrPntr = Node2Pntr;
  Node1Pntr->largerChildPntr = Node2Pntr->smallerChildPntr;
  Node2Pntr->smallerChildPntr = Node1Pntr;

  /* Fix up the heights of the nodes which have been changed. */

  AVLDupRecalculateNodeHeight (Node1Pntr);
  AVLDupRecalculateNodeHeight (Node2Pntr);
}



/* Internal function for rebalancing the left and right subtrees if their
heights differ by more than 1 so that they end up differing by at most 1.
It also updates the height of the node in all cases.  This function is called
after an addition or deletion is made to the tree, as part of the
addition/deletion recursive chain so that it rebalances the changed parts all
the way from the added/deleted node up to the root. */

static void AVLDupFixupSubtrees (AVLDupNodePointer *ParentNodePntrPntr)
{
  AVLDupNodePointer CurrentNode;
  int               Delta;
  unsigned int      LeftHeight;
  AVLDupNodePointer RaiseNode;
  unsigned int      RightHeight;

  CurrentNode = *ParentNodePntrPntr;
  if (CurrentNode == NULL) return; /* Nothing to do. */

  if (CurrentNode->smallerChildPntr == NULL)
    LeftHeight = 0;
  else
    LeftHeight = CurrentNode->smallerChildPntr->height;

  if (CurrentNode->largerChildPntr == NULL)
    RightHeight = 0;
  else
    RightHeight = CurrentNode->largerChildPntr->height;

  Delta = (int) LeftHeight - (int) RightHeight;

  if (Delta <= -2)
  {
    /* Left side is deficient in height, raise the right side node up to be
    the new root of the subtree, reducing the height of the right side of the
    tree relative to the left side, and increasing the left side height by
    pushing down the old root of the subtree into the left side. */

    /* But before raising the right child, make sure the right side's right
    child (a grandchild) is the deeper of the two grandchildren otherwise the
    raised node will have a right child which isn't deep enough (the raised
    right node keeps its right grandchild) and the AVL property won't be
    true (right grandchild will be off by more than 1 in height from the left
    grandchild). */

    RaiseNode = CurrentNode->largerChildPntr;
    LeftHeight = (RaiseNode->smallerChildPntr == NULL) ?
      0 : RaiseNode->smallerChildPntr->height;
    RightHeight = (RaiseNode->largerChildPntr == NULL) ?
      0 : RaiseNode->largerChildPntr->height;
    if (RightHeight < LeftHeight)
      AVLDupRaiseLeftChild (&CurrentNode->largerChildPntr);

    AVLDupRaiseRightChild (ParentNodePntrPntr);
  }
  else if (Delta >= 2)
  {
    /* Right side is deficient in height, lower right / raise left side. */

    RaiseNode = CurrentNode->smallerChildPntr;
    LeftHeight = (RaiseNode->smallerChildPntr == NULL) ?
      0 : RaiseNode->smallerChildPntr->height;
    RightHeight = (RaiseNode->largerChildPntr == NULL) ?
      0 : RaiseNode->largerChildPntr->height;
    if (LeftHeight < RightHeight) /* Avoid AVL grandchild problem. */
      AVLDupRaiseRightChild (&CurrentNode->smallerChildPntr);

    AVLDupRaiseLeftChild (ParentNodePntrPntr);
  }
  else /* Delta is -1, 0, or +1.  No balancing needed, just update height. */
  {
    /* Rather than calling AVLDupRecalculateNodeHeight, we can do it faster
    right here.  Use the larger of the child heights + 1. */

    if (Delta < 0)
      CurrentNode->height = RightHeight + 1;
    else
      CurrentNode->height = LeftHeight + 1;
  }
}



/* Internal function for adding a new node.  It recursively traverses the tree
until the key/value is found (in which case it does nothing if it is exactly
the same as an existing node), or it finds an empty spot where it can add the
new node.  The parent node's pointer to the child being considered is passed
in so that we can change the parent's child pointer when the tree gets
rearranged as part of the balancing, without knowing if the parent is a node
(left or right subree) or the tree header.  Returns RAN_ADDED_A_NODE if it
added a node, RAN_ALREADY_IN_TREE if it found a duplicate key/value (does
nothing to tree), and RAN_OUT_OF_MEMORY if it ran out of memory (also does
nothing to the existing tree). */

typedef enum RecursiveAddNodeReturnCodesEnum {
  RAN_OUT_OF_MEMORY = -1,
  RAN_ALREADY_IN_TREE = 0,
  RAN_ADDED_A_NODE = 1
} RANReturnCode;

static RANReturnCode AVLDupRecursiveAddNode (
  NonRecursiveArgumentsPointer ArgsPntr,
  AVLDupNodePointer           *ParentsChildPntrPntr)
{
  int               ComparisonResult;
  AVLDupNodePointer CurrentNode;
  RANReturnCode     ErrorCode;
  AVLDupNodePointer NewNode;

  CurrentNode = *ParentsChildPntrPntr;
  if (CurrentNode == NULL) /* Empty tree or new leaf situation. */
  {
    /* Create a new node and add it to the tree. */

    NewNode = malloc (sizeof (AVLDupNodeRecord));
    if (NewNode == NULL)
      return RAN_OUT_OF_MEMORY;

    /* Copy the key and value to the new node. */

    if (!AVLDupCopyThingArray (&NewNode->key, &ArgsPntr->userKey1,
    ArgsPntr->keyType, 1))
    {
      free (NewNode);
      return RAN_OUT_OF_MEMORY;
    }
    if (!AVLDupCopyThingArray (&NewNode->value, &ArgsPntr->userValue1,
    ArgsPntr->valueType, 1))
    {
      AVLDupFreeThingArray (&NewNode->key, ArgsPntr->keyType, 1);
      free (NewNode);
      return RAN_OUT_OF_MEMORY;
    }

    /* Set up the remaining fields in the new node. */

    NewNode->smallerChildPntr = NULL;
    NewNode->largerChildPntr = NULL;
    NewNode->height = 1;

    *ParentsChildPntrPntr = NewNode;
    return RAN_ADDED_A_NODE;
  }

  /* Adding a node to an existing CurrentNode.  See if the key/value pair is
  less than, equal to, or greater than the CurrentNode's key/value, which will
  decide which subtree to add the new node to. */

  ComparisonResult = ArgsPntr->keyComparisonFunctionPntr (
    &ArgsPntr->userKey1, &CurrentNode->key);

  if (ComparisonResult == 0) /* Equal keys, use the value to decide. */
    ComparisonResult = ArgsPntr->valueComparisonFunctionPntr (
    &ArgsPntr->userValue1, &CurrentNode->value);

  /* Key/value pair is totally equal to the current node.  Just do nothing. */

  if (ComparisonResult == 0)
    return RAN_ALREADY_IN_TREE;

  /* Add the new key/value to one of the subtrees. */

  if (ComparisonResult < 0)
  {
    /* Key is smaller than the current node, add the new node to the left
    subtree of the current node. */

    ErrorCode =
      AVLDupRecursiveAddNode (ArgsPntr, &CurrentNode->smallerChildPntr);
  }
  else
  {
    /* Key is larger than the current node, add the new node to the right
    subtree of the current node. */

    ErrorCode =
      AVLDupRecursiveAddNode (ArgsPntr, &CurrentNode->largerChildPntr);
  }

  /* If the node wasn't added, the tree hasn't changed. */

  if (ErrorCode != RAN_ADDED_A_NODE)
    return ErrorCode;

  /* A new node was added as a child.  Do post-addition fixups.  Recompute
  heights and rebalance the tree if needed. */

  AVLDupFixupSubtrees (ParentsChildPntrPntr);

  return RAN_ADDED_A_NODE;
}



/* Adds a key/value pair to the AVLDupTree.  Returns TRUE if successful, FALSE
if it ran out of memory or something else went wrong (program interrupted while
waiting on a semaphore, or tree deleted while waiting).  Also returns TRUE and
does nothing if you specified an existing key/value pair.  The types of the key
and value are assumed to be the same as defined in the tree header record.  The
key and value are copied so that you can deallocate your values after calling
this function.  If you specified a string thing, the string will be copied by
the tree routines, so you can free your string too.  It will also convert
internally between short and long strings, so you can always pass in a long
string if you wish. */

bool AVLDupAdd (
  AVLDupTreePointer  TreePntr,
  AVLDupThingPointer Key,
  AVLDupThingPointer Value)
{
  NonRecursiveArgumentsRecord Arguments;
  status_t                    ErrorCode;
  RANReturnCode               ReturnCode;

  if (TreePntr == NULL || Key == NULL || Value == NULL)
    return false;

  if (TreePntr->accessSemaphoreID >= 0)
  {
    ErrorCode = acquire_sem_etc (TreePntr->accessSemaphoreID,
      TreePntr->maxSimultaneousReaders /* we are a writer, grab all */, 0, 0);
    if (ErrorCode < 0)
      return false; /* Semaphore was deleted or a signal interrupted us. */
  }

  Arguments.keyType = TreePntr->keyType;
  Arguments.keyComparisonFunctionPntr = TreePntr->keyComparisonFunctionPntr;
  Arguments.userKey1 = *Key;
  Arguments.valueType = TreePntr->valueType;
  Arguments.valueComparisonFunctionPntr= TreePntr->valueComparisonFunctionPntr;
  Arguments.userValue1 = *Value;

  ReturnCode = AVLDupRecursiveAddNode (&Arguments, &TreePntr->rootPntr);

  if (ReturnCode == RAN_ADDED_A_NODE)
    TreePntr->count++;

  if (TreePntr->accessSemaphoreID >= 0)
  {
    release_sem_etc (TreePntr->accessSemaphoreID,
      TreePntr->maxSimultaneousReaders, B_DO_NOT_RESCHEDULE);
  }

  return (ReturnCode != RAN_OUT_OF_MEMORY);
}



/* An internal recursive function for finding the successor to some distant
parent node (just follow the left links until NULL is encountered), and remove
it from the tree (don't deallocate, just unlink and fix up its children).
Returns the removed node. */

static AVLDupNodePointer AVLDupRecursiveRipOutSuccessor (
  AVLDupNodePointer *ParentsChildPntrPntr)
{
  AVLDupNodePointer CurrentNode;
  AVLDupNodePointer RippedOutNode;

  CurrentNode = *ParentsChildPntrPntr;

  if (CurrentNode->smallerChildPntr == NULL)
  {
    /* Unlink the node from the tree.  Don't need to fix up its child since
    we didn't change it.  But the caller will need to fix up its heights
    and rebalance too, as we changed a child. */

    *ParentsChildPntrPntr = CurrentNode->largerChildPntr;
    CurrentNode->largerChildPntr = NULL;
    return CurrentNode;
  }

  /* Not at the successor yet, continue on down. */

  RippedOutNode =
    AVLDupRecursiveRipOutSuccessor (&CurrentNode->smallerChildPntr);

  /* Since child trees may have been modified, need to recalculate our
  height and do rebalancing. */

  AVLDupFixupSubtrees (ParentsChildPntrPntr);

  return RippedOutNode;
}



/* An internal recursive function for finding and deleting a node matching the
given key/value pair.  Once found, if the node has zero or one children, it is
removed from the tree normally.  If it has two children, then it gets replaced
by the next larger key in the tree, which is removed from its old position in
the tree.  The next larger value is guaranteed to be a leaf node in the tree,
found by taking the right child, then all the left children until there are no
more left children.  That next-larger-value node is ripped out of the tree
(replaced by it's right child if any).  Then the rip-out traversal returns back
to the deletion point (fixing up the tree heights along the way), and the
replacement of the deleted node with the next larger node is made.  The
deletion function then returns, continuing to fix up heights along the return
chain.  The parent node's pointer to the node being considered is passed in so
that we can change the parent's child pointer when the tree gets rearranged as
part of the balancing, without knowing if the parent is a node (left or right
subree) or the tree header.  Returns TRUE if it deleted the node, FALSE if it
couldn't find it. */

static bool AVLDupRecursiveDeleteNodeFindIt (
  NonRecursiveArgumentsPointer ArgsPntr,
  AVLDupNodePointer *ParentsChildPntrPntr)
{
  int               ComparisonResult;
  AVLDupNodePointer CurrentNode;
  bool              ReturnCode;
  AVLDupNodePointer SuccessorNode;

  CurrentNode = *ParentsChildPntrPntr;
  if (CurrentNode == NULL) return false; /* Failed to find node to delete. */

  ComparisonResult = ArgsPntr->keyComparisonFunctionPntr (
    &ArgsPntr->userKey1, &CurrentNode->key);

  if (ComparisonResult == 0) /* Equal keys, use the value to decide. */
    ComparisonResult = ArgsPntr->valueComparisonFunctionPntr (
    &ArgsPntr->userValue1, &CurrentNode->value);

  if (ComparisonResult < 0)
    ReturnCode =
    AVLDupRecursiveDeleteNodeFindIt (ArgsPntr, &CurrentNode->smallerChildPntr);
  else if (ComparisonResult > 0)
    ReturnCode =
    AVLDupRecursiveDeleteNodeFindIt (ArgsPntr, &CurrentNode->largerChildPntr);
  else /* Found the key/value pair. */
  {
    if (CurrentNode->largerChildPntr == NULL)
    {
      /* Can delete this node directly, replacing it with the left subtree. */

      *ParentsChildPntrPntr = CurrentNode->smallerChildPntr;
    }
    else if (CurrentNode->smallerChildPntr == NULL)
    {
      /* Safe to replace the node with the right subtree. */

      *ParentsChildPntrPntr = CurrentNode->largerChildPntr;
    }
    else /* Has 2 children.  Replace with successor node. */
    {
      SuccessorNode = AVLDupRecursiveRipOutSuccessor (
        &CurrentNode->largerChildPntr);

      SuccessorNode->smallerChildPntr = CurrentNode->smallerChildPntr;
      SuccessorNode->largerChildPntr = CurrentNode->largerChildPntr;
      *ParentsChildPntrPntr = SuccessorNode;
    }

    /* Deallocate the old node, which matched our key/value pair. */

    AVLDupFreeThingArray (&CurrentNode->key, ArgsPntr->keyType, 1);
    AVLDupFreeThingArray (&CurrentNode->value, ArgsPntr->valueType, 1);
    memset (CurrentNode, 0, sizeof (AVLDupNodeRecord));
    free (CurrentNode);
    ReturnCode = true;
  }

  /* Recompute heights and rebalance the tree if needed. */

  AVLDupFixupSubtrees (ParentsChildPntrPntr);

  return ReturnCode;
}



/* Deletes the given key/value pair.  Yes, you need to specify a value since
duplicate keys can't otherwise be distinguished.  Returns FALSE if it can't
find the key/value pair or was interupted, TRUE if it deleted it. */

bool AVLDupDelete (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer Key,
  AVLDupThingPointer Value)
{
  NonRecursiveArgumentsRecord Arguments;
  status_t                    ErrorCode;
  bool                        Successful;

  if (TreePntr == NULL || Key == NULL || Value == NULL)
    return false;

  if (TreePntr->accessSemaphoreID >= 0)
  {
    ErrorCode = acquire_sem_etc (TreePntr->accessSemaphoreID,
      TreePntr->maxSimultaneousReaders /* we are a writer, grab all */, 0, 0);
    if (ErrorCode < 0)
      return false; /* Semaphore was deleted or a signal interrupted us. */
  }

  Arguments.keyType = TreePntr->keyType;
  Arguments.keyComparisonFunctionPntr = TreePntr->keyComparisonFunctionPntr;
  Arguments.userKey1 = *Key;
  Arguments.valueType = TreePntr->valueType;
  Arguments.valueComparisonFunctionPntr= TreePntr->valueComparisonFunctionPntr;
  Arguments.userValue1 = *Value;

  Successful =
    AVLDupRecursiveDeleteNodeFindIt (&Arguments, &TreePntr->rootPntr);

  if (Successful)
    TreePntr->count--;

  if (TreePntr->accessSemaphoreID >= 0)
  {
    release_sem_etc (TreePntr->accessSemaphoreID,
      TreePntr->maxSimultaneousReaders, B_DO_NOT_RESCHEDULE);
  }

  return Successful;
}



/* Internal recursive function for iterating over a subtree without doing
any tests.  Even assumes that the entry node is non-NULL.  Returns TRUE if
successful, FALSE if the user callback aborted the iteration. */

static bool AVLDupRecursiveSimpleIterate (
  NonRecursiveArgumentsPointer ArgsPntr,
  AVLDupNodePointer CurrentNode)
{
  /* Output left subtree first, since we are doing it in ascending order. */

  if (CurrentNode->smallerChildPntr != NULL)
  {
    if (!AVLDupRecursiveSimpleIterate (ArgsPntr,
    CurrentNode->smallerChildPntr))
      return false; /* User aborted. */
  }

  /* Output the middle node. */

  if (!ArgsPntr->iterationCallback (&CurrentNode->key, &CurrentNode->value,
  ArgsPntr->extraUserData))
    return false; /* The user requested an early abort of the iteration. */

  /* Finally the right subtree. */

  if (CurrentNode->largerChildPntr != NULL)
  {
    if (!AVLDupRecursiveSimpleIterate (ArgsPntr,
    CurrentNode->largerChildPntr))
      return false; /* User aborted. */
  }

  return true;
}



/* Recursively iterate over the tree, cutting off traversals which don't fit
in the given range of keys.  Returns TRUE if it got to the end of the range.
If TestLowerBound is TRUE then tests will be done against userKey1 (the lower
bound key).  If it is FALSE then we will assume that the node and all its
children are greater than or equal to userKey1 and avoid the test.  Similarly
if TestUpperBound is FALSE then we assume everything is less than or equal to
the upper bound.  If both are false, we revert to a simple tree traversal and
do no tests. */

static bool AVLDupRecursiveRangeIterate (
  NonRecursiveArgumentsPointer ArgsPntr,
  AVLDupNodePointer CurrentNode,
  bool TestLowerBound,
  bool TestUpperBound)
{
  int ComparisonLower;
  int ComparisonUpper;

  if (CurrentNode == NULL)
    return true; /* Successfully finished iterating the NULL tree. */

  if (!(TestLowerBound || TestUpperBound))
    return AVLDupRecursiveSimpleIterate (ArgsPntr, CurrentNode);

  /* For convenience in understanding the code, both high and low comparisons
  are done as (BoundsLimit - CurrentNodeKey).  Meaning ComparisonResult is
  less than zero for key bigger than the bounds limit, and so on. */

  if (TestLowerBound)
  {
    ComparisonLower = ArgsPntr->keyComparisonFunctionPntr (
    &ArgsPntr->userKey1, &CurrentNode->key);

    if (ComparisonLower == 0) /* Equal keys, use the value to decide. */
    {
      if (ArgsPntr->userValue1WasNULL)
        ComparisonLower = -1; /* Effectively lower bound is -infinity. */
      else
        ComparisonLower = ArgsPntr->valueComparisonFunctionPntr (
        &ArgsPntr->userValue1, &CurrentNode->value);
    }
  }
  else /* No comparison, current is always larger. */
    ComparisonLower = -1;

  if (TestUpperBound)
  {
    ComparisonUpper = ArgsPntr->keyComparisonFunctionPntr (
    &ArgsPntr->userKey2, &CurrentNode->key);

    if (ComparisonUpper == 0) /* Equal keys, use the value to decide. */
    {
      if (ArgsPntr->userValue2WasNULL)
        ComparisonUpper = 1; /* Effectively upper bound is +infinity. */
      else
        ComparisonUpper = ArgsPntr->valueComparisonFunctionPntr (
        &ArgsPntr->userValue2, &CurrentNode->value);
    }
  }
  else /* No comparison, current is always smaller. */
    ComparisonUpper = 1;

  /* Examine the left subtree.  The lower limit of the range has to be less
  than the current node's key otherwise the lower tree is outside the bounds
  and doesn't need to be traversed.  If the upper limit is greater than or
  equal to the current key then the left subtree evaluation doesn't have to
  check the upper bound as it is completely below it. */

  if (ComparisonLower < 0) /* If lower bound is less than current key. */
  {
    if (!AVLDupRecursiveRangeIterate (ArgsPntr, CurrentNode->smallerChildPntr,
    TestLowerBound, (ComparisonUpper >= 0) ? false : TestUpperBound))
      return false; /* The user requested an early abort of the iteration. */
  }

  /* Output the current node, if it is between lower and upper bounds.
  Well, maybe not - if the user doesn't want to output nodes equal to the
  start and end bounds. */

  if ((ComparisonLower < 0 ||
  (ComparisonLower == 0 && ArgsPntr->includeThingEqualToStart)) &&
  (ComparisonUpper > 0 ||
  (ComparisonUpper == 0 && ArgsPntr->includeThingEqualToEnd)))
  {
    if (!ArgsPntr->iterationCallback (&CurrentNode->key, &CurrentNode->value,
    ArgsPntr->extraUserData))
      return false; /* The user requested an early abort of the iteration. */
  }

  /* Examine the right subtree.  The upper limit of the range has to be larger
  than the current key otherwise nothing needs to be done.  If the lower limit
  is less than or equal to the current key then no lower limit checks need to
  be done for the subtree. */

  if (ComparisonUpper > 0)
  {
    if (!AVLDupRecursiveRangeIterate (ArgsPntr, CurrentNode->largerChildPntr,
    (ComparisonLower <= 0) ? false : TestLowerBound, TestUpperBound))
      return false; /* The user requested an early abort of the iteration. */
  }

  return true; /* Got successfully to the end of this subtree iteration. */
}



/* This function will call the user provided callback function for every
key/value pair in the given range, optionally including ones which equal the
start and end keys.  AVLDupIterate will return TRUE if it reached the end of
the iteration, or FALSE if you stopped it early.

If you want to start at the beginning, pass in NULL for StartKeyPntr
(StartValuePntr will be ignored in that case).  Otherwise, when specifying a
non-NULL key, you can also pass in NULL for StartValuePntr if you want to
start at the lowest value available for the key (useful for iterating over all
values for a given key).  If IncludeThingEqualToStart is TRUE then the
key/value pair matching your starting key/value will be included in the
iteration.  If not, the iteration's first item is the key/value just greater
than your starting key/value.  Similarly you can use NULL for EndKeyPntr and
EndValuePntr and control equality with the ending key/value with
IncludeThingEqualToEnd.  One example: a full tree traversal can be done by
passing in NULL for both StartKeyPntr and EndKeyPntr (equality settings don't
matter).  Another example: for all values for a given key, pass in the same key
as StartKeyPntr and EndKeyPntr, and set StartValuePntr and EndValuePntr to
NULL.

While the iteration is proceeding, you can't modify the tree, so other threads
are only allowed to do read operations.  For that matter, your callback
function can't call routines which modify the tree either, or change the
key/value items it is passed.  If your callback function returns FALSE, then
the iteration will be stopped early.  ExtraUserData is also passed to your
function, it can be whatever you want it to be. */

bool AVLDupIterate (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer StartKeyPntr,
  AVLDupThingPointer StartValuePntr,
  bool IncludeThingEqualToStart,
  AVLDupThingPointer EndKeyPntr,
  AVLDupThingPointer EndValuePntr,
  bool IncludeThingEqualToEnd,
  AVLDupIterationCallbackFunctionPointer CallbackFunctionPntr,
  void *ExtraUserData)
{
  NonRecursiveArgumentsRecord Arguments;
  status_t                    ErrorCode;
  bool                        Successful;

  if (TreePntr == NULL || CallbackFunctionPntr == NULL)
    return false;

  if (TreePntr->accessSemaphoreID >= 0)
  {
    ErrorCode = acquire_sem_etc (TreePntr->accessSemaphoreID,
      1 /* we are a reader, grab just 1 unit */, 0, 0);
    if (ErrorCode < 0)
      return false; /* Semaphore was deleted or a signal interrupted us. */
  }

  Arguments.keyType = TreePntr->keyType;
  Arguments.keyComparisonFunctionPntr = TreePntr->keyComparisonFunctionPntr;
  Arguments.valueType = TreePntr->valueType;
  Arguments.valueComparisonFunctionPntr= TreePntr->valueComparisonFunctionPntr;
  Arguments.includeThingEqualToStart = IncludeThingEqualToStart;
  Arguments.includeThingEqualToEnd = IncludeThingEqualToEnd;
  Arguments.iterationCallback = CallbackFunctionPntr;
  Arguments.extraUserData = ExtraUserData;

  /* Copy the starting key and value, if present, to our semi-global data. */

  if (StartKeyPntr != NULL)
  {
    Arguments.userKey1 = *StartKeyPntr;
    if (StartValuePntr == NULL)
      Arguments.userValue1WasNULL = true;
    else
    {
      Arguments.userValue1 = *StartValuePntr;
      Arguments.userValue1WasNULL = false;
    }
  }

  /* Copy the ending key and value, if present. */

  if (EndKeyPntr != NULL)
  {
    Arguments.userKey2 = *EndKeyPntr;
    if (EndValuePntr == NULL)
      Arguments.userValue2WasNULL = true;
    else
    {
      Arguments.userValue2 = *EndValuePntr;
      Arguments.userValue2WasNULL = false;
    }
  }

  /* Start off the big recursive iteration. */

  Successful = AVLDupRecursiveRangeIterate (&Arguments, TreePntr->rootPntr,
    StartKeyPntr != NULL, EndKeyPntr != NULL);

  if (TreePntr->accessSemaphoreID >= 0)
    release_sem_etc (TreePntr->accessSemaphoreID, 1, B_DO_NOT_RESCHEDULE);

  return Successful;
}



/******************************************************************************
 * Some possible functions to implement at some future time.
 */

/* Find a bunch of values given a key, or just get a count of the number of
values for a key.  Searches the tree and optionally fills in a user provided
array of things, with copies of all the values at the time of the operation.
You need to free any copied long strings in the things array (use
AVLDupFreeThingArray() to do it if the values are string type things).  You
can pass in zero for ArraySizeInThings if you just want to find out if a
particular key exists and how many values it has.  The pointers
NumberOfValuesActuallyInTree, NumberOfThingsReturnedInArray, ArrayOfThings can
be NULL and it will behave appropriately.  Returns FALSE if it runs out of
memory while making copies (will not return any values so you don't need to
worry about freeing partial allocations).  Returns TRUE in most other cases,
including when it doesn't find anything (*NumberOfValuesActuallyInTree will be
set to zero in that case). */

bool AVLDupFindAllValuesForKey (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer Key,
  uint32 ArraySizeInThings,
  AVLDupThingPointer ArrayOfThings,
  uint32 *NumberOfValuesActuallyInTree,
  uint32 *NumberOfThingsReturnedInArray);


/* Finds the smallest or largest key in the tree.  Useful for starting an
iteration through the tree.  Returns FALSE if the tree is empty. */

bool AVLDupFindSmallestKey (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer NewKey);

bool AVLDupFindLargestKey (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer NewKey);


/* Finds the next larger or smaller key when given a starting key.  Returns
TRUE if succesful, FALSE if it couldn't find the key. */

bool AVLDupFindNextLargerKey (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer OldKey,
  AVLDupThingPointer NewKey);

bool AVLDupFindNextSmallerKey (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer OldKey,
  AVLDupThingPointer NewKey);


/* Find all the keys between LowKey and HighKey, including the low and high
keys themselves.  Copies the resulting list of keys (in ascending order)
into your array of keys.  Copies of string keys are allocated too, so use
AVLDupFreeThingArray() when you are finished.  The pointers ArrayOfKeys and
NumberOfThingsReturnedInArray can be NULL if you wish.  A key with duplicate
values appears only once. */

bool AVLDupFindKeysInRange (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer LowKey,
  AVLDupThingPointer HighKey,
  uint32 ArraySizeInKeys,
  AVLDupThingPointer ArrayOfKeys,
  uint32 *NumberOfThingsReturnedInArray);
