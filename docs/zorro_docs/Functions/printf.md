---
title: printf, print, msg
url: https://zorro-project.com/manual/en/printf.htm
category: Functions
subcategory: None
related_pages:
- [set, is](mode.htm)
- [Verbose](verbose.htm)
- [Status flags](is.htm)
- [Performance Report](performance.htm)
- [panel, ...](panel.htm)
- [PlotMode](plotmode.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Formats Cheat Sheet](format.htm)
- [File access](file_.htm)
- [wait](sleep.htm)
- [watch](watch.htm)
- [sound](sound.htm)
- [exec](exec.htm)
- [keys](keys.htm)
- [String handling](str_.htm)
- [progress](progress.htm)
---

# printf, print, msg

## printf (string format, ...)

Prints text and variables in a log file and the message window in [Test] and [Trade] modes. This is a standard C function. It allows to display messages and variables.  

## print (int to, string format, ...)

Similar to **printf** , but prints text and variables to a target channel given by the **to** parameter, such as the log file, a .csv spreadsheet file, the HTML status page, or the performance report. 

## msg (string format, ...): int 

Displays text and variables in a modal or nonmodal message box with [Yes] and [No] buttons, and waits for the user to click one of the buttons. Format strings beginning with '#' open a nonmodal box. 

### Returns

**0** when [No] was clicked, **1** when [Yes] was clicked. 

### Parameters:

**to** | **TO_WINDOW** \- print to the message window and the log file (default for **printf**).  
**TO_LOG** \- print in [Test] mode only to the log file, in [Trade] and in [STEPWISE](mode.htm) mode also to the message window.  
**TO_FILE** \- print in [Test] and [Trade] mode to the log file.   
**TO_ANY** \- print in all modes to the message window and log file (= **TO_WINDOW+TRAINMODE**).   
**TO_DIAG** \- print to the **diag.txt** file (see [Verbose](verbose.htm)). Note that log files and **diag.txt** file are not yet open in the [INITRUN](is.htm).  
**TO_ALERT** \- print to an alert box and stop black box recording when the **ALERT** flag is set for [Verbose](verbose.htm).  
**TO_OUT** \- print to a file with the script name plus **"out.txt"** in the **Log** folder.   
******TO_CSV** \- print to a file with the script name and extension **".csv"** in the **Data** folder, for exporting data.   
**TO_CSVHDR** \- print to the first line of the CSV file; for writing a header before writing the data..   
**TO_REPORT** \- print in the [EXITRUN](is.htm) to the [performance report](performance.htm), for displaying additional performance parameters.  
**TO_HTML** \- print in [Trade] or in [STEPWISE](mode.htm) mode to the HTML file that displays the live trading status. HMTL format codes can be included in the text.   
**TO_TITLE** \- print to the title bar.  
**TO_INFO** \- print to the info field. **print(TO_INFO, 0)** displays the current account or simulation state in the info field.  
**TO_PANEL** \- print to the caption bar of the current [control panel](panel.htm).   
**TO_CHART** \- print to the caption bar and - if [PL_TITLE](plotmode.htm) is set - to the title of the current [chart](chart.htm).  
**+TRAINMODE** \- print also in [Train] mode.   
---|---  
**format** | C-style format string, containing arbitrary text with placeholders for subsequent variables (see [format codes](format.htm)). The placeholders begin with a percent sign **'%'** and define the format of the displayed number. Examples: **"$%.2f"** prints a **var** with a dollar symbol and 2 decimals; **"%i"** prints an **int** ; **"%+4.1f"** prints a **var** with +/- sign, 4 digits, and one decimal. For printing **float** variables, use the **%f** placeholder and typecast them to **(var)**.  
********  
**...** | Expressions or variables, one for each placeholder in the **format** text.   
  
###  Remarks:

  * !!  Since the above functions accept any number and type of parameters, variable types are not checked and not automatically converted. A wrong variable type in a **printf** call will produce wrong output - not only of the variable, but also of all subsequent variables - and can even cause a program crash. So make sure that variables match the **format** string precisely. If you print a predefined variable, use the correct placeholder for **var** , **float** , **int** ,**** or **string**. For printing **float** variables, typecast them to **(var)** or **(double)** , like this: **printf("Profit: $%.2f",(var)TradeProfit)**. The **printf** function will not automatically promote a **float** to a **double**. 
  * For printing a percent sign in the text, use **"%%"** ; for a backslash, use **"\\\"**. A short list of C-style format strings can be found under [format codes](format.htm), a more detailed description in any C/C++ book. 
  * For printing on a new line in the log or message window, begin the text with the linefeed symbol **"\n"**. Otherwise **printf** appends the text to the end of the current line. For printing on a new line in a HTML file, begin the text with the tag **" <br>"**.
  * For deleting the current line in the message window and replacing it with new content, begin the text with the carriage-return symbol **"\r"**. This way running counters or similar effects can be realized. 
  * The maximum text size depends on the print channel. For the message window it's about 2000 characters, for a file 20000 characters. Longer texts are clipped.
  * For printing into a string, use the [sprintf](str_.hmt) or [strf](str_.hmt) functions. For printing into a file, use the [file_append](file_.htm) function. 
  * For printing with **printf** only to the log file and not to the window, add a **"#"** at the begin of the text (**printf("#\n...",...);**). In [Trade] mode log messages are also printed to the window.
  * For temporarily suppressing all **printf** messages, set **[Verbose](verbose.htm) |= SILENT**. 
  * For opening a nonmodal message box with the **msg** function, add a **"#"** at the begin of the text (**msg("#...",...);**). A nonmodal message box has an [Ok] and a [Cancel] button and does not wait for the user to click a button; the script just continues. When the user later clicks [Ok], the [AFFIRMED](is.htm) flag is set (see example). An empty message (**msg("#");**) lets the nonmodal box disappear. 
  * For refreshing the GUI so that printed messages are updated in the window, use the [wait](sleep.htm) function. 
  * For debugging purposes, the [watch](watch.htm) function is often more convenient than the **printf** function. It allows breakpoints, needs no format string, and prevents errors by printing wrong variable types.
  * For displaying or editing a text file, use **exec("Editor",...)**.

### Examples:

```c
// printf example
var vPrice = 123.456;
float fDiscount = 1.5;
int nNumber = 77;
...
printf("\nPrice %.3f for %d items, total %.3f, discount %.1f%%",
  vPrice, nNumber, nNumber*vPrice, (var)fDiscount);
```

```c
// HTML status message example
if(ATR(30) < Threshold)
  print(TO_HTML,"<br>Low volatility - ATR30 = %.3f",ATR(30));
```

```c
// nonmodal box example
function run()
{
  if(is(INITRUN))
    msg("#Nonmodal message box.\nClick [Ok] to exit script!");
  if(is(AFFIRMED))
    quit("Ok!");
  Sleep(10);
}
```

```c
// CSV file output example
function run()
{
  LookBack = 100;
  asset(Asset);
  print(TO_CSVHDR,"Date,Price,SMA,EMA\n");
  print(TO_CSV,"%s,%.4f,%.4f,%.4f\n",
    strdate(YMDHMS,0),
    priceC(),
    SMA(seriesC(),50),
    EMA(seriesC(),50));
}
```

### See also:

[sound](sound.htm), [string](aarray.htm#string), [Editor](exec.htm), [keys](keys.htm), [sprintf](str_.htm), [strf](str_.htm), [progress](progress.htm), [watch](watch.htm), [format codes](format.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
