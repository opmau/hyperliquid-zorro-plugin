---
title: window
url: https://zorro-project.com/manual/en/window.htm
category: Functions
subcategory: None
related_pages:
- [keys](keys.htm)
- [order](order.htm)
- [exec](exec.htm)
- [mouse](mouse.htm)
- [Window handle](hwnd.htm)
---

# window

## ******** window(string title) : HWND

Returns a handle to the active window when its title bar contains the **title** string. Can be used to wait until a certain window or dialog becomes active. 

### Parameters:

**title** \- part of the window title (case sensitive), or **0** for returning the handle to the window that recently became active.  

###  Returns:

**HWND** of the found active window. Otherwise **0**. 

### Remarks:

  * Normally Zorro's own window is the active window, unless another application was started or a Windows dialog became active. 

### Example:

See [keys](keys.htm). 

### See also:

[keys](keys.htm), [order](order.htm), [exec](exec.htm), [mouse](mouse.htm), [HWnd](hwnd.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
