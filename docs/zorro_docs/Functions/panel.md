---
title: Control panel
url: https://zorro-project.com/manual/en/panel.htm
category: Functions
subcategory: None
related_pages:
- [slider](slider.htm)
- [Licenses](restrictions.htm)
- [Colors](colors.htm)
- [String handling](str_.htm)
- [System strings](script.htm)
- [printf, print, msg](printf.htm)
- [Status flags](is.htm)
- [call](call.htm)
- [wait](sleep.htm)
- [Functions](funclist.htm)
---

# Control panel

# User interface

For setting up strategy parameters, triggering actions, or displaying something, the following elements are available:

  * The [sliders](slider.htm) for entering or displaying numerical values, 
  * the [Result] button that can be redefined by script,
  * the [Action] scrollbox for displaying or selecting items or triggering script functions,
  * and a **control panel** with user-defined buttons, switches, displays, entry fields, and spreadsheet lists ([Zorro S](restrictions.htm) or executable strategies only).

The control panel can be created by script or with Excel™. The panel is a rectangular grid of multi-purpose cells, similar to a spreadsheet. Every cell can display text, numbers, or a color, and can serve as a numeric or text input field or as a push, toggle, radio, or spin button. With this concept, even complex control functions for commercial strategies can be realized in a simple way. Such a control panel does not look particularly pretty, but it's functional: 

![Panel](../images/grid.png)

A panel can have a virtually unlimited number of rows and columns. If there are more than fit on the screen, the panel can be scrolled with scrollbars on the right and bottom side. Header rows or columns can be defined that stay fixed when the rest of the panel scrolls. This way, individual asset parameters can be set up or displayed for multi-asset strategies. An example of a scrolling panel with thousands of rows can be found in the **History.c** script. 

The following functions can be used for defining and handling a control panel or for populating the [Action] scrollbox: 

## panel (int rows, int columns, int color, int size)

Opens an empty control panel with the given number of **rows** (at least 2) and **columns** (at least 1), grid **color** , and horizontal cell **size** in pixels. If a control panel was already open, it is replaced by the new one. The panel is opened next to the Zorro window, and can be freely moved by dragging it with the mouse. If the number of rows or columns exceed the screen size, scrollbars are added to the panel. A negative **size** limits the panel height to the height of the Zorro panel. **panel(0,0,0)** removes the panel. 

## panel (string filename, int color, int size) 

As above, but loads a control panel from a **.csv** spreadsheet file. Every cell of the panel is set up from the corresponding spreadsheet cell. The cells in the file must be separated with  ;  or  ,  delimiters. Every row must end with a newline character. The following control characters in the cell content have a special meaning: **!...** \- cell is a button;  #...  \- cell can be edited; **@** \- merge the cell with the next not-empty cell to the left. The default colors are grey for text cells, red for numbers, white for editable cells, and blue for buttons; they can be changed with the [ColorPanel](colors.htm) variables.   
CSV delimiters and control characters must otherwise not appear in cell content. For still displaying them, the cell content can be set with **panelSet()** after loading the panel. Examples of spreadsheet-generated panels can be found in the **Strategy** folder. 

##  panelSet (int row, int col, string text,_int color, int style, int type_)

Set properties or update content of an individual cell.  
  
**row** \- row number starting with **0** , or **-1** for the [Result] button, or **-2** for the [Action] scrollbox.   
**col** \- column number starting with **0** , or line number in the [Action] scrollbox.   
**text** \- content to appear in the cell, button, or scrollbox line, or **0** for not changing the content. Numbers can be converted to text with [strf](str_.htm).   
**color** \- background color of the cell, or **0** for not changing the color.   
**style** \- **1** normal text, **2** highlighted text, **3** greyed-out text, **+4** right-aligned, **+8** left-aligned, **+12** centered, **+16** bold, or **0** for not changing the style.  
**type** \- **1** normal cell, **2** editable cell, **4** button, **0** for not changing the type. If **row** is **-2** and **type** is nonzero, the **text** is selected in the [Action] scrollbox.  
  
The default colors for generating the panel from a spreadsheet are stored in [ColorPanel](colors.htm)**[n]** , where **n = 0** for text background, **1** for number background, **2** for an editable field, **3** for a button, **4** for highlighted text, **5** for greyed out text.   

##  panelGet (int row, int col): string

Returns a temporary string with the current content of the cell. Numbers can be converted from the returned string with [sscanf](str_.htm), [atof](str_.htm), or [atoi](str_.htm).   

##  panelMerge(int Row, int Col, int NumRows, int NumCols)

Merges the given area of the panel to a single cell, with the content of the first cell. **Row** / **Col** is the upper left corner and **NumRows** / **NumCols** the size. 

