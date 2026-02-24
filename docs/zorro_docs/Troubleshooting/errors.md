---
title: Error Messages
url: https://zorro-project.com/manual/en/errors.htm
category: Troubleshooting
subcategory: None
related_pages:
- [Log Messages](log.htm)
- [FXCM](fxcm.htm)
- [IB / TWS Bridge](ib.htm)
- [Oanda](oanda.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [watch](watch.htm)
- [printf, print, msg](printf.htm)
- [sound](sound.htm)
- [email](email.htm)
- [call](call.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [series](series.htm)
- [Variables](aarray.htm)
- [Verbose](verbose.htm)
- [enterLong, enterShort](buylong.htm)
- [contract, ...](contract.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [Time / Calendar](month.htm)
- [Troubleshooting](trouble.htm)
- [BarMode](barmode.htm)
- [Dates, holidays, market](date.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [set, is](mode.htm)
- [asset, assetList, ...](asset.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [Asset and Account Lists](account.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [plot, plotBar, ...](plot.htm)
- [adviseLong, adviseShort](advisor.htm)
- [optimize](optimize.htm)
- [loop](loop.htm)
- [vector, matrix](matrix.htm)
- [run](run.htm)
- [if .. else](if.htm)
- [trade management](trade.htm)
- [tick, tock](tick.htm)
- [algo](algo.htm)
- [TrainMode](opt.htm)
- [price, ...](price.htm)
- [dayHigh, dayLow, ...](day.htm)
- [Indicators](ta.htm)
- [bar](bar.htm)
- [for(open_trades), ...](fortrades.htm)
- [Virtual Hedging](hedge.htm)
- [Licenses](restrictions.htm)
- [Asset Symbols](symbol.htm)
- [brokerCommand](brokercommand.htm)
- [The Z Strategies](zsystems.htm)
- [assetHistory](loadhistory.htm)
- [Dataset handling](data.htm)
- [panel, ...](panel.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Zorro in the Cloud](vps.htm)
- [Visual Studio](dlls.htm)
- [Zorro.ini Setup](ini.htm)
- [Training](training.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Live Trading](trading.htm)
---

# Error Messages

# Error messages

Below you'll find a list of possible errors that can occur during compilation or at runtime, together with suggestions for finding the reason of the error. Messages related to opening and closing positions are listed under [Log](log.htm); known issues with brokers are commented on the related page ([FXCM](fxcm.htm), [IB](ib.htm), [Oanda](oanda.htm), [ MT4](mt4plugin.htm), etc...) in this manual. 

Error messages must never be ignored. Even when your script runs and produces a result, an error message always indicates that something is seriously wrong and must be fixed. Warning messages can be ignored in some cases, but only when you know and understood their reason. You can encounter three types of warning or error messages:

  * **Compiler errors** are simply caused by wrong syntax: Mistyping something, forgetting a bracket or semicolon, or calling a function the wrong way. Even experienced programmers see those errors all the time. The script name and line in question is displayed in the error message, so the error is easy to fix. Some syntax errors cause error messages in subsequent lines.  
  

  * **Runtime errors** occur while the script is running, either in the message window, or in a separate message box. If **[watch("?...")](watch.htm)** statements are placed in the script, their argument is displayed at the end of most error messages for identifying the position in the code. Some non-critical runtime errors can be suppressed by calling **ignore(ErrorNumber)**. For instance**, ignore(53);** lets Zorro start a trading session with no Error 053 message even when some assets are unavailable.  
  

  * **Broker messages** are printed with an explamation mark. They come from the broker API, for instance when using a wrong symbol, exceeding some API limit, or trading outside market hours. The error or warning by the broker is displayed and recorded in the Zorro log. In the case of **MT4/MT5** the error or warning messages are recorded in the terminal's experts log.

For dealing with error messages by script, two functions are available:

## error (string Message)

User-supplied function that is called on any runtime error message or any [print/printf](printf.htm) call that contains the word **"Error"** , **"Warning"** , or **"Info"**. This function can be used to sound an [alert](sound.htm) or send an [email](email.htm) about a potential problem. It is not in sync with other functions or I/O operations, so use the [call](call.htm) scheduler when modifying global variables or accessing non-reentrant parts of the broker API. 

## allow (int Number)

Call this function for not generating messages on runtime errors or warnings with the given number. For instance, **allow(73)** suppresses all **Error 073** messages. Not all errors can be suppressed; fatal errors that terminate the script will still generate a message. 

# Compiler errors

### Error in line ...

> Wrong or missing token in the script code. Sometimes the line in question is ok, but some command in the preceding code was incomplete. Examples are an **#ifdef** without an **#endif** , or a missing semicolon in the previous line, or an orphaned **{** **}** bracket in the previous function. Another not-obvious error is declaring a variable or function with the same name as a predefined system variable, such as [Stop](stop.htm) or [Lots](lots.htm). 

### Wrong type ...

> Wrong variable type. You've used a numeric operation with wrong parameter types, for instance an **AND** or **OR** operation with a **float** or **var** type. Or you're using a variable with the same name as a predefined variable - look them up in **variables.h**. Or you've called a function with a **var** although it needs a **series** , or vice versa. Example: "**Wrong type POINTER::DOUBLE** " means that the code expects a **double** (or **var**) and you gave it a **pointer** (or **series**) instead. 

### Pointer expected ...

> Function call with a wrong parameter type, for instance with a **var** when a [series](series.htm) or an **array** is needed.

### Undeclared identifier ...

> The given name or type is unknown. A common reason is an **#include** statement without including **< default.c>** before. **default.c** contains all language definitions, but is only automatically included when nothing else is included.

### Can not translate ... 

> Numerical operation with wrong parameter types, for instance an **AND** or **OR** operation with a **float** or **var** type. In that case, [typecast](aarray.htm) the expression to the correct type.  
> 

# Runtime errors 

For suppressing a non-critical runtime error or warning, call **ignore(ErrorNumber)**. Some warnings are only displayed on a higher [Verbose](verbose.htm) level. 

### Error 010: Invalid trade parameter 

> A [trade](buylong.htm) was entered with an invalid [Stop](stop.htm), [TakeProfit](stop.htm), [Trail](stop.htm), [Margin](lots.htm), or asset parameter that's unusually high, low, or at the wrong side of the current price. Or with an option or futures [contract](contract.htm) was entered with a wrong or missing [Multiplier](contracts.htm), strike, or expiration date. This trade cannot be executed. Wrong price limits can be caused by a bug in the script or by wrong margin cost, pip cost, or leverage parameters in the asset list. If it happens during live trading, an alert box will pop up dependent on [Verbose](verbose.htm) settings. 

### Warning 010: Invalid trade parameter / no price

> A [trade](buylong.htm) was entered with an unknown parameter; for instance an option contract with no price in the historical or live data (call **contractPrice** before entering a trade). In live trading the position will be entered nevertheless, but this message is just to inform that it is not advisable to trade at unknown prices or other parameters.

###  Error 011: xxx called with invalid parameters 

> A function was called with wrong parameters - for instance a price of zero, or a parameter outside its valid range. This is usually caused by a bug in the preceding script code. The function name is included in the message.

### Error / Warning 012: Bad time / bad date / outdated 

> A [date/time function](month.htm) was called with bad data, or the bar with the given time could not be found, or a contract chain had not the current date. 

### Error 013: Invalid expression (...)

> An invalid value or a wrong variable format caused an exception in a standard C function or expression. For instance a division by zero, the square root of a negative number, an error in a string, or a wrong placeholder in a format statement. The reason is normally a [bug](trouble.htm) in the script, but can also be a history or .csv file that is not found or has a wrong format, or a function returning a NaN number. This error is raised by the Windows invalid parameter handler.

### Warning 014: Bar nnn clipped

> Market hours, weekend, holidays, bar offset, time zone, and [BarMode](barmode.htm) parameters contradict each other, causing a wrong start or end time at the given bar. This usually happens when the bar offset enforces bars outside market hours and the **BR_MARKET** flag suppresses such bars at the same tiome. It can also be caused by large gaps in the historical data. The bar is then clipped at the bar period. When using [BarZone](barzone.htm) with daily bars, consider that the bar period is one hour smaller or longer when daylight saving changes - this can cause bars to end sometimes inside, sometimes outside market hours.

### Error 015: Invalid xxx data

> Object or function **xxx** got invalid data, f.i. a wrong parameter for a pointer or string, or data from a non initialized array, or from an invalid mathematical operation such as a division by zero. 

### Error 016: Invalid date

> A YYYYMMDD date has been given in a wrong format.

### Error 017: Bad return value

> A script function returned an invalid value.

### Error 018: Bad name

> A name was too long, too short, or contained spaces or other invalid characters.

### Error 019: Empty function

> A function prototype was defined, but not set to a function body.

### Warning/Error 030: Check dates / price history / order of settings

> The lookback period, simulation period, or price history was not set up correctly. Parameters that affect the bars generation ([StartDate](date.htm), [BarPeriod](barperiod.htm), [BarOffset](barperiod.htm), [LookBack](lookback.htm), [TICKS](mode.htm), [LEAN](mode.htm), etc.) were either inconsistent (f.i. a too short [LookBack](lookback.htm) period for a subsequent indicator call), or changed after the bars were generated, or did not match the historical data. Make sure to set up all parameters and a sufficient lookback period before calling [asset](asset.htm). Use the [PRELOAD](mode.htm) flag for live trading with extremely long lookback periods. 

### Error 031: Price history missing 

> The simulation could not be finished because historical data was missing. 

### Error 032: Constant parameter changed

> A variable that should remain constant - such as [NumWFOCyles](numwfocycles.htm) \- was changed after the initial run. Make sure not to change those parameters at runtime.

### Warning 033: Asset missing in INITRUN

> The given asset was not selected in the initial run of the strategy. The initial [asset](asset.htm) call was either missing, or skipped, or failed due to insufficient historical data. The asset parameters are filled with default data. Make sure that all assets are loaded and initialized before they are needed. 

### Warning 034: No asset data for ...

> The given asset is missing in the [asset list](account.htm) (note that asset names are case sensitive). The simulation will still run when historical price data is found, but [asset parameters](pip.htm) such as spread, margin, lot size, trade costs, trade profit etc. are made up and do not reflect the true values. The asset can not be traded. 

### Warning/Error 035: Price gap / invalid ticks / outliers detected

> Prices are missing, outliers, or have invalid time stamps. The broker API did not provide the required historical data, or did not send the current price during the last bar. The missing prices will be automatically replaced with the last valid price. If the warning is triggered by heavy price fluctuations in downloaded data, reduce the sensitivity of the [Outlier](outlier.htm) variable. 

### Error 036: Index exceeded

> A function wrote past the maximum index of an array or a [series](series.htm).

### Error 037: Invalid value

> A [plot](plot.htm) function was called with an invalid value, caused by a division by zero, the square root of a negative number, or similar errors in the script. See [Troubleshooting](trouble.htm) for working around such errors.

### Error 038: Too many elements 

> The [plotGraph](plot.htm) with the given name had more elements that bars exist on the chart. If you need this many, use another name for the rest of the elements. 

### Error 039: Expression too complex 

> The rule generated by a [machine learning function](advisor.htm) is too complex for conversion to a C expression. In case of candlestick patterns, use the **FAST** pattern detection mode that generates simpler rules, or reduce the number of patterns. 

### Error 040: Skipped optimize calls

> Number or order of [optimized parameters](optimize.htm) differ from run to run. Make sure that [optimize](optimize.htm) is always called in the same order, the number of **optimize** calls is the always same as the number of optimized parameters, and the [PARAMETERS](mode.htm) flag is not changed between **optimize** calls. 

### Error 041: series size / number / usage 

> Something was wrong with a [series](series.htm), [loop](loop.htm), or [matrix](matrix.htm) call in your script. They are special functions that cannot be arbitrarily called. Series might have been called in wrong order or with different size, or **loop** might have been called outside the **run** function. Check all such calls and all indicators or functions that are described to create series, such as **Volatility** , **ATR** , **LowPass** , etc. Any [run](run.htm) must have the same series with the same size in the same order. Don't skip **series** calls with **[if](if.htm)** statements, don't create them in event-triggered functions such as [tmf](trade.htm), [tick](tick.htm), or [tock](tick.htm), and don't chnage the size of a series. The error message tells you at which bar the problem occured first. If you need an extremely large number of series, increase [TradesPerBar](lookback.htm) until the error message disappears. 

### Error 042: No parameters

### Error 043: Irregular optimize calls

> The [optimize](optimize.htm) function was called too often per component, or the produced parameters were not found or are inconsisted with the current strategy. Make sure that the [optimize](optimize.htm) calls or the [loop](loop.htm) parameters were not changed after training. For asset-dependent parameters, make sure that all assets and algos are selected with [loop](loop.htm) calls and their **optimize** calls are inside the inner loop. For asset-independent parameters in a portfolio system, make sure that always the same asset is selected before the **optimize** call.

### Error 044: No rule ... / ... not trained

> The [advise](advisor.htm) or [optimize](optimize.htm) function could not find a setup or a trained rule, model, or parameter for the current asset/algo combination. Either it did not enter any trades, or something was wrong with the setup order in your script. Make sure that prior to any [advise](advisor.htm) or [optimize](optimize.htm) call, the [asset](asset.htm) and [algo](algo.htm) (if any) was selected, the **PARAMETERS** or **RULES** flag was set, and [TrainMode](opt.htm) was set up. Then [Train] again. 

### Error 045: Negative price offset

> A price that lies in the future was requested by a [price](price.htm) or [day](day.htm) function. For the simulation you can suppress this message by setting the [PEEK](mode.htm) flag. **PEEK** is not available for real trading though - Zorro is good in predicting prices, but not this good. 

### Error 046: LookBack exceeded 

> A [price](price.htm), [series](series.htm), or [TA function](ta.htm) required a higher lookback period than reserved through [LookBack](lookback.htm) (default = **80**). When no asset is selected and time periods don't change at runtime, **Lookback** is automatically adapted to the longest indicator time period of the initial run. Otherwise it should be manually set to the longest possible time period. Setting **LookBack** to **0** , or calling **ignore(46)** allows series longer than the lookback period and suppresses this error message at user's risk.

### Warning / Error 047: Not enough bars 

> Historical price data was insufficient for covering the test period or the [LookBack](lookback.htm) period. Possible reasons:
> 
>   * An invalid symbol was selected. Check your asset list. 
>   * Historical price data was missing, had gaps, or had insufficient resolution, like EOD data for 1-hour bars. Fix the data.
>   * The broker server delivered insufficient history (especially with MT4). Download price data from other sources, and/or use [PRELOAD](mode.htm).
>   * A wrong test or lookback period was set up in the script (see [asset](asset.htm)). Check dates and periods in the script.
>   * The historical data had large gaps or overlapping periods, possibly due to broken downloads. Delete it and download it again, 
>   * Bars are skipped due to weekend, holiday, or outside market hours. Check if the asset is traded around the clock or at weekends (such as cryptocurrencies), and set [BarMode](barmode.htm) accordingly. 
>   * A too high [MinutesPerDay](lookback.htm) value for one of the used assets.
>   * A a user-defined [bar](bar.htm) type is much larger than the given [BarPeriod](barperiod.htm). Decrease [BarPeriod](barperiod.htm) or increase [ MinutesPerDay](lookback.htm) for allocating more bars.
> 

> 
> Make sure that used assets are selected in the first run, and that historical data - when needed - is sufficient for the backtest plus lookback period and is of the same type for all assets. Don't use a mix of .t1, .t2, and .t6 data, or a part of the data split into years and another part not. You can download price data from online sources with the Download script. Frequently used data is also available on the Zorro Download page. If price data for a certain year is not available, create a .t6 file of 0 bytes size with the name of the asset and year (f.i. **SPX500_2017.t6**). Zorro will then skip that period in the simulation and not display the error message.

### Error 048: Endless loop or recursion 

> Trades are possibly generated in an endless loop or recursion, f.i. by a [TMF](trade.htm) that enters trades who themselves enter new trades through their **TMF**. Or a [for(trades)](fortrades.htm) loop is nested inside another **for(trades)** loop. 

### Error 049: Too many trades

> The script entered more than one trade per bar and asset, which is normally a sign of a script bug - therefore the error message. If you really need a huge number of trades, for instance for unlimited grid trading or for training several [advise](advisor.htm) functions at the same time, set the **[TradesPerBar](lookback.htm)** variable to the required number of trades per bar and asset. 

### Warning 050: Trade closed / not found

> The trade from the previous session cannot be resumed. It was either opened with a different algo version, or on a different account, or closed on the broker side, for instance due to liquidation by insufficient margin. It's also possible that the trade got 'orphaned' due to an ID change or a glitch on the broker server. You can normally continue the trading session, but check the trade in your broker's platform. You can identify it from the last 5 digits of the trade ID. If it is still open, close it manually. If it was a pool trade in [virtual hedging](hedge.htm) mode, it will be automatically re-opened at the next trade entry.

### Error 051: Can't continue trades

> A trading session was stopped without closing the open trades, then resumed with a different Zorro version or script version. Zorro can only continue trades that the same version has opened. Use the broker platform for closing the trades manually. 

### Error 052: Broker interface busy

> Zorro attempted to log in to the broker, but another Zorro instance on the same PC is already connected. For multiple trading sessions and accounts, [Zorro S](restrictions.htm) is required. 

### Error 053: ... prices unavailable / invalid asset

> The script failed to retrieve prices. The selected asset does not exist, or is not available, or had a wrong [symbol](symbol.htm), or had no prices due to market closure, an offline server, no [market subscription](ib.htm), an unsupported [price type](brokercommand.htm), or for other reasons. A checklist:
> 
>   * What's the error message from the broker API? It normally tells if a symbol is wrong, ambiguous, or unavailable. If the symbol is correct, but the price type is wrong, you might get no broker error message.
>   * Is the market is open and the broker server online? Sometimes servers are offline due to maintenance at a certain time of day or on weekends. No problem during a trading session, but at startup.
>   * Have you set the correct [price type](brokercommand.htm)? Some assets have only ask/bid prices, others have only trade prices.
>   * Do you have all needed market data subscriptions? Some brokers require a subscription not only for market snapshots, but also for streaming data. 
>   * Is the asset visible and accessible in the watchlist / market list of the broker platform? Some assets of some brokers, f.i. IB or MT4, might be only available via API when previously entered in the watchlist. 
> 

### Warning / Error 054: asset parameter 

> One or several parameters in the [asset list](account.htm) do not match the values returned from the broker API. If it happens in [Train] or [Test] mode, it's an invalid value, and you need to edit the [asset list](account.htm) and fix it. If it happens in [Trade] mode, the asset parameter returned from the broker API differed by more than 50% from their values in the asset list, of from values previously received. If you are sure that the asset parameters in your list are correct, but the broker API got them wrong, you can override the API with the [SET_PATCH](brokercommand.htm) command or [ BrokerPatch](zsystems.htm) setting. MT4/MT5 servers can temporarily return zero values for previously unused assets (see [remarks](mt4plugin.htm#remarks) about MT4 issues). In this case, simply start the script again. If the displayed value is substantially different to the value in the asset list, backtests won't reflect the connected account, so check and correct the asset list. 

### Error 056: ... can't download / no data

> Historical price data could not be downloaded from the broker's price server or from an [online source](loadhistory.htm). The server can be offline or does not offer price data for the given asset and time period. For brokers that require market data subscriptions, check if you've subscribed the assets to be downloaded. Errors from online sources are usually due to an unsupported symbol or to exceeding a download limit. The full response can be found in the downloaded **History\history.csv** or **history.json** file.

### Error 057: inconsistent history format 

> Check if the historical data is consistent (no mix of different **.t1** , **.t6** , or **.t8** files) and has similar resolution for all used assets. Second or tick resolution is needed for testing with bar periods less than one minute.

### Error 058: ... can't parse

> A CSV [dataset](data.htm) can not be parsed due to an invalid file format. If the file was recently downloaded (f.i. **history.csv**), open it with a text editor - it might contain no CSV data, but an error message from the data provider. Otherwise compare the CSV file content with your conversion [format string](data.htm). If in doubt, set **Verbose** to 7 for checking the source and parse result of the first 2 lines - this normally reveals the format error. The error details:  
>   
> **Bad code** \- your format string contained an invalid field placeholder.  
> **Bad date** \- the date code in your format string does not match the date format in the file.  
> **Bad field number** \- your format string contained invalid field numbers.

### Error 059: ... invalid format

> An [asset list](account.htm), [account list](account.htm), or [panel definition](panel.htm) could not be read due to an invalid parameter or format. Check if it has valid CSV format and that no field is empty or contains invalid content. Every line in a .csv file must have the same number of delimiters - all commas or all semicolons - and end with a 'new line' character. 

### Error 060: Out of memory / Memory fragmented / limit exceeded 

> Available memory is insufficient for your script. Windows can assign 3 GB memory to a 32-bit process, and all free memory to a 64-bit process. If that size is exceeded, or if the memory became too fragmented by previous simulation or training runs, the memory required by the script cannot be allocated. Some methods to overcome the problem or to reduce the memory requirement:
> 
>   * In case of fragmented memory, simply restart Zorro for getting a fresh contiguous memory area. 
>   * Reduce the number of assets by splitting large asset lists and testing asset subsets in separate runs.
>   * Reduce the simulation period by setting a later [StartDate](date.htm), an earlier [EndDate](date.htm), or a [MaxBars](date.htm) limit.
>   * Don't use the [TICKS](mode.htm) flag. Or increase [TickTime](ticktime.htm) before selecting the asset.
>   * Use the [LEAN](mode.htm) and [LEANER](mode.htm) flags, and M1 rather than T1 price data.
>   * On short bar periods in the minute or second range, avoid too many [plot](plot.htm) commands.
>   * When your script needs more than 3 GB data: Use [Zorro64](vps.htm) and a [C++ script](dlls.htm)
> 

> 
> A formula for calculating the memory requirement per asset can be found under [asset](asset.htm). Usually, the memory on a standard Windows PC is sufficient even with large high-resolution data files in 64 bit mode. 

### Error 061: Incompatible / Can't compile ... 

> The executable script could not be compiled, or was compiled with an incompatible version, like 32 bit vs 64 bit, or is missing a VC++ library on your PC. Delete the outdated **.x** or **.dll** file (if any), make sure that you entered the correct VC++ compiler path in [Zorro.ini](ini.htm), and compile it again. For running compiled C++ code in 64 bit mode, install the **VC++ 64-bit redistributable package** that is available either from Microsoft or from [our server](http://opserver.de/down/VC_redist.x64.exe). 

### Error 062: Can't open file ... 

> A file was not found, could not be opened, or had wrong format. Make sure that the file exists, that it has the correct name, that the Zorro process has access rights to it, and that it is not opened with exclusive access in another application (f.i. a **.csv** file in Excel). If a Windows error number is given, f.i. **(wb:13)** , the reason of the error can be found in the list below.  |   
> ---|---  
> **2** |  No such file or directory  
> **9** |  Bad file number  
> **11, 12** |  Not enough memory or resources  
> **13** |  No access rights, or file opened in another application  
> **16** |  Device or resource busy  
> **23, 24** |  Too many files open in system  
> **28** |  No space left on device  
> **30** |  Read-only file system  
> **38** |  Filename too long  
>   
> If the missing file is a **.par** , **.c** , **.fac** , or **.ml** file, either the script was not yet [trained](training.htm), or the file had a wrong name. The usual reason is selecting a different asset, or no asset at all, before an [optimize](optimize.htm) or [advise](advisor.htm) call. Parameter and rule files will then get inconsistent names in test and training and therefore cannot be read.

### Error 063: ... not available

> The function is not available in this script because it requires a different setup, an external plugin, or [Zorro S](restrictions.htm). Make sure that you installed your personal Zorro S key or token.

### Warning 070: Potential orphan

> Zorro sent a trade order to the broker API, but received no response. This can happen due to a server glitch or connection issue right at or after sending the order. It is then unknown whether the position was opened or not. Zorro will assume that the order was rejected. But if it was in fact executed, the trade is orphaned and not anymore under Zorro control.   
>  Check in the broker platform if the position was opened. If so, close it manually. If orphaned trades occur frequently, check your Internet connection and/or connect - if possible - to a different server of your broker. Some Forex brokers, such as [FXCM](fxcm.htm), are known for occasional orphaned trades.

### Error 071: Trade not found ... 

> Zorro closed the trade with the given ID, but the trade was not found in the broker's trade lists. Possibly it was never opened, or was already closed manually or by a margin call or similar event. 

### Error 072: Login failed ... 

> A broker connection could not be established. The broker server could be offline or the broker API nonfunctional. 

### Warning 073: Can't close nn lots 

> An [exitShort/exitLong](selllong.htm) command for partial closing could not be fully executed because not enough trades were open to close the given number of lots. When partial closing, make sure not to close more lots than are open.

### Error 074: Balance limit exceeded

> You need [Zorro S](restrictions.htm) when your annual profit or your balance exceeds the real account limits of the free version. There are no restrictions on demo or paper accounts.

### Warning 075: Can't open / can't close / trade removed...

> A trade order was not executed by the broker API during a live trading session, and subsequently cancelled. The reason is normally printed either to the Zorro log, or to the MT4/MT5 'Experts' log. Valid reasons for not excuting orders are: not enough funds, not enough free margin, no permission to trade that asset, a too distant or too close stop or order limit, a wrong asset symbol, a wrong or missing exchange symbol, a closed market, a broker server issue, no liquidity or no taker, FIFO violation, NFA rules violation, unsupported hedging (f.i. Oanda), or unsupported partial closing (f.i. some MT4 / MT5 brokers). All this, except for funds, liquidity, and permissions, can often be solved or worked around in the script, the asset list, or in case of a Z system, in the **Z.ini** configuration.   
>  If a close order is not executed, it will be automatically repeated in increasing intervals until the position is closed. If it is still not closed after 3 working days, the trade will be removed and no further close orders will be sent. If an open order is rejected, it is not automatically repeated, so it's up to the script to either ignore or handle it.

### Error 075: Check NFA flag

> You're trading with a NFA broker, but the [NFA flag](mode.htm) in your script or [account list](account.htm) is wrong or missing. Stopping or closing trades is prohibited on the broker side due to NFA compliance.

### Error 080: Python / R code

Python or R code could not be executed due to a syntax error or for other reasons.

### Error 111: Crash in ... 

> A function in the script crashed due to a wrong operation. Examples are a division by zero, a wrong type or missing variable in a **print/printf** call, a wrong array index, or exceeding the stack size by declaring huge local arrays. The current [run](run.htm) is aborted, possibly causing subsequent errors, f.i. inconsistent series calls. The name of the faulty function is normally displayed. See [Troubleshooting](trouble.htm) about how to fix bugs in your script functions or deal with other sorts of crashes. 

# Broker errors and messages

### !... 

> All messages beginning with an exclamation mark are sent from the broker API in a [live trading](trading.htm) session, so their content depends on the broker. On high [Verbose](verbose.htm) settings, some diagnostics can be printed on events such as session start. Other messages can indicate that the connection was temporarily interrupted or a buy/sell order was rejected (see [enter](buylong.htm)/[exit](selllong.htm)). This happens when assets are traded outside their market hours, such as an UK stock index when the London stock exchange is closed. A "**Can't close trade** " message can also indicate that the [NFA](mode.htm#nfa) flag was not set on a NFA compliant account, or vice versa. Read the comments on the broker related page ([FXCM](fxcm.htm), [IB](ib.htm), [Oanda](oanda.htm), [MTR4](mt4plugin.htm), ...) in this manual. When trading with the [MT4/5 bridge](mt4plugin.htm), details and reason of error message is printed under the [Experts] and [Journal] tabs of the MTR4 platform.   
>  If a trade cannot be opened, Zorro will not attempt to open it again unless either enforced by the script, or when it's a [pool trade](hedge.htm). Pool trades will be rebalanced at any bar until they match the virtual trades. If a trade cannot be closed, Zorro will attempt to close it again in increasing intervals. If it is still not closed after 4 days, Zorro will assume that it was manually closed, and make no further attempts.

### 

****[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
