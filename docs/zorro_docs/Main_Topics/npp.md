---
title: Untitled
url: https://zorro-project.com/manual/en/npp.htm
category: Main Topics
subcategory: None
related_pages:
- [Zorro.ini Setup](ini.htm)
---

# Untitled

### Notepad++ script editor

Notepad++ (official website: <https://notepad-plus-plus.org>) is an open source text editor by Don Ho, included in the Zorro distribution under the GNU General Public License (Notepad++\license.txt) as a replacement of the previous SED editor.The source code of Notepad++ is available under the URLs in Notepad++\readme.txt. The editor configuration and language highlighting have been adapted to Zorro's C functions and keywords. You can change the default editor in [Zorro.ini](ini.htm).  
  
  
![Notepad++](../images/npp2.jpg)  
  
\- For running the current script with Zorro, hit **F5** , or click the 'Z' icon in the toolbar.   
\- For opening the Zorro manual at the keyword under the cursor, hit **F1** , or right click and select **Help**.   
\- Installing updates from the N++ website will overwrite the editor and language configurations. For restoring them, preserve the following Zorro-specific files and copy them back after the update: **langs.xml; shortcuts.xml; autoCompletion\c.xml; plugins\config.\\*.***.   
\- For adding Zorro functions and shortcuts to another Npp instance, copy the above files to the Npp folders. The **LanguageHelp** and **CustomizeToolbar** plugins must be installed.  
  
The command reference below was compiled by Andres Gomez Casanova for a prior Notepad++ version; the commands added or modified for Zorro have been marked with *.    

# File menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**Ctrl-O** |  Open File  
**Ctrl-N** |  New File  
**Ctrl-S** |  Save File  
**Ctrl-Alt-S** |  Save As  
**Ctrl-Shift-S** |  Save All  
**Ctrl-P** |  Print  
**Alt-F4** |  Exit  
**Ctrl-Tab** |  Next Document (also shows list of open files). Can be disabled - see Settings/Preferences/Global.  
**Ctrl-Shift-Tab** |  Previous Document (also shows list of open files). Can be disabled - see above.  
**Ctrl-Numpadn** |  Go to the n-th document on tab bar, n between 1 and 9.  
**Ctrl-PgUp** |  Next document  
**Ctrl-PgDn** |  Previous document  
**Ctrl-W** |  Close Current Document  
  
# Edit menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**Ctrl-C** |  Copy selection, copy current line*  
**Ctrl-Insert** |  Copy  
**Ctrl-Shift-T** |  Copy current line  
**Ctrl-X** |  Cut selection, cut current line*  
**Shift-Delete** |  Cut  
**Ctrl-V** |  Paste  
**Shift-Insert** |  Paste  
**Ctrl-Z** |  Undo  
**Alt-Backspace** |  Undo  
**Ctrl-Y** |  Redo  
**Ctrl-A** |  Select All  
**Alt-Shift-Arrow keys, or Alt + Left mouse click** |  Column Mode Select  
**Ctrl + Left mouse click** |  Start new selected area. Only multiple stream areas ca be selected this way.  
**ALT-C** |  Column Editor  
**Ctrl-D** |  Duplicate Current Line  
**Ctrl-T** |  Switch the current line position with the previous line position  
**Ctrl-Shift-Up** |  Move Current Line, or current selection if a single stream, Up  
**Ctrl-Shift-Down** |  Move Current Line, or current selection if a single stream, Down  
**Ctrl-L** |  Delete Current Line*  
**Ctrl-I** |  Split Lines  
**Ctrl-J** |  Join Lines  
**Ctrl-G** |  Launch GoToLine Dialog  
**Ctrl-Q** |  Toggle single line comment  
**Ctrl-Shift- K** |  Single line uncomment  
**Ctrl-K** |  Single line comment  
**Ctrl-Shift- Q** |  Block comment  
**Tab (selection of one or more full lines)** |  Insert Tabulation or Space (Indent)  
**Shift-Tab (selection of one or more full lines)** |  Remove Tabulation or Space (outdent)  
**Ctrl-BackSpace** |  Delete to start of word  
**Ctrl-Delete** |  Delete to end of word  
**Ctrl-Shift-BackSpace** |  Delete to start of line  
**Ctrl-Shift-Delete** |  Delete to end of line  
**Ctrl-U** |  Convert to lower case  
**Ctrl-Shift-U** |  Convert to UPPER CASE  
**Ctrl-B** |  Go to matching brace  
**Ctrl-Space** |  Launch CallTip ListBox  
**Ctrl-Shift-Space** |  Launch Function Completion ListBox  
**Ctrl-Alt-Space** |  Launch Path Completion ListBox  
**Ctrl-Enter** |  Launch Word Completion ListBox  
**Ctrl-Alt-R** |  Text Direction RTL  
**Ctrl-Alt-L** |  Text Direction LTR  
**Enter** |  Split line downwards, or create new line  
**Shift-Enter** |  Split line downwards, or create new line  
**Ctrl-Alt-Enter** |  SInsert new unindented line above current  
**Ctrl-Alt-Shift-Enter** |  SInsert new unindented line below current  
  
# Search menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**Ctrl-F** |  Launch Find Dialog  
**Ctrl-H** |  Launch Find / Replace Dialog  
**F3** |  Find Next  
**Shift-F3** |  Find Previous  
**Ctrl-Shift-F** |  Find in Files  
**F7** |  Switch to Search results window (was Activate sub view before v5.2)  
**Ctrl-Alt-F3** |  Find (volatile) Next  
**Ctrl-Alt-Shift-F3** |  Find (volatile) Previous  
**Ctrl-F3** |  Select and Find Next (was Find (Volatile) Next prior to v5.6.5)  
**Ctrl-Shift-F3** |  Select and Find Previous (was Find (Volatile) Previous prior to v5.6.5)  
**F4** |  Go to next found  
**Shift-F4** |  Go to previous found  
**Ctrl-Shift-I** |  Incremental Search  
**Ctrl-n** |  Jump Down (to next text marked using n-th stye. n is 1 to 5, or 0 for default Found style.  
**Ctrl-Shift-n** |  Jump Up (to next text marked using n-th stye. n is 1 to 5, or 0 for default Found style.  
**Ctrl-F2** |  Toggle Bookmark  
**F2** |  Go To Next Bookmark  
**Shift-F2** |  Go To Previous Bookmark  
**Ctrl-B** |  Go to Matching Brace (caret must be on a brace)  
**Ctrl-Alt-B** |  Select All between Matching Braces (caret must be on a brace)  
  
# View menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**Ctrl-(Keypad-/Keypad+)** |  or Ctrl + mouse wheel button (if any) Zoom in (+ or up) and Zoom out (- or down)  
**Ctrl-Keypad/** |  Restore the original size from zoom  
**F11** |  Toggle Full Screen Mode  
**F12** |  Toggle Post-It Mode  
**Ctrl-Alt-F** |  Collapse the Current Level  
**Ctrl-Alt-Shift-F** |  Uncollapse the Current Level  
**Alt-0** |  Fold All  
**Alt-(1~8)** |  Collapse the Level (1~8)  
**Alt-Shift-0** |  Unfold All  
**Alt-Shift-(1~8)** |  Uncollapse the Level (1~8)  
  
# Macro menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**Ctrl-Shift-R** |  Start to record /Stop recording the macro  
**Ctrl-Shift-P** |  Play recorded macro  
**Alt-Shift-S** |  Trim Trailing and Save  
  
# Run menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**F5** |  Run Script with Zorro*  
**Shift -F1** |  Open Zorro Help*  
**Alt-F 3** |  Wikipedia search  
**Alt-F6** |  Open file in another instance  
  
# ? menu

**Shortcut ↓** |  **Action ↓**  
---|---  
**Ctrl- F1** |  About  
**Shift+F1** |  Open Zorro Help*  
**F1** |  Open Zorro Help at Keyword under Cursor*  
  
# Incremental search widget

**Shortcut ↓** |  **Action ↓**  
---|---  
**Enter** |  Next match (same as > button)  
**Shift+Enter** |  Previous match (same as < button)  
  
# Mouse gestures

**Shortcut ↓** |  **Action ↓**  
---|---  
**Single left click** |  Set current line  
**Single left click on rightmost status bar pane** |  Toggle typing mode between Insert and Overtype  
**Single left click on bookmark margin** |  Toggle bookmark  
**Shift+left click on fold point** |  Uncollapse this fold and all those below  
**Ctrl+left click on fold point** |  Toggle collapsed state of this fold, and propagate below  
**Right click** |  Pop up context menu  
**Double left click** |  Select word  
**Double left click on location pane (status bar)** |  Go to line  
**Triple left click** |  Select line
