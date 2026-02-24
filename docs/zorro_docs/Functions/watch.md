---
title: watch
url: https://zorro-project.com/manual/en/watch.htm
category: Functions
subcategory: None
related_pages:
- [Chart Viewer / Debugger](chart.htm)
- [Verbose](verbose.htm)
- [set, is](mode.htm)
- [printf, print, msg](printf.htm)
- [Troubleshooting](trouble.htm)
---

# watch

## watch (string text, ...)

Prints the given text and up to 8 following **bool** , **int** , **var** , **float** , or **string** variables to the message window or the log, or add text to exception or crash messages (lite-C only). Can enter [single step](chart.htm) debugging mode. Allows to quickly debug into functions and watch variable behavior. 

### Parameters:

**text** | Text string to be displayed, followed by the variables. If the string begins with an exclamation mark **"!..."** , script execution stops at that line and Zorro changes to [debugging](chart.htm) mode. This way the behavior of variables inside a loop or function can be debugged step by step. If the string begins with a **"#"** character, the text is not displayed in the message window, but printed in all modes to the log or - in diagnostics mode - to the **diag.txt** file (see [Verbose](verbose.htm)). If the string begins with a **"?"** , the text is displayed together with the next exception or crash message. If the string contains a decimal point **'.'** , **float** , **double** , or **var** variables are printed in higher precision.  
---|---  
**...** | Up to 8**** variables, function calls, or expressions to be watched. Supported are **bool** , **int** , **var** , **double** , **float** , or **string**. Floating point types are displayed with 5 decimals.   
  
###  Remarks:

  * **watch("!...", ...)** can be used as a breakpoint in the code for debugging variables in loops or between commands.
  * **watch("#...", ...)** can be used to examine variables while training.
  * **watch("?...")** can be used to determine the position of a script crash.
  * Take care to either remove or out-comment all **watch** statements, or disable them with [NOWATCH](mode.htm) before releasing or live trading the strategy. 

### Examples:

```c
int i;
for(i=0; i<10; i++)
  watch("!i",i,"twice",2*i,"square",i*i);
```

### See also:

[Verbose](verbose.htm), [debugging](chart.htm), [printf](printf.htm), [troubleshooting](trouble.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
