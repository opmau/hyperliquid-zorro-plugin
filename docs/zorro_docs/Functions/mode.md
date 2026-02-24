---
title: Mode
url: https://zorro-project.com/manual/en/mode.htm
category: Functions
subcategory: None
related_pages:
- [enterLong, enterShort](buylong.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [Status flags](is.htm)
- [price, ...](price.htm)
- [adviseLong, adviseShort](advisor.htm)
- [watch](watch.htm)
- [series](series.htm)
- [bar](bar.htm)
- [BarMode](barmode.htm)
- [Dates, holidays, market](date.htm)
- [Error Messages](errors.htm)
- [Oversampling](numsamplecycles.htm)
- [Trade Statistics](winloss.htm)
- [Performance Report](performance.htm)
- [trade management](trade.htm)
- [tick, tock](tick.htm)
- [Historical Data](history.htm)
- [asset, assetList, ...](asset.htm)
- [Indicators](ta.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Included Scripts](scripts.htm)
- [assetHistory](loadhistory.htm)
- [Multiple cycles](numtotalcycles.htm)
- [optimize](optimize.htm)
- [Money Management](optimalf.htm)
- [TrainMode](opt.htm)
- [Visual Studio](dlls.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Spread, Commission, ...](spread.htm)
- [brokerCommand](brokercommand.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [Virtual Hedging](hedge.htm)
- [IB / TWS Bridge](ib.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Asset and Account Lists](account.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Licenses](restrictions.htm)
- [Command Line Options](command.htm)
- [Zorro.ini Setup](ini.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Export](export.htm)
- [System strings](script.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [plot, plotBar, ...](plot.htm)
- [setf, resf, isf](setf.htm)
---

# Mode

# System Flags 

Zorro's general behavior can be set up through **system flags** \- that are internal "switches" that can be either set (activated) or reset (deactivated). By default, they are normally off. Flags that affect the bar generation, such as **TICKS** , must be set in the initial run at start of the script before selecting any assets. The other flags can be set or reset at any point. Flags are set, reset, or read with the following functions: 

## set (int Flag1 _, int Flag2..., int Flag10_)

Sets or resets the given system flags (up to 10). A flags is reset by adding **|OFF** to the flag name. For instance, **set(PARAMETERS,TICKS|OFF);** sets the **PARAMETERS** flag and resets the **TICKS** flag.

## is (int Flag): int 

Returns nonzero (or **true**) when the given system flag is set, otherwise zero or **false**.   

The main system flags:

## SKIP1

## SKIP2

## SKIP3

Do not [enter](buylong.htm) trades in the first, second, or third of every 3 weeks of the historical price data (the period can be set up with [DataSkip](dataslope.htm)). These flags can be used for out-of-sample testing by separating test data from training data while still covering the same time period. These flags must be set in the [INITRUN](is.htm).  
  

## PEEK

Allow peeking in the future through negative [price](price.htm) function offsets; [Test] and [Train] mode only. Sometimes required for [advise](advisor.htm) functions and price difference analysis. 

## NOLOCK

Don't synchronize the API and file accesses of multiple trading Zorro instances, to prevent that they interfere with each other. Set this flag for speeding up API access when the trading Zorros don't share the broker API, or when the broker API allows simultaneous sccesses from multiple sources (MT4/MT5 does not). This flag must be set in the [INITRUN](is.htm). 

## NOWATCH

Ignore all [watch](watch.htm) statements. Can be set for trading a script that is full of watches. 

## NOSHIFT

Do not shift [dynamic series](series.htm) and do not change their first element on **series()** calls. Useful for shifting series only under special conditions, f.i. for emulating [special bars](bar.htm) or for not updating indicators outside market hours.

## NOFLAT

Do not shift [dynamic series](series.htm) and do not change their first element on **series()** calls when the current asset has a flat bar, i.e. all prices are the same. Useful for skipping gaps in the historical data when [BR_FLAT](barmode.htm) was set. 

## NOFACTORS

Don't automatically calculate **OptimalF** factors in test and training. Useful when OptimalF calculation takes a long time due to a large number of trades. For training script-generated factors with a user-defined algorithm, set both **FACTORS** and **NOFACTORS** flags and set **OptimalF** , **OptimalFLong** , **OptimalFShort** to a script calculated value in the **[FACCYCLE](is.htm)** training run (**if(is(FACCYCLE)) ...**). 

## COMMONSTART

Delay the simulation until price history of all assets is available. Otherwise the simulation starts either at the [StartDate](date.htm) or at the history begin of the first asset, whichever is later, and missing price history of other assets produce an [Error 055](errors.htm) message. This flag must be set in the [INITRUN](is.htm). 

## OPENEND

Do not close the remaining open trades at the end of a simulation cycle; leave them open and ignore their results. This has two purposes. In [Test] mode it eliminates the effect of prematurely closed trades on the performance report. In [Train] mode it prevents that parameters or rules are affected by the results of those trades. 

## ALLCYCLES

Do not reset the statistics values inside the **STATUS** structs when a new [sample cycle](numsamplecycles.htm) is started. The [Long/Short statistics](winloss.htm) and the [portfolio analysis](performance.htm) are then the result of all sample cycles so far. Print any sample cycle to the log; otherwise only the last cycle is logged. This flag is always set in [Train] mode. This flag must be set in the [INITRUN](is.htm).  

##  TICKS 

Tick-precise intrabar simulation. This flag must be set in the [ INITRUN](is.htm). If it is set, not only the Open, Close, High, and Low, but also the price curve inside any bar is evaluated for triggering entry, stop, trail, or takeprofit limits. [TMFs](trade.htm) are run on every [tick](bars.htm#tick) in the simulation, instead of only once per bar. This flag gives a more accurate simulation result, but also requires more time for a simulation cycle, and allocates more memory.   
If this flag is not set, an intra-bar approximation is used for simulating entry and exit. In this approximation, a stop loss is always triggered earlier than a profit target, resulting in a slightly pessimistic simulation. Trades closed by [TMFs](trade.htm) are sold at the open price of the next bar. This less accurate simulation is sufficient in most cases, but **TICKS** should be used when trades enter and exit at the same bar, when stop loss, or takeprofit distances are small, or when trading is strongly affected by [tick](tick.htm) functions or [TMFs](trade.htm). 

## LEAN

Use compressed historical data. [marketVal](price.htm) and [marketVol](price.htm) are not available, [.t1 data](history.htm) is stored without bid prices, and the open/close price of a bar is approximated by the center point of its first and last tick (except for EOD historical data). This flag can reduce memory requirement when market volume and candles are not needed by the strategy, and the historical data has a much higher resolution than one bar period, f.i. M1 data with 1-hour bars. This flag reduces the memory requirement for backtests by 30%. It must be set in the [INITRUN](is.htm) before calling [asset()](asset.htm). 

## LEANER

Use only the Close from historical price data; disregard Open, High, Low, and Mean. Indicators that require full candles, such as [ATR](ta.htm) or [TypPrice](ta.htm), can not be used. This flag is useful for strategies with hundreds or thousands of stocks, and reduces in combination with the **LEAN** flag the memory requirement by 50%. It must be set in the [INITRUN](is.htm) before calling [asset()](asset.htm). 

## RECALCULATE

Run a full [LookBack](lookback.htm) period at the begin of every WFO cycle in [Test] mode. Discard the content of all [series](series.htm) at lookback start, and recalculate them from the parameters and rules of the new cycle.  Otherwise the series keep their values from the previous test cycle.  **RECALCULATE** increases the test time, but simulates the start behavior of a new trading session a bit more realistically. This flag must be set in the [INITRUN](is.htm). See also [ DataHorizon](dataslope.htm).

## PRELOAD

Causes historical data to be read at any script start. In [Trade] mode, data from the history files is then used for the [LookBack](lookback.htm) period, instead of data from the broker API or from external data sources. If the historical data is not sufficient for filling the complete lookback period, the remaining data between the end of the history and the current time is still loaded from the broker or data source. This flag reduces the session start time of systems with long lookback periods, and overcomes history limitations of broker servers. Recent price history files from the current and the last year must be available in sufficient resolution and preferably from the same broker or data source. [LookBackResolution](lookback.htm) can be used to enforce the same resolution for pre-loaded and live historical data. Use the [Download](scripts.htm) script or the [ assetHistory](loadhistory.htm) function for getting the most recent data. This flag must be set in the [INITRUN](is.htm).  
In [Test] mode, this flag causes historical data to be read again at any script start and for any further [simulation cycle](numtotalcycles.htm). This delays the start, but allows cycles with shuffled or otherwise manipulated price data, for instance for generating reality check histograms. Otherwise historical data is only read at the first script start or when the [Edit] button was clicked.  

## PARAMETERS

[Train] mode: generate strategy parameters with [optimize](optimize.htm) calls and store them in **Data/*.par**. This flag must be set before calling [optimize](optimize.htm). If this flag is not set, parameters are not generated in the training run, but still loaded from previously generated ***.par** files.  
[Test] / [Trade] mode: load optimized parameters. If this flag is not set, the default parameters from the [optimize](optimize.htm) calls are used. 

## FACTORS

[Train] mode: generate [OptimalF](optimalf.htm) capital allocation factors and store them in **Data/*.fac**. [Test] / [Trade] mode: load **OptimalF** factors for allocating capital. If this flag is not set, all **OptimalF** factors are **1**. This flag must be set before the first [asset](asset.htm) call.  
**OptimalF** factors are normally generated from the whole backtest period. For generating them individually per WFO cycle, use the **[ALLCYCLES](opt.htm)** training mode. 

## RULES

[Train] mode: use the [advise](advisor.htm) machine learning functions to generate trade rules or machine learning models, and store them in **Data/*.c** or **Data/*.ml**. This flag must be set before calling [advise](advisor.htm). Otherwise rules or models are not generated, but loaded from previously generated ***.c** or ***.ml** files.  
[Test] / [Trade] mode: load trade rules or machine learnign models and use them in the **advise** functions. If this flag is not set, the **advise** functions always return the value **100**. 

## VCPP

Use the [Visual Studio C++ compiler](dlls.htm), rather than the lite-C compiler, for compiling trade rules with the **RULES** flag. Default for the 64-bit version.  

## MARGINLIMIT

Don't enter a [trade](buylong.htm) when even the minimum amount of 1 [Lot](lots.htm) exceeds twice the used [Margin](lots.htm) value, or when the trade margin plus the trade risk exceeds the account balance. Trades skipped due to too-high risk or too-low balance are indicated in the log. This flag has no effect in training mode or for [phantom trades](lots.htm) that are always executed regardless of the margin. 

## RISKLIMIT

Don't enter a trade when even with the minimum amount of 1 [Lot](lots.htm), the trade risk is still higher than twice the than the allowed [Risk](lots.htm). Also don't enter a new trade in [Trade] mode when the total risk of all open trades exceeds the available margin left in the account. Setting this flag can reduce profit, as trades with a high stop loss distance are often profitable trades. Trades skipped due to too-high risk or too-low account are indicated in the log. This flag has no effect in training mode or for [phantom trades](lots.htm) that are always executed regardless of the risk. 

## ACCUMULATE

Accumulate the [Margin](lots.htm) of trades skipped by **MARGINLIMIT** or **RISKLIMIT** , until the accumulated margin is high enough to overcome the limits. The trade is then executed and the accumulated margin is reset. This allows trading - although less frequently - with very small capital that would otherwise result in trade sizes of less than one lot. This flag has no effect in training mode**.**   

## BINARY

Simulate binary options for training and testing. This flag must be set in the [INITRUN](is.htm). In binary mode the trade profit is not proportional to the price difference between entry and exit, but determined by the [WinPayout](spread.htm) or [LossPayout](spread.htm) of the selected asset. The stake can be set with the [Margin](lots.htm) variable for the backtest; in live trading the stake is set with the [Lots](lots.htm) variable or with an [order comment](brokercommand.htm), dependent on the broker. Rollover and commission are ignored for binary trades. [Spread](spread.htm) is **0** by default in binary mode, but can be set to a nonzero value for simulating an additional disadvantage. The trade time can be set through [ExitTime](timewait.htm). Stop loss and profit targets are normally not set for binary trades, but can be used for betting on negative or positive excursions in special binary modes. Hedging should be enabled by **[Hedge](hedge.htm) = 2** for preventing the premature close of opposite trades. 

## NFA

Activate NFA Rule 2-43(b) compliant trading. This is required for most US based brokers, such as [IB](ib.htm). Zorro handles NFA compliant trading in a transparent way, so a strategy normally needs not be modified for NFA compliance. When the NFA flag is set, the following NFA restrictions are observed and worked around:

  * NFA brokers support only positions, but no individual trades. Trades are managed on the Zorro side.
  * Closing trades is not permitted. The [exit](selllong.htm) functions open an opposite trade instead.
  * Stopping out trades is not permitted. Stops are managed on the Zorro side and open an opposite trade. 
  * Simultaneous long and short positions are not permitted. On the Zorro side they are still possible with [virtual hedging](hedge.htm). 
  * First opened positions must be closed first (FIFO compliance). [Virtual hedging](hedge.htm) takes also care of that.

The **NFA** flag must be set in the [INITRUN](is.htm). It is automatically set when the selected account has a nonzero [NFA parameter](account.htm) in the account list. Do not set this flag for non-NFA compliant accounts. You can find out if your account is NFA compliant by manually opening a long and short position of the same asset in the broker platform. If two positions are then really open, your account is not NFA compliant and you must not set the **NFA** flag. If the short position cancels the long one, your account is NFA compliant and the **NFA** flag must be set. [MT4/5 accounts](mt4plugin.htm) are normally not NFA compliant even when the broker is located in the US; but they can be FIFO compliant and require exiting trades in the order of their entry.  

## EXE

Do not run the simulation, but compile the script to an executable in **.x** file format. Executable scripts start faster and can be used for distributing strategies without revealing the source code. If a **.c** and a **.x** script with the same name are found in the **Strategy** folder, the **.c** script has priority. [Zorro S](restrictions.htm) required. The **EXE** flag must be set in the [INITRUN](is.htm); it can also be set by [command line](command.htm) or with the [Action] scrollbox.

## AUTOCOMPILE

Unless the script was changed, do not compile it again and do not reset sliders and global variables on subsequent runs. This flag is off by default so that global variables are always reset at start, but can also be set in [Zorro.ini](ini.htm). 

## STEPWISE

Do not run, but single step through the simulation in [Test] mode. A click on [Step] moves one bar forward. [Skip] moves to the next opening or closing a position. The current chart and trade status will be displayed on every step in a browser window. For details see [debugging](chart.htm). 

## STRAIGHT

Don't correct or check the lookback period and switch the [ta-lib indicators](ta.htm) from time-descending [series](series.htm) mode to time-ascending array mode. In this mode they accept any data array of sufficient length and can be used even when series are disabled.   

## LOGFILE

Generate and export [logs and statistics files](testing.htm#files) in the **Log** and **Data** folders, dependent on [Test], [Train], or [Trade] mode. This flag affects the backtest speed.It is always set in [Trade] mode. When [LogNumber](numtotalcycles.htm) is set, the number is appended to the filenames, which allows different log files from the same script for comparing or appending. This flag must be set in the [INITRUN](is.htm). 

## EXPORTED

Export charts and training curves in the **Log** and **Data** folders, dependent on [Test], [Train], or [Trade] mode (see [export](export.htm)). This flag must be set in the [INITRUN](is.htm). 

## BALANCE

Use balance rather than equity for the profit curve in the chart, for the monthly profit table in the [performance report](performance.htm), and for the values in the exported [profit/loss curves](script.htm).  This flag must be set in the [INITRUN](is.htm). 

## PIPRETURN

Use volume-independent returns in [pip](pip.htm) units for the profit curve in the chart, for the monthly profit table in the [performance report](performance.htm), and for the values in the exported [profit/loss curve](script.htm).  This flag must be set in the [INITRUN](is.htm).

## MAECAPITAL

Do not calculate the required capital, annual return, and the 'underwater profile' on the chart from maximum drawdown, but from maximum adverse excursion (see [performance report](performance.htm) for the difference). This is for comparing with results of other platforms and normally produces slightly higher required capital. This flag must be set in the [INITRUN](is.htm).  

## TESTNOW

Run a test immediately after training, without clicking [Test]. If the simulation is repeated multiple times by setting [NumTotalCycles](numtotalcycles.htm), **TESTNOW** causes the price curve to be generated anew at the begin of every cycle. This is useful for testing different bar offsets or detrend modes. Since the test uses the settings from the previous training, its result can differ from a normal test when static variables or parameters are different. 

## PLOTNOW

Plot a chart immediately after testing, without clicking [Result]. Automatically set when the script plots a histogram with [plotBar](plot.htm). This flag must be set in the [INITRUN](is.htm).   

### Remarks

  * In old scripts, flag combinations like **set(PARAMETERS|TICKS)** were used for setting several flags, **reset(TICKS)** for switching off a flag, and **mode(TICKS)** for checking the flag state. These methods are still supported, but for new scripts use **set(PARAMETERS,TICKS)** , **set(TICKS|OFF)** , and **is(TICKS)**.

### Example:

```c
function run()
{
  if(Train) set(SKIP3); // leave every 3rd week for the test
  if(Test) set(SKIP1,SKIP2,TICKS,FAST,LOGFILE); 
  ...
}
```

### See also:

[DataSkip](dataslope.htm), [Status flags](is.htm), [setf](setf.htm), [Zorro.ini](ini.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
