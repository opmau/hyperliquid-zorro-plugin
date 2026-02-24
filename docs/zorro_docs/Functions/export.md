---
title: Data export / import
url: https://zorro-project.com/manual/en/export.htm
category: Functions
subcategory: None
related_pages:
- [set, is](mode.htm)
- [printf, print, msg](printf.htm)
- [Verbose](verbose.htm)
- [Log Messages](log.htm)
- [asset, assetList, ...](asset.htm)
- [Multiple cycles](numtotalcycles.htm)
- [Zorro.ini Setup](ini.htm)
- [algo](algo.htm)
- [trade management](trade.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [Tips & Tricks](tips.htm)
- [CBI](ddscale.htm)
- [optimize](optimize.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [System strings](script.htm)
- [TrainMode](opt.htm)
- [Bars and Candles](bars.htm)
- [File access](file_.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [assetHistory](loadhistory.htm)
---

# Data export / import

# Exported Files

### The log

Zorro records script printouts and trade or other events in a **.log** file when the [LOGFILE](mode.htm) flag is set. The log contains the currrent equity and asset price at every bar, plus events such as opening or closing a trade, adjusting a trailing stop, or a [printf](printf.htm) message by the script. If [Verbose](verbose.htm) is at a higher level, it also records the daily state of all open trades, the daily profit or loss, and the current drawdown depth. Event logs are normal text files and can be opened with any text editor. Their content looks like this (the messages are explained under [Log](log.htm)):

```c
--- Monday 03.08. - daily profit +363$ ---
[GER30:EA:L7407] +684.97 / +685.0 pips
[GER30:HU:L7408] +684.97 / +685.0 pips
[UK100:EA:L7409] +580.55 / +464.4 pips
[USD/CAD:LP:S7612] +136.29 / +851.8 pips
[USD/CAD:LS:S7613] +68.15 / +851.8 pips
[AUD/USD:LP:L8412] +115.92 / +483.0 pips
[AUD/USD:MA:L1511] +42.50 / +265.6 pips
[USD/CAD:HU:S5412] +14.63 / +182.9 pips
[AUD/USD:HU:L5413] +28.05 / +175.3 pips
[GER30:EA:L7407] Trail 1@4746 Stop 4907
 
[5770: 04.08. 04:00]  6: 9289p 143/299
[GER30:EA:L7407] Trail 1@4746 Stop 4914
[AUD/USD:MA:L1511] Exit after 56 bars
[AUD/USD:MA:L1511] Exit 2@0.8406: +38.71 07:36
 
[5771: 04.08. 08:00]  6: 9773p 143/299
[GER30:EA:L7407] Trail 1@4746 Stop 4920
 
[5772: 04.08. 12:00]  6: 9773p 143/299
[GER30:EA:L7407] Trail 1@4746 Stop 4927
[USD/JPY:LP:S7308] Short 1@94.71 Risk 8
[USD/JPY:LS:S7309] Short 2@94.71 Risk 29
[USD/JPY:HU:S7310] Short 1@94.71 Risk 13
```

The name of the log file is the script name with appended **.log** extension. If the script contains no [asset](asset.htm) call or if the first **printf** message precedes it, the name of the selected asset from the scrollbox is added to the log file name. Optionally a number ([LogNumber](numtotalcycles.htm)) can be added to the file name, for comparing log files generated with different script versions. New log files overwrite old log files of the same name. For preventing that the **Log** folder gets cluttered with thousands of log files, Zorro can be set up in [Zorro.ini](ini.htm) to automatically delete old files.

### Trade lists 

The exported trade lists - ***_trd.csv** , **demotrades.csv** , **trades.csv** \- contain a description of every finished trade in comma separated format for import in Excel™ or other spreadsheet or database programs. They can be used for evaluating trade statistics \- or for the tax declaration.  
The list named ***_trd.csv** is exported in [Test] mode when [LOGFILE](mode.htm) is set. **demotrades.csv** and **trades.csv** are exported in [Trade] mode, dependent on whether a demo or real account was selected. The latter two are perpetual files, meaning that they are never overwritten, but their content is preserved and any new finished trade is just added at the end. Thus, theses lists will grow longer and longer until they are deleted manually or moved to a different folder. Depending on the [Comma](ini.htm) setting, numbers are exported with either a decimal comma or point, and separated with either a semicolon or a comma; this is because German spreadsheet programs require CSV data to be separated with semicolon. The trade list contains the followiing fields:

**Name** | Algo identifier (see [algo](algo.htm)), or the script name when no identifier is used.  
---|---  
**ID** | Trade identifier number, for finding the trade in the log file and in the broker's records.   
**Type** | Trade type, **Long** or **Short**. For options, also **Put** or **Call**.  
**Asset** | Traded asset.   
**Lots** | Number of lots, shares, or contracts. For a lot size different to one contract, multiply this with the asset's **LotAmount** (see above) to get the number of contracts.   
**Open** | Timestamp when the trade was opened, in the format **Year-Month-Day Hour:Minute(:Second.Millisecond)**.   
**Close** | Timestamp when the trade was closed., in the format **Year-Month-Day Hour:Minute(:Second.Millisecond)**.  
**Entry** | Trade fill price ([TradeFill](trade.htm)). If the fill price is not returned from the broker API, the fill price is estimated from the ask or bid (dependent on **Type**) at trade entry.   
**Exit** | Trade exit price. If the exit fill price is not returned from the broker API, the ask or bid (dependent on **Type**) market price at trade exit. For in the money expired or exercised options, it's the executed underlying price. 0 for out of the money expired options.  
**ExitType** | **Sold** (by [exitTrade](selllong.htm)), **Reverse** (by **[enterTrade](selllong.htm)**), **Stop** ([stop loss](stop.htm)), **Target ([profit ](stop.htm)**[target](stop.htm)), **Time** ([ExitTime](timewait.htm)), **Expired** (options, futures), **Exit** (by a [TMF](trade.htm) that returned **1**), **Cancelled** ([cancelTrade](selllong.htm)) or **Closed** (externally closed in the broker platform).   
**Roll** | Rollover ('swap') received from or paid to the broker for keeping the position open overnight, in units of the account currency.   
**Commission** | Commission paid to the broker for opening and closing the position, in units of the account currency.   
**MAE** | Maximum adverse excursion in units of the account currency.   
**MFE** | Maximum favorable excursion in units of the account currency.   
**Profit** | Profit or loss in units of the account currency, as returned from the broker API. Includes spread, commission, and slippage.   
  
### Spreadsheets 

The [print(TO_CSV,...)](printf.htm) function offers an easy way to export backtest data or indicator values to a .csv spreadsheet in the **Log** folder. Under [Tips & Tricks](tips.htm) some more examples can be found for exporting data to **.csv** files or for importing data from an external text file, for instance to set up strategy parameters.  

### P&L curves 

When the [LOGFILE](mode.htm) flag is set in [Test] or [Trade] mode, the daily profit and loss curve is exported at the end of the session to a **_pnl.csv** file in the **Log** folder, and a **_pnl.dbl** file in the **Data** folder. For this the session must have run for more than one day and at least one trade must have been executed. The files contain the daily balance or equity values in ascending date order, in text format in the **.csv** file, and as a **double** array in the **.dbl** file. This file is used for calculating the [Cold Blood Index](ddscale.htm) in a subsequent live trading session. Note that the array samples the values at the end of the day, so its last value is not identical to the end profit when the simulation ended before the end of the day.

When the [EXPORT](mode.htm) flag is set in [Train] mode or for a [multi-cycle](numtotalcycles) test, the P&L curves generated from [ Parameter optimization](optimize.htm) and from any cycle are exported separately for any asset and algo to a **_cyc.dbl** file in the **Data** folder. The curves can be evaluated, for instance with [White's Reality Check](https://financial-hacker.com/whites-reality-check/). The file contains multiple **double** arrays, one for any curve. The size of any curve can be determined from the size of the **_pnl.dbl** file.

### Training data

Rules, parameters, factors, and machine learning models are exported to **.c** , **.par** , **.fac** , and **.ml** files in the **Data** folder. With the exception of factors, any [WFO cycle](numwfocycles.htm) exports a different set of files. The **.c** files contain the trained rules in C code, the **.par** and **.fac** are plain text files files containing the parameter values and factors separately for any component of the system. The **.ml** format is binary and depends on the used machine learning algorithm. The name of these files is affected by the [Script string](script.htm).  
  
[Brute Force](opt.htm) optimization exports all parameter combinations and the resulting objectives to a ***par.csv** file in the **Log** folder. The first column contains the [objective](objective) returns, the subsequent columns the parameter velues in the order of their appearance in the script. The exported spreadsheet can be evaluated by a script for generatiung heatmaps of any two parameters, or by Excel to generate a 3-d surface. Note that Excel requires re-arranging the objective column in a 2-d matrix for generating the surface.

### Remarks

  * Be careful when manually opening and saving CSV files with Excel. Dates, times, delimiters and decimal points are then possibly saved in your local format, which can cause the file to become unreadable by Zorro. 
  * Excel requires exclusive access, so Zorro cannot read or write to a file while it is open in Excel. This is indicated by an error message.

### See also:

[Bars](bars.htm), [file](file_.htm), [asset parameters](pip.htm), [assetHistory](loadhistory.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
