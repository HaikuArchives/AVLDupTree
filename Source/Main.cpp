/******************************************************************************
 * $Header: /boot/home/agmsmith/Programming/AVLTree/RCS/Main.cpp,v 1.17 2001/03/28 22:16:23 agmsmith Exp $
 *
 * This program tests my AVL tree (G. M. Adel'son-Velskii and Y.M. Landis wrote
 * about it first in 1962 in Soviet Math Dockl., titled "An Algorithm for the
 * Organization of Information") implementation.  A pretty GUI lets you see
 * the tree as you try out various operations on it.
 *
 * This testing program (does not include the tree library code, just this
 * Main.cpp file) is released to the public domain, by Alexander G. M. Smith,
 * March 2001.
 *
 * $Log: Main.cpp,v $
 * Revision 1.17  2001/03/28 22:16:23  agmsmith
 * Prepared for 1.0 release.
 *
 * Revision 1.16  2001/03/26 21:57:25  agmsmith
 * Update pop-up menus with new data types after the data types
 * are changed during a test run.
 *
 * Revision 1.15  2001/03/23 02:52:48  agmsmith
 * Added functionality tests and tested iteration etc.
 *
 * Revision 1.14  2001/03/19 02:49:48  agmsmith
 * *** empty log message ***
 *
 * Revision 1.13  2001/03/19 02:26:11  agmsmith
 * Added speed test.
 *
 * Revision 1.12  2001/03/18 18:31:55  agmsmith
 * The Add Lots button now displays each step of the addition.
 *
 * Revision 1.11  2001/03/18 17:53:23  agmsmith
 * Added semaphore for multitasking safety.
 *
 * Revision 1.10  2001/03/18 00:27:38  agmsmith
 * *** empty log message ***
 *
 * Revision 1.9  2001/03/16 22:30:13  agmsmith
 * Added delete button action.
 *
 * Revision 1.8  2001/03/15 18:02:13  agmsmith
 * Removed duplicates tree, partially working now without it.
 *
 * Revision 1.7  2001/03/12 22:39:06  agmsmith
 * Just before removing duplicates subtree, but it does work a bit.
 *
 * Revision 1.6  2001/03/11 04:40:59  agmsmith
 * Added type selection pop-up menus, and started scrolling tree view.
 *
 * Revision 1.5  2001/03/08 20:38:00  agmsmith
 * Added global tree allocation and deallocation.
 *
 * Revision 1.4  2001/03/04 22:50:29  agmsmith
 * Removed test code.
 *
 * Revision 1.3  2001/03/04 18:53:34  agmsmith
 * Added a colourful background, and some test code to see what file attributes
 * and indices use as data types (indices use a subset of primitive types
 * only).
 *
 * Revision 1.2  2001/02/28 23:38:45  agmsmith
 * Added window and buttons.
 *
 * Revision 1.1  2001/02/28 19:08:49  agmsmith
 * Initial revision
 */

/* BeOS headers. */

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

/* Posix headers. */

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* My AVL Tree headers. */

#include "AVLDupTree.h"


/* Copies of the node and tree structure definitions, normally they are secret
but we want to display the secret values and do our own traversal to help with
the debugging.  So, if they get changed in AVLDupTree.c, change these too. */

typedef struct AVLDupNodeStruct AVLDupNodeRecord, *AVLDupNodePointer;

struct AVLDupNodeStruct
{
  AVLDupThingRecord key;
  AVLDupThingRecord value;
  AVLDupNodePointer smallerChildPntr;
  AVLDupNodePointer largerChildPntr;
  unsigned int      height; /* Only needs to be uint8, rest is for padding. */
};

struct AVLDupTreeStruct
{
  char             *indexName;
  type_code         keyType;
  void             *keyComparisonFunctionPntr;
  type_code         valueType;
  void             *valueComparisonFunctionPntr;
  AVLDupNodePointer rootPntr;
  unsigned int      count;
};



/******************************************************************************
 * Some globals.
 */

static float g_MarginBetweenControls; /* Space of a letter "M" between them. */
static float g_LineOfTextHeight;      /* Height of text the current font. */
static float g_ButtonHeight;          /* How many pixels tall buttons are. */
static float g_CheckBoxHeight;        /* Same for check boxes. */
static float g_RadioButtonHeight;     /* Also for radio buttons. */
static float g_PopUpMenuHeight;       /* Again for pop-up menus. */
static float g_TextBoxHeight;         /* Ditto for text controls. */


AVLDupTreePointer g_TheTree; /* The tree we are displaying is stored here. */
type_code g_TypeForKeys = B_DOUBLE_TYPE; /* Use double for initial types so */
type_code g_TypeForValues = B_DOUBLE_TYPE; /* longest string sets menu size. */
AVLDupThingRecord g_KeyThing; /* Key corresponding to the text entry box. */
AVLDupThingRecord g_ValueThing; /* Another corresponding value. */

const int MAX_TYPE_NAMES = 5;
const char TypeCodeMsgIDString [] = "TypeCode";

static const char *g_TypeNames [MAX_TYPE_NAMES] =
{
  "B_INT32_TYPE",
  "B_INT64_TYPE",
  "B_FLOAT_TYPE",
  "B_DOUBLE_TYPE",
  "B_STRING_TYPE"
};

static type_code g_TypeCodes [MAX_TYPE_NAMES] =
{
  B_INT32_TYPE,
  B_INT64_TYPE,
  B_FLOAT_TYPE,
  B_DOUBLE_TYPE,
  B_STRING_TYPE
};


/* Various message codes used by buttons etc, targetted mostly for the
window, but some are for the App object. */

typedef enum MessageCodesEnum
{
  MSG_KEY_TYPE = 'KeyT',
  MSG_VALUE_TYPE = 'ValT',
  MSG_ADD = 'Add ',
  MSG_DELETE = 'Dele',
  MSG_ADD_LOTS = 'Alot',
  MSG_TEST_SPEED = 'Sped',
  MSG_TEST_FUNCTIONALITY = 'Test',
} MessageCodes;




/******************************************************************************
 * Global utility function to display an error message and return.  The message
 * part describes the error, and if ErrorNumber is non-zero, gets the string
 * ", error code $X (standard description)." appended to it.  If the message
 * is NULL then it gets defaulted to "Something went wrong".  The title part
 * doesn't get displayed (no title bar in the dialog box, but you can see it
 * in the debugger as the window thread name), and defaults to "Error Message"
 * if you didn't specify one.
 */

