---
title: Verbose
url: https://zorro-project.com/manual/en/verbose.htm
category: Functions
subcategory: None
related_pages:
- [Log Messages](log.htm)
- [Error Messages](errors.htm)
- [CBI](ddscale.htm)
- [printf, print, msg](printf.htm)
- [set, is](mode.htm)
- [Multiple cycles](numtotalcycles.htm)
- [Command Line Options](command.htm)
---

# Verbose

## Verbose

Determines the verbosity of trade, error, and diagnostics messages (see [Log](log.htm)). The more verbose, the slower the execution of large backtests. 

### Range:

**0** | Few messages. Less important [warnings](errors.htm) are suppressed. Bars are only printed to the log in [Trade] mode or when trades are open.   
---|---  
**1** | More messages (default). All bars and all major events, such as entering or closing a trade, are printed to the log and in [Trade] mode also displayed in the window..  
**2** | Even more messages, including the trade parameters at any trade entry and exit. In [Trade] mode the daily profit, drawdown, and [CBI](ddscale.htm) is printed once per day. In [Test] mode with [TICKS](ticks.htm) flag the tick preceding a trade is displayed. The prices of the selected asset are printed at any bar in the form **Open/High\Low/Close**. Critical error messages in any mode are also appended to the file **Log\Errors.txt**.  
**3** | Even more messages, including skipped trades, possible price outliers, and function parameter errors. Prices are displayed with more digits. In [Trade] mode all open trades are listed once per day.   
**7** | Extensive diagnostics messages, including all memory allocations that exceed 2 MB, all **BrokerTrade** calls, and API execution times.  
**+DIAG** | (**+8**) Additional flag to activate a 'black box recorder' for diagnostics. A **...diag.txt** file is created in the **Log** folder. It contains a list with the 4000 last events and can be used to determine the reason of a crash or other problem that leads to the termination of the script. For details see [troubleshooting](errors.htm#diag). Black box recording strongly affects the program speed, so do not use this flag unnecessarily.  
**+ALERT** | (**+16**) Additional flag to display errors and critical messages, such as suspicious asset parameters, possibly orphaned trades, broker API errors, or [print(TO_ALERT,..)](printf.htm) calls, in a separate alert box. **DIAG** recording is stopped at the first critical event, so the details leading to the situation can be evaluated from the black box log (**Log\\...diag.txt**). Recording continues when the alert box is closed.   
**+SILENT** | (**+32**) Suppresses all [printf](printf.htm) messages. Messages by **print(TO_WINDOW,...)** are still printed.  
**+LOGMSG** | (**+512**) Prints the log also to the message window.   
  

### Type:

**int**

### Remarks:

  * In the daily trade list at **Verbose** >= 2, the state of any trade is printed in the form **[Trade ID] - Profit/Loss - Stop - Current price - Entry** , f.i. **[AUD/USD:CY:S4400] +116$ s0.8733** **c0.8729 e0.8731**. The same list is printed on a click on the [Result] button. 
  * If **Verbose** >= 2 and the system is in a drawdown from a previous equity peak ("Adverse Excursion"), its depth and width, as well as the [Cold Blood Index](ddscale.htm) are once per day printed to the log in the form **Last DD: xx% in yy days, CBI zz%**.
  * In "black box recorder" mode a file ending with "**..diag.txt** " is generated in the **Log** folder. It contains a list with the last events and [printf](printf.htm) format strings, and can be used to determine the reason of a crash or other problem that leads to the termination of the script. It is normally locked during a running trade session, but can be viewed while the [Stop] messagebox is active. For details see [troubleshooting](errors.htm#diag).
  * Verbosity and black box recording can heavily reduce the training and test speed, so do not use this feature unnecessarily. 

### Example:

```c
function run()
{
  Verbose = 7+DIAG; // extensive messages plus black box recorder 
  ...
}
```

### See also:

[LOGFILE](mode.htm), [LogNumber](numtotalcycles.htm), [-diag](command.htm), [log](log.htm), [troubleshooting](errors.htm#diag)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
