---
title: quit
url: https://zorro-project.com/manual/en/quit.htm
category: Functions
subcategory: None
related_pages:
- [run](run.htm)
- [return](return.htm)
- [Status flags](is.htm)
- [printf, print, msg](printf.htm)
- [wait](sleep.htm)
- [require, version](version.htm)
- [ExitCode](exitcode.htm)
---

# quit

## quit (string Text): int

Terminates or aborts the session or simulation cycle, and prints the text to the message window.

###  Returns

**1** if **Text** begins with **'!'** or **'#'** , otherwise **0**. 

### Remarks:

  * A **quit** call does not return immediately from the current [run](run.htm) function, which will continue until the end. For returning immediately from a function, place a [return](return.htm) statement before **quit()** (see example). 
  * If **Text** begins with **'!'** , the session or simulation will abort without **EXITRUN** and end statistics, as if [Stop] was pressed. No further cycles will be excecuted. 
  * If **Text** begins with **'+'** , the session or simulation will restart from the first bar on. 
  * Otherwise the current session or simulation cycle will prematurely end, the [EXITRUN](is.htm) flag will be set, open trades closed, statistics calculated, and the simulation will then proceed to the next training or simulation cycle, if any. At the end of all cycles, the simulation stops and **Text** is printed to the message window.

### Example:

```c
if(Equity < 100)
  return quit("!Out of money!");
```

### See also:

[printf](printf.htm), [run](run.htm), [wait](sleep.htm), [version](version.htm), [ExitCode](exitcode.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