##  panelFix(int NumRows, int NumCols)

Gives the number of header rows or header columns that stay fixed on a vertical or horizontal scrolling panel. Set the other number to **0**. Header cells cannot be merged.   
  

##  panelSave (string Filename)

Saves the panel content in **.csv** format to the given **FileName** , or to **"Data\\*.pan"** when the name is empty or zero. 

##  panelLoad (string Filename)

Loads the saved panel content in **.csv** format from the given **FileName** , or from **"Data\\*.pan"** when the name is empty or zero. Only editable or button cells are affected.   

## click(int Row, int Col) 

User-supplied function that is triggered by the following GUI actions:  
\- mouse click on a button cell,  
\- entering a number or editing a cell,  
\- clicking the [Result] button (**Row** = **-1**),  
\- selecting a previously set action from the [Action] scrollbox (**Row = -2, Col =** action number),  
\- selecting an asset with the [Asset] scrollbox (**Row** = **-3** ; asset name in [AssetBox](script.htm)),   
\- or moving one of the 3 user-defined sliders (**Row** = **-4, Col =** slider number).  
Dependent on the desired behavior of the button, the function can then change the cell color or content through **panelSet** ,  and/or save the panel content with **panelSave()**.

### Remarks:

  * Control panels can only be created with [Zorro S](restrictions.htm), but can be included in executable strategies (**.x** files) and can then also be used with the free Zorro version.
  * Even without a panel, the [Result] button and the [Asset] and [Action] scrollboxes can trigger the **click** function. If this is unwanted, filter out clicks with negative row values.
  * Text on the panel caption bar can be displayed with [print(TO_PANEL,...)](printf.htm). 
  * Panel, Result button, and Action scrollbox stay active after the end of the script unless they are explicitely removed either with **panel(0,0,0,0)** , or by clicking [Edit], or by selecting a new script. In lite-C (not in C++), panel fields can still be edited and buttons can trigger functions when clicked on even after the script has terminated. This can be used for establishing a permanent working panel; if this is not desired, prevent triggering functions by checking the [RUNNING](is.htm) flag. 
  * For keeping the previous panel content when the strategy is restarted, call **panel()** or **panelLoad()** only when both the [INITRUN](is.htm) and the [CHANGED](is.htm) flags are set.
  * For keeping a selection state of the [Action] scrollbox, save it in an **AlgoVar** and restore it at the start of the script. Use the **panelSet** function with the **type** parameter at **1** for the selected item.
  * !!  The **click** function is a separate process and not in sync with with the **run** or **tick** functions. Therefore, directly changing the asset, sending broker commands, or modifying global variables in the **click** function can interfere with other functions and with the rest of your script, and may cause errors. For preventing this, use the [call](call.htm)**(1,....)** function for critical operations inside a **click** function. See example below.

### Examples: (see also Download.c, History.c, PanelTest.c)

```c
// click function example
function click(int row,int col)
{
  if(!is(RUNNING)) return; // prevent clicks after the end of the script
  if(row < 0) return; // prevent clicks by result button or scroll box
  sound("click.wav");  string Text = panelGet(row,col);  if(Text == "Buy") // toggle button between "Buy" and "Sell"    panelSet(row,col,"Sell",0,0,0);  else if(Text == "Sell")    panelSet(row,col,"Buy",0,0,0);  else if(Text == "Close All")  // push button to close all trades
    call(1,closeAllTrades,0,0);
  else if(Text == "Enter Trade")  // push button to enter position
    call(1,enterMyTrade,0,0);
  else if(Text == "Cancel")  // back to last saved state
    panelLoad("Data\\mypanel.csv");
  panelSave("Data\\mypanel.csv");  
}
```

```c
// Entering a code number
int Code = 0;

void click(int row,int col)
{
  sound("click.wav");
  Code = atoi(panelGet(0,0));
}

void enterNumber()
{
  Code = 0;
  panel(1,4,GREY,30);
  print(TO_PANEL,"Enter your code number");
  panelSet(0,0,"",0,1,2);
  panelMerge(0,0,1,3);
  panelSet(0,3,"Ok",ColorPanel[3],1,4);
  while(!Code) wait(0);
  panel(0,0,0);
  printf("\nCode: %i",Code);
}
```

```c
// Start a program via Action scrollbox
void click(int row,int col)
{
  if(row == -2 && col == 0) // first action line
    exec("Editor","Strategy\\Z.ini",0);
}

function run()
{
  panelSet(-2,0,"Edit Z9.ini",0,0,0);
  ...
}
```

### See also:

[sliders](slider.htm), [wait](sleep.htm), [user supplied functions](funclist.htm), [ColorPanel](colors.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
