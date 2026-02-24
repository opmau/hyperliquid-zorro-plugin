---
title: Test mode
url: https://zorro-project.com/manual/en/testing.htm
category: Main Topics
subcategory: None
related_pages:
- [Historical Data](history.htm)
- [Zorro.ini Setup](ini.htm)
- [Asset and Account Lists](account.htm)
- [Spread, Commission, ...](spread.htm)
- [Oversampling](numsamplecycles.htm)
- [Monte Carlo Analysis](montecarlo.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [set, is](mode.htm)
- [Performance Report](performance.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Live Trading](trading.htm)
- [printf, print, msg](printf.htm)
- [Verbose](verbose.htm)
- [plot, plotBar, ...](plot.htm)
- [CBI](ddscale.htm)
- [Log Messages](log.htm)
- [Export](export.htm)
- [asset, assetList, ...](asset.htm)
- [Multiple cycles](numtotalcycles.htm)
- [assetHistory](loadhistory.htm)
- [Fill modes](fill.htm)
- [NxCore](nxcore.htm)
- [Licenses](restrictions.htm)
- [trade management](trade.htm)
- [tick, tock](tick.htm)
- [System strings](script.htm)
- [price, ...](price.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Error Messages](errors.htm)
- [Money Management](optimalf.htm)
- [Detrend, shuffling](detrend.htm)
- [Training](training.htm)
- [Troubleshooting](trouble.htm)
---

# Test mode

# Testing a Strategy. Reality Check 

[Backtesting](https://zorro-project.com/backtest.php) has the purpose to determine an algo trading strategy's profit expectancy in live trading. For this, the strategy is usually simulated with a couple years of historical price data. However, a simple backtest will often not produce a result that is representative for live trading. Aside from overfitting and other sorts of bias, backtest results can differ from live trading in various ways that should be taken into account. Before going live, the final backtest should be verifies with a Reality Check (see below). Testing a strategy is not a trivial task.

### Testing a script with Zorro 

For quickly testing a strategy, select the script and click [Test]. Depending on the strategy, a test can be finished in less than a second, or it can run for several minutes on complex portfolio strategies. If the test requires strategy parameters, capital allocation factors, or machine-learned trade rules, they must be generated in a [Train] run before. Otherwise Zorro will complain with an error message that the parameters or factors file or the rules code was not found. 

If the script needs to be recompiled, or if it contains any global or static variables that must be reset, click [Edit] before [Test]. This also resets the sliders to their default values. Otherwise the static and global variables and the sliders keep their settings from the previous test run. 

If the test needs historical price data that is not found, you'll get an error message. Price data can be downloaded from your broker, from Internet price providers, or from the Zorro download page. It is stored in the **History** folder in the way described under [Price History](history.htm). If there is no price data for all years in the test period, you can substitute the missing years manually with files of length **0** to prevent the error message. No trades take then place during those years. 

All test results - performance report, log file, trade list, chart - are stored in the **Log** folder. If the folder is cluttered with too many old log files and chart images, every 60 days a dialog will pop up and propose to automatically delete files that are older than 30 days. The minimum number of days can be set up in the [Zorro.ini](ini.htm) configuration file. 

The test simulates a real broker account with a given leverage, spread, rollover, commission, and other asset parameters. By default, a microlot account with 100:1 leverage is simulated. If your account is very different, get your actual account data from the broker API as described under [Download](history.htm), or enter it manually. Spread and rollover costs can have a large influence on backtest results. Since spread changes from minute to minute and rollover from day to day, make sure that they are up to date before backtesting. Look up the current values from the broker and enter them in the [asset list](account.htm). Slippage, spread, rollover, commission, or pip cost can alternatively be set up or calculated in the [script](spread.htm). If the [NFA](mode.htm#nfa) flag is set, either directly in the script or through the [NFA parameter](account.htm) of the selected account, the simulation runs in NFA mode.

The test is not a real historical simulation. It rather simulates trades as if they were entered today, but with a historical price curve. For a real historical simulation, the spread, pip costs, rollovers and other asset and account parameters had to be changed during the simulation according to their historical values. This can be done by script when the data is available, but it is normally not recommended for strategy testing because it would add artifacts to the results. 

The test runs through one or many [sample cycles](numsamplecycles.htm), either with the full historical price data set, or with out-of-sample subsets. It generates a number of equity curves that are then used for a [Monte Carlo Analysis](montecarlo.htm) of the strategy. The optional [Walk Forward Analysis](numwfocycles.htm) applies different parameter sets for staying always out-of-sample. Several switches (see [ Mode](mode.htm)) affect test and data selection. During the test, the progress bar indicates the current position within the test period. The lengths of its green and red area display the current gross win and gross loss of the strategy. The result window below shows some information about the current profit situation, in the following form:

3: +8192 +256 214/299 

****3:**** | Current oversampling cycle (if any).  
---|---  
**+8192** | Current balance.   
**+256** | Current value of open trades.   
**214** | Number of winning trades so far.   
**/299** | Number of losing trades so far.   
  
After the test, a [performance report](performance.htm) and - if the [LOGFILE](mode.htm) flag was set - a log file is generated in the **Log** folder. If [Result] is clicked or the [PLOTNOW](mode.-htm) flag was set, the trades and equity chart is displayed in the [chart viewer](chart.htm). are displayed. The message window displays the annual return (if any) in percent of the required capital, and the average profit/loss per trade in pips. Due to different pip costs of the assets, a portfolio strategy can have a negative average pip return even when the annual return is positive, or vice versa. 

![](../images/testing1.png)

The content of the message window can be copied to the clipboard by double clicking on it. The following performance figures are displayed (for details see [performance report](performance.htm#ar)): 

**Median AR** | Annual return by Monte Carlo analysis at 50% confidence level.   
---|---  
**Profit** | Total profit of the system in units of the account currency.   
**MI** | Average monthly income of the system in units of the account currency.   
**DD** | Maximum balance-equity drawdown in percent of the preceding peak.  
**Capital** | Required initial capital.  
**Trades** | Number of trades in the backtest period.   
**Win** | Percentage of winning trades.   
**Avg** | Average profit/loss of a trade in pips.  
**Bars** | Average number of bars of a trade. Fewer bars mean less exposure to risk.   
**PF** | Profit factor, gross win divided by gross loss.   
**SR** | Sharpe ratio. Should be > 1 for good strategies.   
**UI** | Ulcer index, the average drawdown percentage. Should be < 10% for ulcer prevention.   
**R2** | Determination coefficient, the equity curve linearity. Should be ideally close to 1.  
**AR** | Annual return per margin and drawdown, for non-reinvesting systems with leverage.  
**ROI** | Annual return on investment, for non-reinvesting systems without leverage.  
**CAGR** | Compound annual growth rate, for reinvesting systems.  
  
For replaying or visually debugging a script bar by bar, click [Replay] or [Step] on the chart window. 

### Backtest result files 

When the [LOGFILE](mode.htm) flag is set, a backtest generates the following files for examining the trade behavior or for further evaluation:

  * **Log\\[Scriptname]_[Asset]_ _test.log **\- the main log file with all recorded trade events as described under [ Trading](trading.htm), and all [print](printf.htm) output. Prices in the log are ask prices. The higher the [Verbose](verbose.htm) setting, the more details are recorded. 
  * **Log\\[Scriptname]_[Asset]_ _pnl.csv** \- a spreadsheet with the daily equity or balance curve, depending on the [ BALANCE](mode.htm) flag. 
  * **Log\\[Scriptname]_[Asset]_ _plot.csv** \- a spreadsheet with all curves generated with [plot](plot.htm) commands. 
  * **Data\\[Scriptname]_[Asset]_ _pnl.dbl** \- a file containing a **double** array with the daily balance or equity values in ascending date order. Used for calculating the [Cold Blood Index](ddscale.htm) in live trading.
  * **Log\\[Scriptname]_[Asset]_ _trd.csv** \- a spreadsheet with a list of all trades during the simulation period, their details and their results. 
  * **Log\\[Scriptname]_[Asset]_.txt** \- the [performance report](performance.htm). 
  * **Log\\[Scriptname]_[Asset]_.png** \- an image file with a histogram or chart, depending on [plot](plot.htm) commands. Prices in the chart are ask prices.
  * **[Webfolder]\\[Scriptname]_[Asset]_.htm** \- a web page displaying the trade status at the end of the simulation, as described under [Trading](trading.htm).

The messages in the log file are described under [Log](log.htm). The other file formats are described under [Export](export.htm). The optional asset name is appended when the script does not call [asset](asset.htm), but uses the asset selected with the scrollbox. When [LogNumber](numtotalcycles.htm) is set, the number is appended to the script name, which allows comparing logs from the current and previous backtests. When [Result] is clicked, the chart image and the **plot.csv** are generated again for the asset selected from the scrollbox. The chart is opened in the chart viewer and the log and performance report are opened in the editor. 

### Backtest resolution - TICKS, M1, T1, HFT.. 

The required price history resolution, and whether using [TICKS](mode.htm) mode or not, depends on the strategy to be tested. As a rule of thumb, the duration of the shortest trade should cover at least 20 ticks in the historical data. For long-term systems, such as options trading or portfolio rebalancing, [daily data](loadhistory.htm) as provided from STOOQ™ or Quandl™ is normally sufficient. Day trading strategies need historical data with 1-minute resolution (M1 data, **.t6** files). If entry or exit limits are used and the trades have only a duration of a few bars, [TICKS](mode.htm) mode is mandatory. Scalping strategies that open or close trades in minutes require high resolution price history - normally T1 data in **.t1** or **.t2** files. For backtesting [high frequency trading](fill.htm) systems that must react in microseconds, you'll need order book or BBO (**B** est **B** id and **O** ffer) price quotes with exchange time stamps, available from data vendors in CSV or [NxCore](nxcore.htm) tape format. An example of how to test a HFT system can be found on [Financial Hacker](http://www.financial-hacker.com/hacking-hft-systems/). For testing with T1, BBO, or NxCore data, [ Zorro S](restrictions.htm) is required. 

The file formats are described in the [Price History](history.htm) chapter. In T1 data, any tick represents a price quote. In M1 data, a tick represents one minute. Since the price tick resolution of **.t1** and **.t6** files is not fixed, **.t1** files can theoretically contain M1 data and **.t6** files can contain T1 data - but normally it's the other way around. Since many price quotes can arrive in a single second, T1 data usually contains a lot more price ticks than M1 data. Using T1 data can have a strong effect on the backtest realism, especially when the strategy uses short time frames or small price differences. Trade management functions ([TMFs](trade.htm)) are called more often, the [tick](tick.htm) function is executed more often, and trade entry and exit conditions are simulated with higher precision.

For using T1 historical price data, set the [History](script.htm) string in the script to the ending of the T1 files (for instance **History = "*.t1"**). Zorro will then load its price history from **.t1** files. Make sure that you have downloaded the required files before, either with the **Download** script, or from the Zorro download page. Special streaming data formats such as NxCore can be directly read by script with the [priceQuote](price.htm) function. An example can be found under [Fill mode](fill.htm). 

Naturally, backtests take longer and require more memory with higher data resolution and longer test periods. Memory limits can restrict the number of assets and backtest years especially with **.t1** or **.t2** backtests in **TICKS** mode. For reducing the resolution of T1 data and thus the memory requirement and backtest time, increase [TickTime](ticktime.htm). Differences of T1 backtests and M1 backtests can also arise from the different composition of M1 ticks on the broker's price server, causing different trade entry and exit prices. For instance, a trade at 15:00:01 would be entered at the first price quote after 15:00:01 with T1 data, but at the close of the 15:00:00 candle with M1 data. If this difference is relevant for your strategy, test it with T1 data. The [TickFix](ticktime.htm) variable can shift historical ticks forward or backward in time and test their effect on the result.

### Backtest results vs live results

Zorro backtests are as close to real trading as possible, especially when the [TICKS](mode.htm) flag is set, which causes trade functions and stop, profit, or entry limits to be evaluated at every price tick in the historical data. Theoretically a backtest generates the same trades and thus returns the same profit or loss as live trading during the same time period. In practice there can and often will be deviations, some intentional and desired, some unwanted. Differences of backtest and live trading are caused by the following effects:

  * Data cleaning. In live trading, price quotes arrive in irregular intervals and with outliers. M1 oder D1 historical data is normally 'cleaned' by removing outliers and combining price quotes to candles. This can cause intrabar price differences, for instance with the [price()](price.htm) function that calculates the average of price ticks within a bar. Indicators derived from cleaned prices can trigger a trade signal in the backtest where it didn't occur in live trading, or vice versa. If a strategy is very senstive to tiny intrabar price differences, the backtest should be performed with raw, unfiltered T1 data.
  * Data resolution. The 'granularity' of M1 or D1 backtests can greatly affect the backtest result when it depends on small entry-exit distances, as with binary trading or scalping systems. Due to the priority of [Stop](stop.htm) over [TakeProfit](stop.htm), backtests will be too pessimistic when both exits are hit within the same historical candle. Pending trades that open and close inside the same candle can even produce totally meaningless test results. For determining the influence of granularity, run a test with and without [TICKS](mode.htm) and compare the results. If it's more than slightly different, use T1 data for backtesting (or better, rework your strategy for making it more robust). As a rule of thumb, systems that react sensitive on price differences less than a 5-minute candle should use T1 data - preferably from different sources - for a realistic backtest.
  * Data sources. Forex, CFD, and cryptocurrency price data is different with any broker and any exchange. If your strategy depends on small price differences, use always in backtests the data from the same source that you use for live trading. Historical data can also have different trading hours than live data. For instance, CFDs can get live price ticks around the clock, but some historical data sets only contain ticks during market hours. This produces a different number of bars in live and in historical data. In this case use the [TimeFrame](barperiod.htm) or [NOSHIFT](mode.htm) mechanisms for skipping bars outside market hours. 
  * Transaction costs. In live trading, account parameters such as [spread](spread.htm), [rollover](spread.htm), and [commission](spread.htm) can change all the time and affect trade profits and losses. In the backtest those parameters have normally fixed values that are set up by the script or the account model in the [asset list](account.htm). It is normally desired to run backtests with constant parameters, in order to not add artifacts to the result. Normally the current account parameter values should be used for more accurately predicting the strategy performance, even when they were very different in the historical period covered by the backtest.
  * Slippage. It is [simulated](spread.htm) in the backtest, but can nevertheless be very different in live trading dependent on market liquidity or external events. 
  * Rejected orders. In live trading, opening or closing of trades can be rejected by the broker for various reasons that don't happen in the backtest (see [Warning 075](errors.htm)), 
  * Account differences. If the backtest simulates an account model with a different leverage or lot size, trades can be skipped in the test where they are entered in live trading, or vice versa. Different margins and leverages can greatly affect the performance. See [asset list](account.htm) for details about the account models. 

### Backtest bias and reality checks

The likeliness that the strategy exploits real inefficiencies depends on in which way it was developed and optimized. There are many factors that can cause bias in the test result. Curve fitting bias**** affects all strategies that use the same price data set for test and training. It generates largely too optimistic results and the illusion that a strategy is profitable when it isn't. Peeking bias is caused by letting knowledge of the future affect the trade algorithm. An example is calculating trade volume with capital allocation factors ([OptimalF](optimalf.htm)) that are generated from the whole test data set (always test at first with fixed lot sizes!). 

Data mining bias (or selection bias) is caused not only by data mining, but already by the mere act of developing a strategy with historical price data, since you will selecting the most profitable algorithm or asset dependent on test results. Trend bias affects all 'asymmetric' strategies that use different algorithms, parameters, or capital allocation factors for long and short trades. For preventing this, [detrend](detrend.htm) the trade signals or the trade results. Granularity bias is a consequence of different price data resolution in test and in real trading. For reducing it, use the [TICKS](mode.htm) flag, especially when a [ trade management function](trade.htm) is used. Sample size bias is the effect of the test period length on the results. Values derived from maxima and minima - such as [drawdown](performance.htm) \- are usually proportional to the square root of the number of trades. This generates more pessimistic results on large test periods. 

Before going live, always verify backtest results with a Reality Check. It can detect many source of bias. For details please read [ Why 90% of Backtests Fail](https://financial-hacker.com/why-90-of-backtests-fail/). Zorro comes with scripts for two tests:

A WFO profile determines the dependence of the strategy on the WFO cycles number or length. It should be used for any walk-forward optimized trading system. For this, open the **WFOProfile.c** script, and enter the name of your strategy script in the marked line. In your script, uncomment the NumWFOCycles or WFOPeriod setting and - it it's a portfolio - disable all components except the component you want to test, and disable reinvesting. Then start **WFOProfile.c** in [Train] mode. If the resulting profile shows large random result differences as in the image below, something is likely wrong with that trading system. 

![](../images/WFOProfile_placebo.png)  
Suspicious WFO profile. Don't trade this live.

The Montecarlo Reality Check determines whether a positive backtest result was caused by a real edge of the trading system, or just by over-training or randomness. Open the **MRC.c** script and enter the name of your strategy script in the marked line. In your script - it it's a portfolio - disable all components except the component you want to test, and disable reinvesting. Then start **MRC.c** in [Train] mode for trained strategies, or in [Test] mode when the strategy has no trained models, rules, or parameters. At the end you'll get histogram and a p-value. It gives the probability that the backtest result is untrustworthy. The p-value should be well below 5%-10% for a valid backtest. 

![](../images/MRC_placebo.png)  
Lucky backtest result (black line). Don't trade this live, either.

### See also:

[training](training.htm), [trading](trading.htm), [mode](mode.htm), [performance](performance.htm), [debugging](chart.htm), [troubleshooting](trouble.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
