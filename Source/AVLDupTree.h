/******************************************************************************
 * $Header: /boot/home/agmsmith/Programming/AVLTree/RCS/AVLDupTree.h,v 1.11 2001/03/28 22:16:58 agmsmith Exp $
 *
 * This set of C subroutines is useful for indexing a set of key/value pairs,
 * using the key to find a matching value.  As an enhancement, multiple values
 * for the same key are supported efficiently.
 *
 * See the AVLDupTree.c file for API descriptions, license, algorithm and
 * contact details.  Copyright © 2001 by Alexander G. M. Smith.
 *
 * $Log: AVLDupTree.h,v $
 * Revision 1.11  2001/03/28 22:16:58  agmsmith
 * Prepared for 1.0 release.
 *
 * Revision 1.10  2001/03/22 22:58:13  agmsmith
 * Added double bounded iteration function, compiles but not tested yet.
 *
 * Revision 1.9  2001/03/18 17:53:23  agmsmith
 * Added semaphore for multitasking safety.
 *
 * Revision 1.8  2001/03/17 21:09:01  agmsmith
 * Printed it out, read it over, corrected a few mistakes.
 *
 * Revision 1.7  2001/03/15 18:02:13  agmsmith
 * Removed duplicates tree, partially working now without it.
 *
 * Revision 1.6  2001/03/12 22:38:36  agmsmith
 * Just before removing duplicates subtree, but it does work a bit.
 *
 * Revision 1.5  2001/03/10 17:59:25  agmsmith
 * Under construction.
 *
 * Revision 1.4  2001/03/08 20:37:20  agmsmith
 * Moved big comments and private data declarations to the new AVLDupTree.c file.
 *
 * Revision 1.3  2001/03/06 19:05:49  agmsmith
 * Clean up the documentation after re-reading it the next day.
 *
 * Revision 1.2  2001/03/05 22:02:54  agmsmith
 * More function prototypes and data structures fleshed out.
 *
 * Revision 1.1  2001/03/04 22:50:44  agmsmith
 * Initial revision
 */

#ifndef _AVL_DUP_TREE_H
#define _AVL_DUP_TREE_H 1

#include <SupportDefs.h>

#ifdef __cplusplus
extern "C" {
#endif


/* AVLDupThing stores the keys and values.  The values are stored directly,
except for strings, which have separately allocated buffers if the string
won't fit directly.  It's 8 bytes long, so it can be set to zero by filling
the int64Thing field with zero (so don't make it longer without fixing up all
the code that assumes that). */

typedef union AVLDupThingUnion
{
  int32     int32Thing;
  int64     int64Thing;
  float     floatThing;
  double    doubleThing;

  char      shortStringThing [8]; /* For shorter strings, [7] set to zero. */
  struct LongStringStruct
  {
    char   *stringPntr; /* String longer than 7 bytes + NUL. */
    uint8   filler1;
    uint8   filler2;
    uint8   filler3;
    uint8   isLongString; /* Non-zero for long strings, 0 for short ones. */
  } longStringThing;

} AVLDupThingRecord, *AVLDupThingPointer;

typedef const AVLDupThingRecord *AVLDupThingConstPointer;


/* A macro for getting a string pointer from a string thing.  Handles both
long and short string formats. */

#define AVLDupGetStringPntrFromThing(Thing) \
  ((Thing).longStringThing.isLongString \
  ? (Thing).longStringThing.stringPntr \
  : &(Thing).shortStringThing[0])  /* Added the &[0] to avoid type problems. */


/* Memory allocation support functions for dealing with things.  Strictly
speaking you only need them for string things. */

bool AVLDupCopyThingArray (
  AVLDupThingPointer DestThingPntr,
  AVLDupThingPointer SourceThingPntr,
  type_code ThingType,
  int NumberOfThings);

void AVLDupFreeThingArray (
  AVLDupThingPointer ThingPntr,
  type_code ThingType,
  int NumberOfThings);

/* These convert between human readable strings and the binary formats. */

bool AVLDupConvertStringToThing (
  const char *StringPntr,
  AVLDupThingPointer ThingPntr,
  type_code ThingType);

bool AVLDupConvertThingToString (
  AVLDupThingPointer ThingPntr,
  type_code ThingType,
  char *StringPntr,
  int StringSize);


/* An opaque (to end users) reference to the AVLDupTree itself. */

typedef struct AVLDupTreeStruct AVLDupTreeRecord, *AVLDupTreePointer;


/* The various functions for operating on an AVLDupTree.  See the AVLDupTree.c
file for details on what they do and an explanation of the algorithms used. */

AVLDupTreePointer AVLDupAllocTree (
  type_code KeyType,
  type_code ValueType,
  const char *IndexName,
  uint32 MaxSimultaneousReaders);

void AVLDupFreeTree (AVLDupTreePointer TreePntr);

unsigned int AVLDupGetTreeCount (AVLDupTreePointer TreePntr);

const char *AVLDupGetTreeName (AVLDupTreePointer TreePntr);

bool AVLDupAdd (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer Key,
  AVLDupThingPointer Value);

bool AVLDupDelete (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer Key,
  AVLDupThingPointer Value);

typedef bool (* AVLDupIterationCallbackFunctionPointer) (
  AVLDupThingConstPointer KeyPntr,
  AVLDupThingConstPointer ValuePntr,
  void *ExtraData);

bool AVLDupIterate (
  AVLDupTreePointer TreePntr,
  AVLDupThingPointer StartKeyPntr,
  AVLDupThingPointer StartValuePntr,
  bool IncludeThingEqualToStart,
  AVLDupThingPointer EndKeyPntr,
  AVLDupThingPointer EndValuePntr,
  bool IncludeThingEqualToEnd,
  AVLDupIterationCallbackFunctionPointer CallbackFunctionPntr,
  void *ExtraUserData);

#ifdef __cplusplus
}
#endif

#endif /* _AVL_DUP_TREE_H */