static void DisplayErrorMessage (
  const char *MessageString = NULL,
  int ErrorNumber = 0,
  const char *TitleString = NULL)
{
  BAlert *AlertPntr;
  char ErrorBuffer [B_PATH_NAME_LENGTH + 80 /* error message */ + 80];

  if (TitleString == NULL)
    TitleString = "Error Message";

  if (MessageString == NULL)
  {
    if (ErrorNumber == 0)
      MessageString = "No error, no message, why bother?";
    else
      MessageString = "Something went wrong";
  }

  if (ErrorNumber != 0)
  {
    sprintf (ErrorBuffer, "%s, error code $%X/%d (%s) has occured.",
      MessageString, ErrorNumber, ErrorNumber, strerror (ErrorNumber));
    MessageString = ErrorBuffer;
  }

  AlertPntr = new BAlert (TitleString, MessageString,
    "Acknowledge", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
  if (AlertPntr != NULL)
    AlertPntr->Go ();
}



#if 0 /* Unused code. */
/******************************************************************************
 * Global utility function to convert a 32 bit number into a 4 character
 * string so that you can see printable message codes etc.
 */

static const char *U32toString (uint32 Number)
{
  union ConverterUnion
  {
    uint32 number;
    char string [4];
  } Converter;

  static char OutputBuffer [5];

  Converter.number = Number;

  OutputBuffer[0] = Converter.string[3];
  OutputBuffer[1] = Converter.string[2];
  OutputBuffer[2] = Converter.string[1];
  OutputBuffer[3] = Converter.string[0];
  OutputBuffer[4] = 0;

  return OutputBuffer;
}
#endif /* Unused code. */



/******************************************************************************
 * Extract the Thing type from the "TypeCode" field of the message.
 */

static type_code GetThingTypeFromMessage (BMessage *MessagePntr)
{
  int32 ReturnValue = 0;

  MessagePntr->FindInt32 (TypeCodeMsgIDString, &ReturnValue);
  return ReturnValue;
}



/******************************************************************************
 * This view just draws a solid colour background (since we can't change
 * the window's background), and has all the control views as children
 * (all subviews except the tree display view and scrollbars).
 */

class BackgroundColourView : public BView
{
public:
  int red, green, blue;
  int dred, dgreen, dblue;

  BackgroundColourView (BRect NewBounds)
  : BView (NewBounds, "Background View", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
    B_WILL_DRAW | B_PULSE_NEEDED)
  {
    red = green = blue = 255;
    dred = 1; dgreen = 2; dblue = 3;
  };

  virtual void Pulse ();
};


void BackgroundColourView::Pulse ()
{
  BView *ChildViewPntr;
  int    i;

  red += dred; green += dgreen; blue += dblue;
  // Note range 159-255 has 97 possible values, a prime number, so it won't
  // repeat the colours for a long time.  Also, 159-255 is bright enough to
  // keep black text readable.
  if (red < 159 || red > 255) {dred = -dred; red += dred;};
  if (green < 159 || green > 255) {dgreen = -dgreen; green += dgreen;};
  if (blue < 159 || blue > 255) {dblue = -dblue; blue += dblue;};

  SetViewColor (red, green, blue);
  Invalidate ();

  for (i = 0; (ChildViewPntr = ChildAt (i)) != NULL; i++)
  {
    ChildViewPntr->SetViewColor (red, green, blue);
    ChildViewPntr->Invalidate ();
  }
}



/******************************************************************************
 * This view subclass displays the g_TheTree tree.  It's usually inside a
 * scroll view so that you can scroll around the larger tree.  The size adapts
 * to the tree contents.
 */

typedef struct LevelItemStruct LevelItemRecord, *LevelItemPointer;

struct LevelItemStruct
{
  AVLDupNodePointer nodePntr;
  BPoint            location;
  BPoint            parentAttachPoint;
  LevelItemPointer  next;
};

typedef struct LevelListStruct
{
  LevelItemPointer head;
  LevelItemPointer tail;
  int              count;
  float            gapBetweenBoxesPixelSize;
  int              currentIndex;
} LevelListRecord, *LevelListPointer;


class AVLTreeView : public BView
{
public:
  float        m_BoxPixelHeight;
  float        m_BoxPixelWidth;
  float        m_BoxVerticalGapPixelSize;
  BScrollView *m_ScrollViewPntr;
  float        m_TreePixelHeight;
  float        m_TreePixelWidth;

  static const int MAX_LEVELS = 50;
  LevelListRecord  m_LevelLists [MAX_LEVELS];
  int              m_NumberOfLevels;

  AVLTreeView (BRect NewBounds);
  virtual ~AVLTreeView ();

  virtual void Draw (BRect UpdateRect);
  virtual void FrameResized (float Width, float Height);
  virtual void TargetedByScrollView (BScrollView *ScrollViewPntr);

  void BuildLevelLists ();
  void DeallocateLevelLists ();
  void RecursivelyCountNodesForLevelList (AVLDupNodePointer CurrentNodePntr,
    int CurrentLevel);
  bool RecursivelyPositionLevelListItems (AVLDupNodePointer CurrentNodePntr,
    int CurrentLevel, BPoint ParentAttachmentPoint);
  void ResizeScrollBars ();
  void TreeHasChanged ();
};


AVLTreeView::AVLTreeView (BRect NewBounds)
: BView (NewBounds, "AVLTreeView", B_FOLLOW_ALL_SIDES,
  B_WILL_DRAW | B_FRAME_EVENTS)
{
  m_TreePixelHeight = 400;
  m_TreePixelWidth = 600;
  m_BoxPixelHeight = 3 * g_LineOfTextHeight + 4;
  m_BoxPixelWidth = be_plain_font->StringWidth ("MMValue: 123456789MM");
  m_BoxVerticalGapPixelSize = 15;
  m_ScrollViewPntr = NULL;
  SetViewColor (230, 230, 255);
  memset (&m_LevelLists, 0, sizeof (m_LevelLists));
  m_NumberOfLevels = 0;
};


AVLTreeView::~AVLTreeView ()
{
  DeallocateLevelLists ();
}


void AVLTreeView::Draw (BRect UpdateRect)
{
  BRect             BoxRect;
  BPoint            DotPoint;
  int               i;
  LevelItemPointer  ItemPntr;
  int               Length;
  AVLDupNodePointer NodePntr;
  char              TempString [1024];

  SetLowColor (255, 255, 0);
  SetPenSize (2);

  for (i = 0; i < m_NumberOfLevels; i++)
  {
    for (ItemPntr = m_LevelLists[i].head;
    ItemPntr != NULL; ItemPntr = ItemPntr->next)
    {
      NodePntr = ItemPntr->nodePntr;

      /* Draw a solid colour box with a black outline for the node. */

      BoxRect.left = ItemPntr->location.x;
      BoxRect.right = BoxRect.left + m_BoxPixelWidth;
      BoxRect.top = ItemPntr->location.y;
      BoxRect.bottom = BoxRect.top + m_BoxPixelHeight;
      FillRect (BoxRect, B_SOLID_LOW);
      StrokeRect (BoxRect);

      /* Draw dots for the child attachment points. */

      DotPoint.x = ItemPntr->location.x + m_BoxPixelWidth / 2;
      DotPoint.y = ItemPntr->location.y;
      FillEllipse (DotPoint, 2, 2);

      DotPoint.x = ItemPntr->location.x + m_BoxPixelWidth / 3;
      DotPoint.y = ItemPntr->location.y + m_BoxPixelHeight;
      FillEllipse (DotPoint, 2, 2);

      DotPoint.x += m_BoxPixelWidth / 3;
      FillEllipse (DotPoint, 2, 2);

      /* Draw the line to the parent node. */

      MovePenTo (ItemPntr->location.x + m_BoxPixelWidth / 2,
        ItemPntr->location.y);
      StrokeLine (ItemPntr->parentAttachPoint);

      /* Draw the text inside the box.  Key first. */

      MovePenTo (ItemPntr->location.x + 5,
        ItemPntr->location.y + g_LineOfTextHeight);

      strcpy (TempString, "Key: ");
      Length = strlen (TempString);
      AVLDupConvertThingToString (&NodePntr->key, g_TypeForKeys,
        TempString + Length, sizeof (TempString) - Length);
      DrawString (TempString);

      /* Draw the value field. */

      MovePenTo (ItemPntr->location.x + 5,
        ItemPntr->location.y + 2 * g_LineOfTextHeight);

      strcpy (TempString, "Value: ");
      Length = strlen (TempString);
      AVLDupConvertThingToString (&NodePntr->value, g_TypeForValues,
        TempString + Length, sizeof (TempString) - Length);
      DrawString (TempString);

      /* Draw the height field. */

      MovePenTo (ItemPntr->location.x + 5,
        ItemPntr->location.y + 3 * g_LineOfTextHeight);

      sprintf (TempString, "Height: %d", (int) ItemPntr->nodePntr->height);
      DrawString (TempString);
    }
  }
}


void AVLTreeView::FrameResized (float Width, float Height)
{
  ResizeScrollBars ();
}


void AVLTreeView::TargetedByScrollView (BScrollView *ScrollViewPntr)
{
  m_ScrollViewPntr = ScrollViewPntr;
}


/* Make the level lists from the current tree.  All nodes at the same depth in
the AVLDupTree get added to the level list for that depth, in increasing key
order (by doing a recursive traversal).  Later the drawing routine will just
iterate through the rows, drawing boxes for the nodes of the tree. */

void AVLTreeView::BuildLevelLists ()
{
  BPoint AttachmentPoint;
  int    i;
  int    LargestCount;

  DeallocateLevelLists ();

  if (g_TheTree == NULL)
    return; /* Nothing to do, not even an empty tree. */

  /* Count the number of nodes at each level. */

  RecursivelyCountNodesForLevelList (g_TheTree->rootPntr, 0 /* level */);

  /* Find the widest level and size the tree to match. */

  LargestCount = 0;

  for (i = 0; i < m_NumberOfLevels; i++)
  {
    if (m_LevelLists[i].count > LargestCount)
      LargestCount = m_LevelLists[i].count;
  }

  m_TreePixelHeight = m_NumberOfLevels *
    (m_BoxPixelHeight + m_BoxVerticalGapPixelSize);
  if (m_TreePixelHeight < 1) m_TreePixelHeight = 1;

  m_TreePixelWidth = LargestCount * (m_BoxPixelWidth + 10);
  if (m_TreePixelWidth < 1) m_TreePixelWidth = 1;

  /* Adjust the gap sizes of all levels to space out the items in each
  level evenly.  Half a gap is provided on the left and right of the whole
  level, and there is a full gap size between boxes. */

  for (i = 0; i < m_NumberOfLevels; i++)
  {
    m_LevelLists[i].gapBetweenBoxesPixelSize =
      (m_TreePixelWidth - (m_LevelLists[i].count * m_BoxPixelWidth)) /
      m_LevelLists[i].count;
  }

  /* Now place the nodes in the tree space we have calculated. */

  AttachmentPoint.x = m_TreePixelWidth / 2;
  AttachmentPoint.y = 0;

  if (!RecursivelyPositionLevelListItems (g_TheTree->rootPntr, 0 /* level */,
  AttachmentPoint))
    DisplayErrorMessage ("Ran out of memory while preparing to display tree!");
}


void AVLTreeView::DeallocateLevelLists ()
{
  LevelItemPointer FreePntr;
  int              i;
  LevelItemPointer ItemPntr;

  m_NumberOfLevels = 0;

  for (i = 0; i < MAX_LEVELS; i++)
  {
    ItemPntr = m_LevelLists[i].head;
    while (ItemPntr != NULL)
    {
      FreePntr = ItemPntr;
      ItemPntr = ItemPntr->next;
      memset (FreePntr, 0, sizeof (LevelItemRecord));
      free (FreePntr);
    }
    m_LevelLists[i].head = NULL;
    m_LevelLists[i].tail = NULL;
    m_LevelLists[i].count = 0;
    m_LevelLists[i].gapBetweenBoxesPixelSize = 0;
    m_LevelLists[i].currentIndex = 0;
  }
}


void AVLTreeView::ResizeScrollBars ()
{
  float       Height;
  BScrollBar *ScrollBarPntr;
  float       Width;

  Height = Bounds().Height();
  Width = Bounds().Width();

  if (m_ScrollViewPntr == NULL)
    return;

  ScrollBarPntr = m_ScrollViewPntr->ScrollBar (B_HORIZONTAL);
  if (ScrollBarPntr != NULL)
  {
    if (m_TreePixelWidth < Width)
      ScrollBarPntr->SetRange (0.0, 0.0); // Fully fits, turn off scroll bar.
    else
    {
      ScrollBarPntr->SetRange (0.0, m_TreePixelWidth - Width);
      ScrollBarPntr->SetProportion (Width / m_TreePixelWidth);
      ScrollBarPntr->SetSteps (1.0, Width / 2);
    }
  }

  ScrollBarPntr = m_ScrollViewPntr->ScrollBar (B_VERTICAL);
  if (ScrollBarPntr != NULL)
  {
    if (m_TreePixelHeight < Height)
      ScrollBarPntr->SetRange (0.0, 0.0); // Fully fits, turn off scroll bar.
    else
    {
      ScrollBarPntr->SetRange (0.0, m_TreePixelHeight - Height);
      ScrollBarPntr->SetProportion (Height / m_TreePixelHeight);
      ScrollBarPntr->SetSteps (1.0, Height / 2);
    }
  }
}


/* Internal recursive routine for counting the nodes at each level and finding
out how many levels there are. */

void AVLTreeView::RecursivelyCountNodesForLevelList (
  AVLDupNodePointer CurrentNodePntr, int CurrentLevel)
{
  if (CurrentNodePntr == NULL || CurrentLevel >= MAX_LEVELS)
    return; /* Nothing to do. */

  /* Visit the smaller children of the node. */

  RecursivelyCountNodesForLevelList (CurrentNodePntr->smallerChildPntr,
    CurrentLevel + 1);

  /* Count this node. */

  m_LevelLists[CurrentLevel].count++;

  if (m_NumberOfLevels <= CurrentLevel)
    m_NumberOfLevels = CurrentLevel + 1;

  /* Visit the larger children of the node. */

  RecursivelyCountNodesForLevelList (CurrentNodePntr->largerChildPntr,
    CurrentLevel + 1);
}


/* Internal recursive routine for allocating list items and calculating the
positions of the items in the level lists.  Returns FALSE if it runs out
of memory. */

bool AVLTreeView::RecursivelyPositionLevelListItems (
  AVLDupNodePointer CurrentNodePntr, int CurrentLevel,
  BPoint ParentAttachmentPoint)
{
  BPoint           AttachLocation;
  BPoint           CurrentLocation;
  LevelListPointer LevelListPntr;
  LevelItemPointer NewItemPntr;

  if (CurrentNodePntr == NULL || CurrentLevel >= MAX_LEVELS)
    return true; /* Nothing to do. */

  /* Find the position of the top left corner of this item on the display. */

  LevelListPntr = m_LevelLists + CurrentLevel;

  CurrentLocation.x = LevelListPntr->gapBetweenBoxesPixelSize / 2 +
    LevelListPntr->currentIndex *
    (m_BoxPixelWidth + LevelListPntr->gapBetweenBoxesPixelSize);
  CurrentLocation.y = m_BoxVerticalGapPixelSize / 2 +
    CurrentLevel * (m_BoxPixelHeight + m_BoxVerticalGapPixelSize);

  LevelListPntr->currentIndex++;

  /* Visit the smaller children of the node.  They will attach to the lower
  edge of our box, near the left side. */

  AttachLocation = CurrentLocation;
  AttachLocation.x += m_BoxPixelWidth / 3;
  AttachLocation.y += m_BoxPixelHeight;

  if (!RecursivelyPositionLevelListItems (CurrentNodePntr->smallerChildPntr,
  CurrentLevel + 1, AttachLocation))
    return false;

  /* Add this node to the level list items. */

  NewItemPntr = (LevelItemPointer) malloc (sizeof (LevelItemRecord));
  if (NewItemPntr == NULL)
    return false; /* Out of memory. */

  NewItemPntr->nodePntr = CurrentNodePntr;
  NewItemPntr->next = NULL;
  NewItemPntr->location = CurrentLocation;
  NewItemPntr->parentAttachPoint = ParentAttachmentPoint;

  if (LevelListPntr->head == NULL)
  {
    LevelListPntr->head = NewItemPntr;
    LevelListPntr->tail = NewItemPntr;
  }
  else /* Add to the tail of the list. */
  {
    LevelListPntr->tail->next = NewItemPntr;
    LevelListPntr->tail = NewItemPntr;
  }

  /* Visit the larger children of the node.  Attach to bottom right of box. */

  AttachLocation.x += m_BoxPixelWidth / 3;

  if (!RecursivelyPositionLevelListItems (CurrentNodePntr->largerChildPntr,
  CurrentLevel + 1, AttachLocation))
    return false;

  return true;
}


/* The tree has been changed.  Rebuild the level lists (needed for drawing it),
recalculate the pixel size of the tree, resize the scroll bars, and request a
redraw. */

void AVLTreeView::TreeHasChanged ()
{
  BuildLevelLists ();
  ResizeScrollBars ();
  Invalidate ();
}



/******************************************************************************
 * The window contains all the controls and a subview which contains the
 * scrolling picture of the tree.  Messages from the controls are sent
 * to the window's thread, so that tree operations and drawing are done
 * by the same thread, avoiding multitasking complexity (and all that
 * looper locking and unlocking).
 */

class AVLTestWindow : public BWindow
{
public:
  AVLTestWindow ();
  virtual bool QuitRequested ();
  virtual void MessageReceived (BMessage *MessagePntr);

  void AddLotsOfNodes ();
  void AddNewKeyAndValue ();
  void CopyGlobalTypesToPopupMenus ();
  void DeleteKeyAndValue ();
  void FreeKeyAndValue ();
  void GetKeyAndValueFromUser ();
  bool InitializeContents ();
  void TestFunctionality ();
  void TestSpeed ();
  void TreeHasChanged ();

  /* Pointers to important subviews, so that their contents can be updated. */

  AVLTreeView  *m_AVLTreeViewPntr;
  BTextControl *m_CountViewPntr;
  BPopUpMenu   *m_KeyPopupMenuPntr;
  BTextControl *m_KeyViewPntr;
  BPopUpMenu   *m_ValuePopupMenuPntr;
  BTextControl *m_ValueViewPntr;
};


AVLTestWindow::AVLTestWindow ()
: BWindow (BRect (30, 30, 600, 300), "AVL Duplicates Tree Demo by AGMSmith",
    B_DOCUMENT_WINDOW, 0),
  m_AVLTreeViewPntr (NULL),
  m_CountViewPntr (NULL),
  m_KeyPopupMenuPntr (NULL),
  m_KeyViewPntr (NULL),
  m_ValuePopupMenuPntr (NULL),
  m_ValueViewPntr (NULL)
{
}


void AVLTestWindow::MessageReceived (BMessage *MessagePntr)
{
//  printf ("\nWindow received message %s.\n",
//    U32toString (MessagePntr->what));
//  MessagePntr->PrintToStream ();

  switch (MessagePntr->what)
  {
    case MSG_ADD:
      AddNewKeyAndValue ();
      break;

    case MSG_DELETE:
      DeleteKeyAndValue ();
      break;

    case MSG_KEY_TYPE:
      AVLDupFreeTree (g_TheTree);
      g_TypeForKeys = GetThingTypeFromMessage (MessagePntr);
      g_TheTree = AVLDupAllocTree (g_TypeForKeys, g_TypeForValues,
        "The Global Tree", 1);
      TreeHasChanged ();
      break;

    case MSG_TEST_FUNCTIONALITY:
      TestFunctionality ();
      break;

    case MSG_TEST_SPEED:
      TestSpeed ();
      break;

    case MSG_VALUE_TYPE:
      AVLDupFreeTree (g_TheTree);
      g_TypeForValues = GetThingTypeFromMessage (MessagePntr);
      g_TheTree = AVLDupAllocTree (g_TypeForKeys, g_TypeForValues,
        "The Global Tree", 1);
      TreeHasChanged ();
      break;

    default:
      BWindow::MessageReceived (MessagePntr);
      break;
  }
}


/* Make the whole application quit when the window is closed. */

bool AVLTestWindow::QuitRequested ()
{
  be_app->PostMessage (B_QUIT_REQUESTED);
  return true;
}


/* Copy the current data types from g_TypeForKeys and g_TypeForValues into
the popup menus - changing their current selection to match the new types. */

void AVLTestWindow::CopyGlobalTypesToPopupMenus ()
{
  int         i, j;
  BMenuItem  *ItemPntr;
  BPopUpMenu *PopPntr;
  type_code   TypeCode;

  for (i = 1; i <= 2; i++)
  {
    if (i == 1)
    {
      PopPntr = m_KeyPopupMenuPntr;
      TypeCode = g_TypeForKeys;
    }
    else
    {
      PopPntr = m_ValuePopupMenuPntr;
      TypeCode = g_TypeForValues;
    }

    if (PopPntr == NULL)
      continue; /* This menu doesn't exist yet. */

    for (j = 0; j < MAX_TYPE_NAMES; j++)
      if (g_TypeCodes[j] == TypeCode)
        break; /* Exit loop when we find a code we know. */

    if (j >= MAX_TYPE_NAMES)
      continue; /* Unknown type code, do nothing for this popup menu. */

    ItemPntr = PopPntr->FindItem (g_TypeNames [j]);
    if (ItemPntr == NULL)
      continue; /* Hmmm, something wrong with menu's label strings. */

    /* Finally, turn on the menu item matching the type code. */

    ItemPntr->SetMarked (true);
  }
}


/* Stress test things by adding lots of nodes.  Also display the tree as it
is growing. */

void AVLTestWindow::AddLotsOfNodes ()
{
  int               i;
  AVLDupThingRecord Key;
  char              TempString [1024];
  AVLDupThingRecord Value;

  Key.int64Thing = 0;
  Value.int64Thing = 0;

  for (i = 0; i < 1024 * 16 - 1; i++)
  {
    sprintf (TempString, "%d is the key.", i);
    if (!AVLDupConvertStringToThing (TempString, &Key, g_TypeForKeys))
      break; /* Ran out of memory. */

    sprintf (TempString, "%d is the value.", i);
    if (!AVLDupConvertStringToThing (TempString, &Value, g_TypeForValues))
    {
      AVLDupFreeThingArray (&Key, g_TypeForKeys, 1);
      break; /* Ran out of memory. */
    }

    if (!AVLDupAdd (g_TheTree, &Key, &Value))
      break; /* The usual. */

    AVLDupFreeThingArray (&Key, g_TypeForKeys, 1);
    AVLDupFreeThingArray (&Value, g_TypeForValues, 1);
  }

  TreeHasChanged ();
}


/* Read the key and value out of the text boxes, convert into Things, try to
add them to the tree, and redraw the tree if successful. */

void AVLTestWindow::AddNewKeyAndValue ()
{
  GetKeyAndValueFromUser ();

  /* Add the key/value to the tree, and redraw it. */

  if (!AVLDupAdd (g_TheTree, &g_KeyThing, &g_ValueThing))
    DisplayErrorMessage ("AVLDupAdd failed!");

  TreeHasChanged ();

  FreeKeyAndValue ();
}


/* Read the key and value out of the text boxes, convert into Things, try to
delete them from the tree, and redraw the tree if successful. */

void AVLTestWindow::DeleteKeyAndValue ()
{
  GetKeyAndValueFromUser ();

  /* Delete the key/value from the tree, and redraw it. */

  AVLDupDelete (g_TheTree, &g_KeyThing, &g_ValueThing);

  TreeHasChanged ();

  FreeKeyAndValue ();
}


/* Deallocate storage allocated by GetKeyAndValueFromUser. */

void AVLTestWindow::FreeKeyAndValue ()
{
  AVLDupFreeThingArray (&g_KeyThing, g_TypeForKeys, 1);
  AVLDupFreeThingArray (&g_ValueThing, g_TypeForValues, 1);
}


/* Read the strings from the key and value controls into the global variables,
and set the controls back to the new values, so that the user knows how their
text was parsed.  You must call FreeKeyAndValue after using this, before
processing the next message, which may change the global datatypes. */

void AVLTestWindow::GetKeyAndValueFromUser ()
{
  const char       *KeyString;
  char              TempString [1024];
  const char       *ValueString;

  /* Read the key and value out of the text boxes. */

  if (m_KeyViewPntr == NULL || m_ValueViewPntr == NULL ||
  m_AVLTreeViewPntr == NULL)
    return; /* Not initialised yet. */

  KeyString = m_KeyViewPntr->Text ();
  ValueString = m_ValueViewPntr->Text ();

  if (KeyString == NULL || ValueString == NULL)
    return;

  /* Convert the key and value into their binary Thing representation. */

  if (!AVLDupConvertStringToThing (KeyString, &g_KeyThing, g_TypeForKeys))
    return;

  if (!AVLDupConvertStringToThing (ValueString, &g_ValueThing, g_TypeForValues))
  {
    AVLDupFreeThingArray (&g_KeyThing, g_TypeForKeys, 1);
    return;
  }

  /* Convert back to text and update the key and value displays, so that the
  user knows exactly what key and value were used. */

  if (AVLDupConvertThingToString (&g_KeyThing, g_TypeForKeys,
  TempString, sizeof (TempString)))
    m_KeyViewPntr->SetText (TempString);

  if (AVLDupConvertThingToString (&g_ValueThing, g_TypeForValues,
  TempString, sizeof (TempString)))
    m_ValueViewPntr->SetText (TempString);
}


/* Create the child views and controls, return true if successful.  Keep in
mind that the window is currently visible and running, so things that get
added get displayed and activated right away, even before this function
returns (since this is in the App thread and the Window thread is running). */

bool AVLTestWindow::InitializeContents ()
{
  BackgroundColourView *BackgroundViewPntr;
  BButton              *ButtonPntr;
  float                 HeightUsedByControls;
  int                   i;
  float                 MarginAboveControl;
  BMenuBar             *MenuBarPntr;
  BMenuItem            *MenuItemPntr;
  BMessage             *MsgPntr;
  BPopUpMenu           *PopUpMenuPntr;
  float                 RowBottomY;
  BScrollView          *ScrollPntr;
  BStringView          *StringViewPntr;
  BRect                 TempRect;
  BTextControl         *TextControlPntr;
  float                 WidthOfButtons;

  /* Create and add the background colour view, which also holds all the
     other views (except the actual tree display) as children.  It fills
     the top part of the window. */

  TempRect = Bounds ();
  RowBottomY = TempRect.top;
  TempRect.bottom = TempRect.top +
    ceilf (((g_TextBoxHeight > g_PopUpMenuHeight) ? /* height of row with */
    g_TextBoxHeight : g_PopUpMenuHeight) * 1.2) /* key/value menus */ +
    ceilf (g_ButtonHeight * 1.4) /* height of row with control buttons */ +
    ceilf (g_TextBoxHeight * 1.2) /* height of row with key/value text */;

  BackgroundViewPntr = new BackgroundColourView (TempRect);
  if (BackgroundViewPntr == NULL) goto ErrorExit;
  AddChild (BackgroundViewPntr);

  /* Set up for the row with the key and value data type pop-up menus
     and a box for the count of the number of key/value pairs. */

  TempRect = Bounds ();
  HeightUsedByControls = g_TextBoxHeight;
  if (g_PopUpMenuHeight > HeightUsedByControls)
    HeightUsedByControls = g_PopUpMenuHeight;
  HeightUsedByControls = ceilf (HeightUsedByControls * 1.2);

  /* Add the "Key:" title string control. */

  MarginAboveControl = ceilf ((HeightUsedByControls - g_LineOfTextHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_LineOfTextHeight;
  TempRect.left += g_MarginBetweenControls;
  TempRect.right = TempRect.left + 500 /* arbitrary, gets resized later */;

  StringViewPntr = new BStringView (TempRect, "Key BStringView", "Key:");
  if (StringViewPntr == NULL) goto ErrorExit;
  StringViewPntr->ResizeToPreferred ();

  BackgroundViewPntr->AddChild (StringViewPntr);

  Lock ();
  TempRect.left = StringViewPntr->Frame().right + g_MarginBetweenControls;
  Unlock ();

  /* Add the pop-up menu for selecting the key's data type. */

  MarginAboveControl = ceilf ((HeightUsedByControls - g_PopUpMenuHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_PopUpMenuHeight;
  TempRect.right = TempRect.left + 500 /* arbitrary, gets resized later */;

  MenuBarPntr = new BMenuBar (TempRect, "Key Type MenuBar",
    B_FOLLOW_LEFT | B_FOLLOW_TOP, B_ITEMS_IN_COLUMN,
    false /* true resize to fit currently marked item */);
  if (MenuBarPntr == NULL) goto ErrorExit;

  PopUpMenuPntr = new BPopUpMenu ("Key Type PopUpMenu",
    true /* radio mode */, true /* take label from marked item */);
  if (PopUpMenuPntr == NULL) goto ErrorExit;

  for (i = 0; i < MAX_TYPE_NAMES; i++)
  {
    MsgPntr = new BMessage (MSG_KEY_TYPE);
    if (MsgPntr == NULL) goto ErrorExit;
    MsgPntr->AddInt32 (TypeCodeMsgIDString, g_TypeCodes [i]);
    MenuItemPntr = new BMenuItem (g_TypeNames [i], MsgPntr);
    if (MenuItemPntr == NULL) goto ErrorExit;
    if (g_TypeCodes [i] == g_TypeForKeys)
      MenuItemPntr->SetMarked (true);
    PopUpMenuPntr->AddItem (MenuItemPntr);
  }

  MenuBarPntr->AddItem (PopUpMenuPntr);
  MenuBarPntr->ResizeToPreferred (); // Account for largest item's space.

  BackgroundViewPntr->AddChild (MenuBarPntr);
  m_KeyPopupMenuPntr = PopUpMenuPntr;

  Lock ();
  TempRect.left = MenuBarPntr->Frame().right + 2 * g_MarginBetweenControls;
  Unlock ();

  /* Add the "Value:" title string control. */

  MarginAboveControl = ceilf ((HeightUsedByControls - g_LineOfTextHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_LineOfTextHeight;
  TempRect.right = TempRect.left + 500 /* arbitrary, gets resized later */;

  StringViewPntr = new BStringView (TempRect, "Value BStringView", "Value:");
  if (StringViewPntr == NULL) goto ErrorExit;
  StringViewPntr->ResizeToPreferred ();

  BackgroundViewPntr->AddChild (StringViewPntr);

  Lock ();
  TempRect.left = StringViewPntr->Frame().right + g_MarginBetweenControls;
  Unlock ();

  /* Add the pop-up menu for selecting the value's data type. */

  MarginAboveControl = ceilf ((HeightUsedByControls - g_PopUpMenuHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_PopUpMenuHeight;
  TempRect.right = TempRect.left + 500 /* arbitrary, gets resized later */;

  MenuBarPntr = new BMenuBar (TempRect, "Value Type MenuBar",
    B_FOLLOW_LEFT | B_FOLLOW_TOP, B_ITEMS_IN_COLUMN,
    false /* true resize to fit currently marked item */);
  if (MenuBarPntr == NULL) goto ErrorExit;

  PopUpMenuPntr = new BPopUpMenu ("Value Type PopUpMenu",
    true /* radio mode */, true /* take label from marked item */);
  if (PopUpMenuPntr == NULL) goto ErrorExit;

  for (i = 0; i < MAX_TYPE_NAMES; i++)
  {
    MsgPntr = new BMessage (MSG_VALUE_TYPE);
    if (MsgPntr == NULL) goto ErrorExit;
    MsgPntr->AddInt32 (TypeCodeMsgIDString, g_TypeCodes [i]);
    MenuItemPntr = new BMenuItem (g_TypeNames [i], MsgPntr);
    if (MenuItemPntr == NULL) goto ErrorExit;
    if (g_TypeCodes [i] == g_TypeForValues)
      MenuItemPntr->SetMarked (true);
    PopUpMenuPntr->AddItem (MenuItemPntr);
  }

  MenuBarPntr->AddItem (PopUpMenuPntr);
  MenuBarPntr->ResizeToPreferred (); // Account for largest item's space.

  BackgroundViewPntr->AddChild (MenuBarPntr);
  m_ValuePopupMenuPntr = PopUpMenuPntr;

  Lock ();
  TempRect.left = MenuBarPntr->Frame().right + 2 * g_MarginBetweenControls;
  Unlock ();

  /* Add the disabled text box which is displaying the tree's current size. */

  MarginAboveControl = ceilf ((HeightUsedByControls - g_TextBoxHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_TextBoxHeight;
  WidthOfButtons = ceilf (be_plain_font->StringWidth ("MCount: 999999M"));
  TempRect.right = TempRect.left + WidthOfButtons;

  TextControlPntr = new BTextControl (TempRect,
    "Count Text Control",
    "Count:" /* label */,
    "123456",
    NULL /* no message */,
    B_FOLLOW_LEFT | B_FOLLOW_TOP,
    B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE);
  if (TextControlPntr == NULL) goto ErrorExit;

  TextControlPntr->SetEnabled (false);
  TextControlPntr->SetDivider (be_plain_font->StringWidth ("Count: "));

  BackgroundViewPntr->AddChild (TextControlPntr);
  m_CountViewPntr = TextControlPntr;

  /* Set up for the row with the control buttons. */

  RowBottomY += HeightUsedByControls;
  TempRect = Bounds ();
  HeightUsedByControls = ceilf (g_ButtonHeight * 1.4);

  /* Add the Add button. */

  MarginAboveControl = ceilf ((HeightUsedByControls - g_ButtonHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_ButtonHeight;
  WidthOfButtons = ceilf (be_plain_font->StringWidth ("MFunctionalityM"));
  TempRect.left += g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  ButtonPntr = new BButton (TempRect,
    "Add Button",
    "Add",
    new BMessage (MSG_ADD),
    B_FOLLOW_TOP | B_FOLLOW_LEFT);
  if (ButtonPntr == NULL) goto ErrorExit;

  BackgroundViewPntr->AddChild (ButtonPntr);

  Lock ();
  SetDefaultButton (ButtonPntr);
  Unlock ();

  /* Add the Delete button. */

  TempRect.left = TempRect.right + g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  ButtonPntr = new BButton (TempRect,
    "Delete Button",
    "Delete",
    new BMessage (MSG_DELETE),
    B_FOLLOW_TOP | B_FOLLOW_LEFT);
  if (ButtonPntr == NULL) goto ErrorExit;

  BackgroundViewPntr->AddChild (ButtonPntr);

  /* Add the Add Lots button for adding lots of nodes. */

  TempRect.left = TempRect.right + g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  ButtonPntr = new BButton (TempRect,
    "Add Lots Button",
    "Add Lots",
    new BMessage (MSG_ADD_LOTS),
    B_FOLLOW_TOP | B_FOLLOW_LEFT);
  if (ButtonPntr == NULL) goto ErrorExit;

  BackgroundViewPntr->AddChild (ButtonPntr);
  ButtonPntr->SetTarget (be_app);

  /* Add the Test Speed button. */

  TempRect.left = TempRect.right + g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  ButtonPntr = new BButton (TempRect,
    "Test Speed Button",
    "Test Speed",
    new BMessage (MSG_TEST_SPEED),
    B_FOLLOW_TOP | B_FOLLOW_LEFT);
  if (ButtonPntr == NULL) goto ErrorExit;

  BackgroundViewPntr->AddChild (ButtonPntr);

  /* Add the Test Functionality button. */

  TempRect.left = TempRect.right + g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  ButtonPntr = new BButton (TempRect,
    "Functionality Button",
    "Functionality",
    new BMessage (MSG_TEST_FUNCTIONALITY),
    B_FOLLOW_TOP | B_FOLLOW_LEFT);
  if (ButtonPntr == NULL) goto ErrorExit;

  BackgroundViewPntr->AddChild (ButtonPntr);

  /* Set up the row with the value and key text boxes. */

  RowBottomY += HeightUsedByControls;
  TempRect = Bounds ();
  HeightUsedByControls = ceilf (g_TextBoxHeight * 1.2);
  WidthOfButtons = ceilf (
    (TempRect.right - TempRect.left - 3 * g_MarginBetweenControls) / 2);
  MarginAboveControl = ceilf ((HeightUsedByControls - g_TextBoxHeight) / 2);
  TempRect.top = RowBottomY + MarginAboveControl;
  TempRect.bottom = TempRect.top + g_TextBoxHeight;
  TempRect.left += g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  /* Add the key box, variable sized as the window size changes. */

  TextControlPntr = new BTextControl (TempRect,
    "Key Text Control",
    "Key:" /* label */,
    "Some Key",
    NULL /* no message */,
    B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
    B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);
  if (TextControlPntr == NULL) goto ErrorExit;

  TextControlPntr->SetDivider (be_plain_font->StringWidth ("Key: "));

  BackgroundViewPntr->AddChild (TextControlPntr);
  m_KeyViewPntr = TextControlPntr;

  /* Add the value box, fixed sized aligned with right window edge. */

  TempRect.left = TempRect.right + g_MarginBetweenControls;
  TempRect.right = TempRect.left + WidthOfButtons;

  TextControlPntr = new BTextControl (TempRect,
    "Value Text Control",
    "Value:" /* label */,
    "Some Value",
    NULL /* no message */,
    B_FOLLOW_RIGHT | B_FOLLOW_TOP,
    B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);
  if (TextControlPntr == NULL) goto ErrorExit;

  TextControlPntr->SetDivider (be_plain_font->StringWidth ("Value: "));

  BackgroundViewPntr->AddChild (TextControlPntr);
  m_ValueViewPntr = TextControlPntr;

  /* Add the tree display view, inside a scrolling view, attached to the
  window (not the background view). */

  RowBottomY += HeightUsedByControls;
  TempRect = Bounds ();
  TempRect.top = RowBottomY + 1 /* avoid overlap */;
  TempRect.right -= B_V_SCROLL_BAR_WIDTH;
  TempRect.bottom -= B_H_SCROLL_BAR_HEIGHT;

  m_AVLTreeViewPntr = new AVLTreeView (TempRect);
  if (m_AVLTreeViewPntr == NULL) goto ErrorExit;

  ScrollPntr = new BScrollView (
    "BScrollView containing an AVLTreeView",
    m_AVLTreeViewPntr,
    B_FOLLOW_ALL_SIDES,
    B_WILL_DRAW,
    true /* horizontal scroll bar */,
    true /* vertical scroll bar */,
    B_PLAIN_BORDER);
  if (ScrollPntr == NULL) goto ErrorExit;

  AddChild (ScrollPntr);

  return true;

ErrorExit:
  DisplayErrorMessage ("InitializeContents() is unable to initialise the "
    "window contents (creating buttons etc).");
  return false;
}


/* Test various operations on the tree to make sure it's working correctly. */

typedef struct OrderStruct
{
  int previousKey;
  int totalCount;
  int *randomArray;
  bool *checkedArray;
  int expectedTreeSize;
} OrderRecord, *OrderPointer;

bool CheckOrderCallback (
  AVLDupThingConstPointer KeyPntr,
  AVLDupThingConstPointer ValuePntr,
  void *ExtraData)
{
  int          Index;
  OrderPointer Order = (OrderPointer) ExtraData;

  Index = ValuePntr->int32Thing;

  if (Index < 0 || Index >= Order->expectedTreeSize)
  {
    DisplayErrorMessage ("Value is corrupt, out of bounds.");
    return false;
  }

  if (Order->randomArray[Index] != KeyPntr->int32Thing)
  {
    DisplayErrorMessage ("Key doesn't match value.");
    return false;
  }

  if (Order->previousKey >= KeyPntr->int32Thing)
  {
    DisplayErrorMessage ("Key is out of order, should be always increasing.");
    return false;
  }

  if (Order->checkedArray[Index])
  {
    DisplayErrorMessage ("Duplicated index, something is wrong.");
    return false;
  }
  Order->checkedArray[Index] = true;

  Order->previousKey = KeyPntr->int32Thing;
  Order->totalCount++;
  return true;
}

bool CheckIterationCallback (
  AVLDupThingConstPointer KeyPntr,
  AVLDupThingConstPointer ValuePntr,
  void *ExtraData)
{
  int *IntPntr = (int *) ExtraData;

  (*IntPntr)++;
  return true;
}

void AVLTestWindow::TestFunctionality ()
{
  const int         MAXCOUNT = 1023;
  bool              CheckedArray [MAXCOUNT];
  int               i, j;
  AVLDupThingRecord Key, Key2;
  OrderRecord       OrderCallbackData;
  int               RandomIntsArray [MAXCOUNT];
  int               TempInt;
  AVLDupThingRecord Value;

  AVLDupFreeTree (g_TheTree);
  g_TypeForKeys = B_INT32_TYPE;
  g_TypeForValues = B_INT32_TYPE;
  g_TheTree = AVLDupAllocTree (g_TypeForKeys, g_TypeForValues, "Functions", 0);

  /* Generate a random ordering of the numbers 0..MAXCOUNT-1. */

  for (i = 0; i < MAXCOUNT; i++)
    RandomIntsArray [i] = i;
  for (i = 0; i < MAXCOUNT; i++)
  {
    j = rand ();
    j = j % MAXCOUNT;
    TempInt = RandomIntsArray [i];
    RandomIntsArray [i] = RandomIntsArray [j];
    RandomIntsArray [j] = TempInt;
  }

  /* Add them to the tree.  At each step, check that the tree is in order and
  contains the numbers so far. */

  for (i = 0; i < MAXCOUNT; i++)
  {
    Key.int32Thing = RandomIntsArray [i];
    Value.int32Thing = i;

    if (!AVLDupAdd (g_TheTree, &Key, &Value))
      break;

    OrderCallbackData.totalCount = 0;
    OrderCallbackData.previousKey = -1;
    OrderCallbackData.randomArray = RandomIntsArray;
    OrderCallbackData.checkedArray = CheckedArray;
    memset (CheckedArray, 0, sizeof (CheckedArray));
    OrderCallbackData.expectedTreeSize = i + 1;

    if (!AVLDupIterate (g_TheTree, NULL, NULL, false, NULL, NULL, false,
    CheckOrderCallback, &OrderCallbackData))
      break;

    if (OrderCallbackData.totalCount != i + 1)
    {
      DisplayErrorMessage ("Count doesn't match number of nodes added.");
      break;
    }

    TempInt = 0;
    for (j = 0; j < MAXCOUNT; j++)
      if (CheckedArray[j])
        TempInt++;

    if (TempInt != i + 1)
    {
      DisplayErrorMessage ("Missing an index value during addition.");
      break;
    }
  }

  if (i != MAXCOUNT)
  {
    DisplayErrorMessage ("Ran out of memory or detected an error, "
      "while adding initial random nodes.");
    goto ErrorExit;
  }

  if ((int) AVLDupGetTreeCount (g_TheTree) != MAXCOUNT)
  {
    DisplayErrorMessage ("Tree's count is incorrect after inserts finished.");
    goto ErrorExit;
  }

  /* Do some iteration tests. */

  for (i = 0; i < MAXCOUNT; i++)
  {
    Key.int32Thing = RandomIntsArray[i];
    Value.int32Thing = i;

    /* Do an exact search, including equals sometimes. */

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, &Value, true, &Key, &Value, true,
    CheckIterationCallback, &TempInt);
    if (TempInt != 1)
    {
      DisplayErrorMessage ("Exact search with equals failed.");
      goto ErrorExit;
    }

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, &Value, false, &Key, &Value, true,
    CheckIterationCallback, &TempInt);
    if (TempInt != 0)
    {
      DisplayErrorMessage ("Exact search with equals failed 2.");
      goto ErrorExit;
    }

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, &Value, true, &Key, &Value, false,
    CheckIterationCallback, &TempInt);
    if (TempInt != 0)
    {
      DisplayErrorMessage ("Exact search with equals failed 3.");
      goto ErrorExit;
    }

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, &Value, false, &Key, &Value, false,
    CheckIterationCallback, &TempInt);
    if (TempInt != 0)
    {
      DisplayErrorMessage ("Exact search with equals failed 4.");
      goto ErrorExit;
    }

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, NULL, false, &Key, NULL, false,
    CheckIterationCallback, &TempInt);
    if (TempInt != 1)
    {
      DisplayErrorMessage ("Key range search failed.");
      goto ErrorExit;
    }

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, NULL, true, &Key, NULL, true,
    CheckIterationCallback, &TempInt);
    if (TempInt != 1)
    {
      DisplayErrorMessage ("Key range search failed 2.");
      goto ErrorExit;
    }

    /* Do some large range searches from -infinity to i, and i to infinity. */

    Key.int32Thing = i;

    TempInt = 0;
    AVLDupIterate (g_TheTree, NULL, NULL, false, &Key, NULL, false,
    CheckIterationCallback, &TempInt);
    if (TempInt != i + 1)
    {
      DisplayErrorMessage ("Lower range search failed.");
      goto ErrorExit;
    }

    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, NULL, false, NULL, NULL, false,
    CheckIterationCallback, &TempInt);
    if (TempInt != MAXCOUNT - i)
    {
      DisplayErrorMessage ("Upper range search failed.");
      goto ErrorExit;
    }

    /* Do some middling searches from i-1 to i+1. */

    Key.int32Thing = i-1;
    Key2.int32Thing = i+1;
    TempInt = 0;
    AVLDupIterate (g_TheTree, &Key, NULL, false, &Key2, NULL, false,
    CheckIterationCallback, &TempInt);
    if ((i == 0 && TempInt != 2) || (i == MAXCOUNT-1 && TempInt != 2) ||
    ((i > 0 && i < MAXCOUNT-1) && TempInt != 3))
    {
      DisplayErrorMessage ("Middling range search failed.");
      goto ErrorExit;
    }
  }

  /* Now do some deletion tests, essentially the opposite of insertion. */

  for (i = MAXCOUNT - 1; i >= 0; i--)
  {
    Key.int32Thing = RandomIntsArray [i];
    Value.int32Thing = i;

    if (!AVLDupDelete (g_TheTree, &Key, &Value))
      break;

    OrderCallbackData.totalCount = 0;
    OrderCallbackData.previousKey = -1;
    OrderCallbackData.randomArray = RandomIntsArray;
    OrderCallbackData.checkedArray = CheckedArray;
    memset (CheckedArray, 0, sizeof (CheckedArray));
    OrderCallbackData.expectedTreeSize = i;

    if (!AVLDupIterate (g_TheTree, NULL, NULL, false, NULL, NULL, false,
    CheckOrderCallback, &OrderCallbackData))
      break;

    if (OrderCallbackData.totalCount != i)
    {
      DisplayErrorMessage ("Count doesn't match number of nodes deleted.");
      break;
    }

    TempInt = 0;
    for (j = 0; j < MAXCOUNT; j++)
      if (CheckedArray[j])
        TempInt++;

    if (TempInt != i)
    {
      DisplayErrorMessage ("Missing an index value during deletion.");
      break;
    }
  }

  if (i != -1)
  {
    DisplayErrorMessage ("Something went wrong while deleting nodes.");
    goto ErrorExit;
  }

  if ((int) AVLDupGetTreeCount (g_TheTree) != 0)
  {
    DisplayErrorMessage ("Tree's count is incorrect after deletion.");
    goto ErrorExit;
  }

  DisplayErrorMessage ("Functionality tests passed.");

ErrorExit:
//AVLDupFreeTree (g_TheTree);
//g_TheTree = AVLDupAllocTree (g_TypeForKeys, g_TypeForValues, "Test Tree", 0);
  TreeHasChanged ();
  CopyGlobalTypesToPopupMenus (); /* We also changed the types. */
}


/* Add and delete a large number of things and report the elapsed times.
Since we want to test the algorithm's speed, use simple small sized keys. */

bool TestSpeedCallback (
  AVLDupThingConstPointer KeyPntr,
  AVLDupThingConstPointer ValuePntr,
  void *ExtraData)
{
  return true;
}

void AVLTestWindow::TestSpeed ()
{
  double            ElapsedSeconds;
  bigtime_t         EndTime;
  int32             i;
  AVLDupThingRecord Key;
  int               MaxCount = 1024 * 1024;
  bigtime_t         StartTime;
  char              TempString [1024];
  AVLDupThingRecord Value;

  AVLDupFreeTree (g_TheTree);
  g_TypeForKeys = B_INT32_TYPE;
  g_TypeForValues = B_INT32_TYPE;
  g_TheTree = AVLDupAllocTree (g_TypeForKeys, g_TypeForValues, "Test Tree", 0);

  /* Measure addition speed. */

  StartTime = system_time ();

  for (i = MaxCount; i > 0; i--)
  {
    Key.int32Thing = i;
    Value.int32Thing = 1;

    if (!AVLDupAdd (g_TheTree, &Key, &Value))
      break;
  }

  EndTime = system_time ();

  ElapsedSeconds = (EndTime - StartTime) / 1000000.0;
  sprintf (TempString, "Elapsed time: %g seconds.  "
    "%g seconds per Add operation.",
    ElapsedSeconds, ElapsedSeconds / MaxCount);
  puts (TempString);
  DisplayErrorMessage (TempString);

  /* Measure iteration speed. */

  StartTime = system_time ();

  AVLDupIterate (g_TheTree, NULL, NULL, false, NULL, NULL, false,
    TestSpeedCallback, NULL);

  EndTime = system_time ();

  ElapsedSeconds = (EndTime - StartTime) / 1000000.0;
  sprintf (TempString, "Elapsed time: %g seconds.  "
    "%g seconds per Iteration operation.",
    ElapsedSeconds, ElapsedSeconds / MaxCount);
  puts (TempString);
  DisplayErrorMessage (TempString);

  /* Measure search speed. */

  StartTime = system_time ();

  for (i = MaxCount; i > 0; i--)
  {
    Key.int32Thing = i;
    Value.int32Thing = 1;

    AVLDupIterate (g_TheTree, &Key, &Value, true, &Key, &Value, true,
      TestSpeedCallback, NULL);
  }

  EndTime = system_time ();

  ElapsedSeconds = (EndTime - StartTime) / 1000000.0;
  sprintf (TempString, "Elapsed time: %g seconds.  "
    "%g seconds per Search operation.",
    ElapsedSeconds, ElapsedSeconds / MaxCount);
  puts (TempString);
  DisplayErrorMessage (TempString);

  /* Measure deletion speed. */

  StartTime = system_time ();

  for (i = MaxCount; i > 0; i--)
  {
    Key.int32Thing = i;
    Value.int32Thing = 1;

    if (!AVLDupDelete (g_TheTree, &Key, &Value))
      break;
  }

  EndTime = system_time ();

  ElapsedSeconds = (EndTime - StartTime) / 1000000.0;
  sprintf (TempString, "Elapsed time: %g seconds.  "
    "%g seconds per Delete operation.",
    ElapsedSeconds, ElapsedSeconds / MaxCount);
  puts (TempString);
  DisplayErrorMessage (TempString);

  TreeHasChanged ();
  CopyGlobalTypesToPopupMenus (); /* We also changed the types. */
}


/* The tree has changed, request a redraw of the tree view and update the
counter display with the new count. */

void AVLTestWindow::TreeHasChanged ()
{
  char TempString [80];

  if (m_AVLTreeViewPntr != NULL)
    m_AVLTreeViewPntr->TreeHasChanged ();

  if (m_CountViewPntr != NULL)
  {
    if (g_TheTree == NULL)
      strcpy (TempString, "No Tree");
    else
      sprintf (TempString, "%d", g_TheTree->count);

    m_CountViewPntr->SetText (TempString);
  }
}



/******************************************************************************
 * The main application class.  It opens a window that shows the current
 * tree in a scrolling subview, and has buttons and controls at the top
 * to let you do operations on the tree.
 */

class AVLTestApp : public BApplication
{
public:
  int m_AddLotsCounter;

  AVLTestApp ();

  virtual void MessageReceived (BMessage *MessagePntr);
  virtual void Pulse ();
  virtual void ReadyToRun ();

  AVLTestWindow *m_AVLTestWindowPntr;
};


AVLTestApp::AVLTestApp ()
: BApplication ("application/x-vnd.agmsmith.AVLTest"),
  m_AVLTestWindowPntr (NULL)
{
}


void AVLTestApp::MessageReceived (BMessage *MessagePntr)
{
//  printf ("\nApp received message %s.\n",
//    U32toString (MessagePntr->what));
//  MessagePntr->PrintToStream ();

  switch (MessagePntr->what)
  {
    case MSG_ADD_LOTS:
      if (m_AddLotsCounter > 0)
        m_AddLotsCounter = 0;
      else
        m_AddLotsCounter = 1023;
      break;

    default:
      BApplication::MessageReceived (MessagePntr);
      break;
  }
}


void AVLTestApp::Pulse ()
{
  AVLDupThingRecord Key;
  char              TempString [1024];
  AVLDupThingRecord Value;

  if (m_AVLTestWindowPntr == NULL || g_TheTree == NULL)
    return;

  if (m_AddLotsCounter > 0)
  {
    Key.int64Thing = 0;
    Value.int64Thing = 0;

    sprintf (TempString, "%d is the key.", m_AddLotsCounter);
    if (AVLDupConvertStringToThing (TempString, &Key, g_TypeForKeys))
    {
      sprintf (TempString, "%d is the value.", m_AddLotsCounter);
      if (AVLDupConvertStringToThing (TempString, &Value, g_TypeForValues))
      {
        AVLDupAdd (g_TheTree, &Key, &Value);

        m_AVLTestWindowPntr->Lock ();
        m_AVLTestWindowPntr->TreeHasChanged ();
        m_AVLTestWindowPntr->Unlock ();

        AVLDupFreeThingArray (&Value, g_TypeForValues, 1);
      }

      AVLDupFreeThingArray (&Key, g_TypeForKeys, 1);
    }
    m_AddLotsCounter--;
  }
}


void AVLTestApp::ReadyToRun ()
{
  float         JunkFloat;
  BButton      *TempButtonPntr;
  BCheckBox    *TempCheckBoxPntr;
  font_height   TempFontHeight;
  BMenuBar     *TempMenuBarPntr;
  BMenuItem    *TempMenuItemPntr;
  BPopUpMenu   *TempPopUpMenuPntr;
  BRadioButton *TempRadioButtonPntr;
  BRect         TempRect;
  const char   *TempString = "Temp My Test";
  BTextControl *TempTextPntr;

  /* Create the empty tree we will be working with. */

  g_TheTree =
    AVLDupAllocTree (g_TypeForKeys, g_TypeForValues, "The Global Tree", 1);
  if (g_TheTree == NULL)
  {
    DisplayErrorMessage ("Unable to allocate the empty tree.");
    Quit (); /* Tell the BApplication object to quit running. */
    return;
  }

  /* Create the new empty window, later the controls will be added. */

  m_AVLTestWindowPntr = new AVLTestWindow;
  if (m_AVLTestWindowPntr == NULL)
  {
    DisplayErrorMessage ("Unable to create window.");
    Quit (); /* Tell the BApplication object to quit running. */
    return;
  }

  m_AVLTestWindowPntr->Show ();

  /* Set the spacing between buttons and other controls to the width of
  the letter "M" in the user's desired font. */

 g_MarginBetweenControls = ceilf (be_plain_font->StringWidth ("M"));

  /* Also find out how much space a line of text uses. */

  be_plain_font->GetHeight (&TempFontHeight);
  g_LineOfTextHeight =
    TempFontHeight.ascent + TempFontHeight.descent + TempFontHeight.leading;

  /* Find the height of a button, which seems to be larger than a text
  control and can make life difficult.  Make a temporary button, which
  is attached to our window so that it resizes to accomodate the font size. */

  TempRect = m_AVLTestWindowPntr->Bounds ();
  TempButtonPntr = new BButton (TempRect, TempString, TempString, NULL);
  if (TempButtonPntr != NULL)
  {
    m_AVLTestWindowPntr->Lock ();
    m_AVLTestWindowPntr->AddChild (TempButtonPntr);
    TempButtonPntr->GetPreferredSize (&JunkFloat, &g_ButtonHeight);
    m_AVLTestWindowPntr->RemoveChild (TempButtonPntr);
    m_AVLTestWindowPntr->Unlock ();
    delete TempButtonPntr;
  }

  /* Find the height of a text box. */

  TempTextPntr = new BTextControl (TempRect, TempString, NULL /* label */,
    TempString, NULL);
  if (TempTextPntr != NULL)
  {
    m_AVLTestWindowPntr->Lock ();
    m_AVLTestWindowPntr->AddChild (TempTextPntr);
    TempTextPntr->GetPreferredSize (&JunkFloat, &g_TextBoxHeight);
    m_AVLTestWindowPntr->RemoveChild (TempTextPntr);
    m_AVLTestWindowPntr->Unlock ();
    delete TempTextPntr;
  }

  /* Find the height of a checkbox control. */

  TempCheckBoxPntr = new BCheckBox (TempRect, TempString, TempString, NULL);
  if (TempCheckBoxPntr != NULL)
  {
    m_AVLTestWindowPntr->Lock ();
    m_AVLTestWindowPntr->AddChild (TempCheckBoxPntr);
    TempCheckBoxPntr->GetPreferredSize (&JunkFloat, &g_CheckBoxHeight);
    m_AVLTestWindowPntr->RemoveChild (TempCheckBoxPntr);
    m_AVLTestWindowPntr->Unlock ();
    delete TempCheckBoxPntr;
  }

  /* Find the height of a radio button control. */

  TempRadioButtonPntr =
    new BRadioButton (TempRect, TempString, TempString, NULL);
  if (TempRadioButtonPntr != NULL)
  {
    m_AVLTestWindowPntr->Lock ();
    m_AVLTestWindowPntr->AddChild (TempRadioButtonPntr);
    TempRadioButtonPntr->GetPreferredSize (&JunkFloat, &g_RadioButtonHeight);
    m_AVLTestWindowPntr->RemoveChild (TempRadioButtonPntr);
    m_AVLTestWindowPntr->Unlock ();
    delete TempRadioButtonPntr;
  }

  /* Find the height of a pop-up menu. */

  TempMenuBarPntr = new BMenuBar (TempRect, TempString,
    B_FOLLOW_LEFT | B_FOLLOW_TOP, B_ITEMS_IN_COLUMN,
    true /* resize to fit items */);
  TempPopUpMenuPntr = new BPopUpMenu (TempString);
  TempMenuItemPntr = new BMenuItem (TempString, new BMessage (12345), 'g');

  if (TempMenuBarPntr != NULL && TempPopUpMenuPntr != NULL &&
  TempMenuItemPntr != NULL)
  {
    TempPopUpMenuPntr->AddItem (TempMenuItemPntr);
    TempMenuBarPntr->AddItem (TempPopUpMenuPntr);

    m_AVLTestWindowPntr->Lock ();
    m_AVLTestWindowPntr->AddChild (TempMenuBarPntr);
    TempMenuBarPntr->GetPreferredSize (&JunkFloat, &g_PopUpMenuHeight);
    m_AVLTestWindowPntr->RemoveChild (TempMenuBarPntr);
    m_AVLTestWindowPntr->Unlock ();
    delete TempMenuBarPntr; // It will delete contents too.
  }

  if (!m_AVLTestWindowPntr->InitializeContents ())
    Quit (); // Error message was already displayed, so just exit.

  SetPulseRate (500000);
}



/******************************************************************************
 * Finally, the main program which drives it all.
 */

int main (int argc, char** argv)
{
  AVLTestApp MyApp;

  MyApp.Run ();

  if (g_TheTree != NULL)
  {
    AVLDupFreeTree (g_TheTree);
    g_TheTree = NULL;
  }

  return 0;
}
