---
title: Button
url: https://zorro-project.com/manual/en/button.htm
category: Functions
subcategory: None
related_pages:
- [Chart Viewer / Debugger](chart.htm)
- [set, is](mode.htm)
- [Status flags](is.htm)
- [Command Line Options](command.htm)
---

# Button

## Button

The mode - by pressing a button on the main or the chart panel, or by starting Zorro with a command line - in which the current code is running.  
**2** | Test   
---|---  
**3** | Train   
**4** | Trade   
**5** | Result   
**8** | Single Step (see [debugging](chart.htm))  
**10** | Replay (see [debugging](chart.htm))  
  

### Type:

**int**

### Remarks:

  * By setting [TESTNOW](mode.htm), the script can perform a **[Test](is.htm)** run even when running in [Train] mode (**Button == 3**).

### Example:

```c
if(Button == 8) // single step
  updateMyPanel() // display variables in a panel
```

### See also:

[Command line](command.htm), [visual debugger](chart.htm), [Test](is.htm), [Train](is.htm)   
[Latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
