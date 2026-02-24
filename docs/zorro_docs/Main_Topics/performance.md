---
title: Performance Report
url: https://zorro-project.com/manual/en/performance.htm
category: Main Topics
subcategory: None
related_pages:
- [Chart Viewer / Debugger](chart.htm)
- [Log Messages](log.htm)
- [Included Scripts](scripts.htm)
- [Export](export.htm)
- [R Bridge](rbridge.htm)
- [report](report.htm)
- [Live Trading](trading.htm)
- [printf, print, msg](printf.htm)
- [bar](bar.htm)
- [Monte Carlo Analysis](montecarlo.htm)
- [Fill modes](fill.htm)
- [Spread, Commission, ...](spread.htm)
- [Strategy Statistics](statistics.htm)
- [Virtual Hedging](hedge.htm)
- [set, is](mode.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Oversampling](numsamplecycles.htm)
- [Money Management](optimalf.htm)
- [evaluate](evaluate.htm)
- [plot, plotBar, ...](plot.htm)
- [plotProfile, ...](profile.htm)
- [Asset and Account Lists](account.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [TrainMode](opt.htm)
- [Testing](testing.htm)
---

# Performance Report

# Performance Report 

Zorro can analyze the performance of its own strategy scripts, and also strategy results from other platforms that are imported from **.csv** trade lists. A click on [Result] after a test run produces a [chart](chart.htm), a [log](log.htm), a trade list in CSV format, and a performance report with portfolio analysis. Chart images and reports are stored in the **Log** folder and can be displayed with Zorro's image viewer or text editor. Exported charts are **.png** image files, generated reports are simple **.txt** files. This way they can be easily posted to websites, imported in documents, exported to spreadsheets, or further evaluated with the [Performance](scripts.htm) script. Additionally, datasets from charts can also be [exported](export.htm) and further evaluated with third party software, f.i. an [R data analysis](rbridge.htm) package. 

![](../images/chartviewer.png)

During a session, the performance report is available through the [report](report.htm) function. In [[Trade](trading.htm)] mode the chart and performance report is part of the **.htm** status page. It can be displayed in a web browser and is updated every minute. It also contains a list of all open and pending trades, as well as any specific information that is [printed](printf.htm) by the script. 

This is an example of a performance report: 

```c
Walk-Forward Test Z4 portfolio
 
Simulated account   AssetsFix.csv (NFA)
Bar Period          4 hours (avg 266 min)

Simulation period   06.06.2007-02.12.2017 (9483 bars)
Test period         10.06.2010-02.12.2017 (6259 bars)
WFO test cycles     11 x 569 bars (137 days)
WFO training cycles 12 x 3224 bars (111 weeks)
Lookback period     700 bars (169 days)
Monte Carlo cycles  200
Fill mode           Realistic (slippage 5.0 sec)
Avg bar             5.1 ticks  470.5 pips range
Spread              0.7 pips (roll -0.70/-0.31)
Lot size            1.0
Capital invested    5000$
 
Gross win/loss      90452$-73672$, +20252p, lr 16325$
Virtual win/loss    90024$-74112$
Average profit      5309$/year, 459$/month, 27$/day
Max drawdown        -2025$ 12% (MAE -2287$)
Total down time     78% (TAE 93%)
Max down time       119 days from May 2011
Max open margin     656$
Max open risk       1863$
Trade volume        $10164943 (2610626$/year)
Transaction costs   -637$ spr, -365$ slp, -906$ rol, -1102$ com
Capital required    $2681 (to cover drawdown)
 
Number of trades    1370 (351/year)
Percent winning     44%
Max win/loss        810$ / -407$
Avg trade profit    12$ 10.4p (+99.0p / -56.5p)
Avg trade slippage  -0.27$ 0.2p (+1.1p / -1.4p)
Avg trade bars      16 (+23 / -10)
Max trade bars      141 (34 days)
Time in market      355%
Max open trades     14
Max loss streak     17 (uncorrelated 13)
 
Annual return       161%
Profit factor       1.38 (PRR 1.28)
Reward/Risk ratio   10.5
Sharpe ratio        1.94 (Sortino 1.98)
Kelly criterion     0.63
Ulcer index         12%
Scholz tax          14026 EUR
Sample cycles ProF  1.39 1.40 1.31 1.33 1.37 1.38

WFO Cycles       Best    Worst     Avg   StdDev
Net Profit     14712$   11013$  12931$    2931$
Profit Factor    2.99     1.12    1.38    0.49
Num Trades        301       69     285
Win Rate          61%      34%     44%

Sample Cycles     Best    Worst    Avg   StdDev
Net Profit      69712$   61013$  63931$  12931$
Profit Factor    1.49     1.22    1.38    0.29
Num Trades       1301      969    1285
Win Rate          51%      39%     44%

Year Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec Total%
2010              -2  20 -16  20 -13  22  10  -8   8   +38
2011  -1  15  15  -9  26  -1  -1  13   6  28   1   1  +135
2012   2  -9  21   1   3  -2  -0   3   1   0   7  13   +51
2013   6  13  -7  -4   9   6  -8  -4   0   1  -2  -8    -3
2014   6   3   0  -0   4  -2   3  -6   8  12  -9   5   +23
2015   5  -7   8  13  -1   0   0   1   3  -1  -5 -12   +17
2016  19   1   6   9   6 -10  10   3   1  -1   6  -2   +30
2017  -1  -7   6  -5   0  -0  -1   2   1  -5  -1   7    -6
 
Confidence level     AR   DDMax  Capital
 10%                236%  1440$  1390$
 20%                227%  1550$  1470$
 30%                218%  1680$  1570$
 40%                209%  1830$  1680$
 50%                202%  1940$  1760$
 60%                193%  2140$  1900$
 70%                186%  2340$  2040$
 80%                174%  2730$  2320$
 90%                166%  3080$  2580$
 95%                146%  4010$  3580$
100%                104%  5640$  4710$
 
Portfolio analysis  Perf  ProF  Win/Loss  Wgt%  Cycles
 
AUD/USD avg         .014  1.36  281/475   11.4  XXXXXXXXXX/
EUR/USD avg         .006  1.41   91/115    6.2  XXXXXX\XXXX
GER30 avg           .030  1.33   34/45     2.5  X/X/\.\XXXX
SPX500 avg          .077  1.75   96/162   16.4  XXXXXXXXXXX
USD/JPY avg         .006  1.69  130/175    8.7  XXXXXXXXXX/
XAG/USD avg         .008  1.35  113/152    5.3  \\XXXXXXXX/

BB avg              .010  1.29   82/111    2.6  X/\XXX/\XXX
CT avg              .019  1.38  106/126    7.6  XXXX\XXXXXX
CY avg              .017  1.83   60/66     5.6  /XXX\/XXX/X
ES avg              .025  1.34  163/275    7.4  XXXXX/XXXXX
HP avg              .017  1.34  332/440   19.2  XXXXXXXXXXX
LP avg              .009  1.54  106/219   11.3  \XXXXXXXXXX
VO avg              .014  1.71  217/288   24.2  /XXXXXXXXXX

AUD/USD:ES          .036  1.14   45/87     0.1  \\/\\//\\//
AUD/USD:ES:L        .036  1.14   45/87     0.1  \\/\\//\\//
AUD/USD:ES:S        .000  ----    0/0      0.0  ...........
AUD/USD:HP          .024  1.18   76/102    2.2  X/\XX\/XX\/
AUD/USD:HP:L        .024  1.13   47/68     1.2  //./\\///\/
AUD/USD:HP:S        .043  1.35   29/34     1.0  \/\\/./\\\/
AUD/USD:LP          .029  1.66   75/149    7.5  \/X///X\/X/
AUD/USD:LP:L        .029  1.80   42/71     6.8  \//////\/\/
AUD/USD:LP:S        .058  1.23   33/78     0.6  \/\///\\///
EUR/USD:CT          .009  1.11   22/33     0.5  X/\X\X\X/XX
EUR/USD:CT:L        .027  1.22   10/19     0.7  \/\/\\\\.//
EUR/USD:CT:S        .000  0.91   12/14    -0.2  /.\\\/\//\\
EUR/USD:HP          .036  1.32   45/59     2.4  \///\/\\\/\
EUR/USD:HP:L        .036  1.32   45/59     2.4  \///\/\\\/\
EUR/USD:HP:S        .000  ----    0/0      0.0  ...........
EUR/USD:VO          .027  2.20   24/23     3.3  .X.//.\X/XX
EUR/USD:VO:L        .027  1.58   12/11     0.9  ././...//\\
EUR/USD:VO:S        .032  2.90   12/12     2.5  .\../.\\.//
GER30:BB            .038  1.03    2/4      0.0  /......\\/\
GER30:BB:L          .038  1.03    2/4      0.0  /......\\/\
GER30:BB:S          .000  ----    0/0      0.0  ...........
GER30:CT            .256  1.92    3/2      0.6  \/./...\/..
GER30:CT:L          .256  1.92    3/2      0.6  \/./...\/..
GER30:CT:S          .000  ----    0/0      0.0  ...........
GER30:ES            .267  1.82    2/3      0.2  ..\/\../..\
GER30:ES:L          .000  ----    0/0      0.0  ...........
GER30:ES:S          .267  1.82    2/3      0.2  ..\/\../..\
GER30:VO            .051  1.31   27/36     1.7  ////\.\\/\/
GER30:VO:L          .051  1.31   27/36     1.7  ////\.\\/\/
GER30:VO:S          .000  ----    0/0      0.0  ...........
SPX500:ES           .110  1.44   17/21     1.3  //\/\///\\\
SPX500:ES:L         .110  1.44   17/21     1.3  //\/\///\\\
SPX500:ES:S         .000  ----    0/0      0.0  ...........
SPX500:LP           .006  1.04   17/55     0.2  \\/\\\\\//\
SPX500:LP:L         .006  1.04   17/55     0.2  \\/\\\\\//\
SPX500:LP:S         .000  ----    0/0      0.0  ...........
USD/JPY:BB          .057  1.51   24/36     0.6  \/\\/\/.\./
USD/JPY:BB:L        .000  ----    0/0      0.0  ...........
USD/JPY:BB:S        .057  1.51   24/36     0.6  \/\\/\/.\./
USD/JPY:CT          .016  1.46   16/19     2.0  \./..\/.\//
USD/JPY:CT:L        .016  1.46   16/19     2.0  \./..\/.\//
USD/JPY:CT:S        .000  ----    0/0      0.0  ...........
USD/JPY:HP          .024  2.24   34/29     6.4  .XX/\/\/.\/
USD/JPY:HP:L        .024  1.22    9/13     0.3  .\//\/\..\/
USD/JPY:HP:S        .028  2.65   25/16     6.0  ./\/.../.\/
XAG/USD:CT          .038  1.87   13/13     0.6  ....\///\\/
XAG/USD:CT:L        .000  ----    0/0      0.0  ...........
XAG/USD:CT:S        .038  1.87   13/13     0.6  ....\///\\/
XAG/USD:HP          .018  1.43   44/46     1.1  \\/X/\//X//
XAG/USD:HP:L        .105  4.16    6/3      0.3  .\./....\/.
XAG/USD:HP:S        .014  1.33   38/43     0.8  \\/\/\/////
XAG/USD:VO          .011  1.39   40/62     3.0  .\/\//\\\//
XAG/USD:VO:L        .000  ----    0/0      0.0  ...........
XAG/USD:VO:S        .011  1.39   40/62     3.0  .\/\//\\\//
```

The following table shows the meaning of the values. Most of the calculated values are only valid for a profitable report (**Gross Win > Gross Loss**); they are meaningless when the profit is negative. Indicated values with a "**$** " suffix are in units of the account currency (not necessarily US-$), indicated values with a "**p** " or "**pips** " suffix are in pips.   
  
---  
**Bar period** | Bar period in seconds, minutes, or hours, and average bar duration in minutes. The average bar duration is the date difference of the last and first bar, divided by number of bars. It is used in subsequent figures for converting bar numbers to time periods - hours, days, or weeks - and might not reflect the real period. Variations in bar duration are caused by weekends, holidays, skipped bars, [special bars](bar.htm), or gaps in the historical data.  
**Simulation period** | Time of the WFO run (usually 4..5 years) and the number of bars, without the preceding lookback period.  
**Test period** | Time and bar number of the test; simulation period without training and lookback.   
**WFO test cycles** | Number and length of the WFO test cycles in bars, hours, trading days, or calendar weeks. This is also the recommended period for re-training a WFO system. The given time periods are based on the average bar duration displayed above, and a 5-day week.  
**WFO train cycles** | Number and length of the WFO training cycles.   
**Lookback period** | Amount of data to be collected before test or training begins.   
**Monte Carlo cycles /  
Confidence** | Number of [Monte Carlo](montecarlo.htm) simulation cycles, and selected confidence level (if any) for the following performance figures.   
**Fill mode** | Simulated fill mode ([Naive](fill.htm) or [Realistic](fill.htm)) and [Slippage](spread.htm).   
**Avg bar** | Average number of price ticks and average high-low difference per bar in the simulation; for single asset strategies.   
**Spread** | Spread in pips, and long/short rollover fee in account currency units; for single asset strategies.   
**Commission** | Roundturn commission in account currency units; for single asset strategies.   
**Lot size** | Lot size in contracts or underlying units; for single asset strategies.   
|   
**Gross win / loss** | Sums of all wins and all losses in account currency units, the overall volume-neutral result in pips, and the result by linear regression of the equity curve ([ReturnLR](statistics.htm)). A negative pip result and a positive gross win - or vice versa - is possible due to different pip costs and different trade volumes.   
**Virtual win / loss** | Sums of all wins and all losses of phantom trades in currency units, for [virtual hedging](hedge.htm). Normally worse than the gross win / loss due to higher transaction costs.   
**Average profit** | Annual, monthly, and daily profit (workdays only); simply the difference between start and end balance divided by the number of periods.   
**Max drawdown / MAE** | Maximum drawdown and MAE during the test period, in account currency units and in percent from the preceding balance peak. **Drawdown** is the difference between a balance peak and the lowest subsequent equity valley (balance = account value, equity = balance plus value of the all open trades). **MAE** (maximum adverse excursion) is the difference between an equity peak and the lowest subsequent equity valley. A drawdown percentage above 100% hints that the equity became negative at some point in the backtest. Drawdown is dangerous to your account, MAE only to your peace of mind (see also **Ulcer Index** below).   
**Total down time / TAE** | Percentage of time when the current equity is below a preceding balance peak. **TAE** (time in adverse excursion) is the time when trades are open and the current equity is below a preceding equity peak. Strategies often have up to 90% down time, but can be still profitable.   
**Max down time** | Longest drawdown duration, i.e. maximum time from a balance peak to the lowest subsequent equity valley. The [BALANCE](mode.htm) flag can be used for generating a balance chart and identifying the balance peaks in the profit curve.   
**Max open margin** | Maximum total margin allocated during the backtest period.   
**Max open risk** | Maximum loss when all open trades hit their initial [Stop](stop.htm) at the worst possible moment. Dependent on the stop distances and the likeliness of such an event, the **Max open risk** can far exceed the **Capital required**. This parameter has no meaning for [virtual hedging](hedge.htm) strategies or for strategies that use no stop loss for some trades.  
**Trade volume** | Total and annualized value of all assets bought and sold, in units of the account currency (see remarks).   
**Transaction costs** | Total costs of spread (**Spr**), slippage (**Slp**), swap/rollover/margin interest (**Rol**) and commission (**Com**) for all trades. [Slippage](spread.htm) and [rollover](spread.htm) can increase the profit in some cases; costs are then positive. In test mode the simulated costs are displayed, in trade mode the real costs when available through the broker API. Otherwise the cost are estimated and can be inaccurate. Slippage cost in trade mode is calculated from the difference of price at order time and fill price.  
**Capital required** | Required initial capital for trading the strategy; equivalent to the maximum margin on non-leveraged accounts, and the sum of maximum margin and normalized maximum drawdown or MAE on leveraged accounts. This amount would be required when the strategy is entered at the worst possible moment of the simulation, for instance directly at a balance peak preceding a drawdown. For strategies that reinvest profits ([Capital](lots.htm) variable), the displayed value is multiplied with the ratio of initial and final balance, and thus can be much smaller than the maximum drawdown or open margin.   
|   
**Number of trades** | Number of trades in the backtest period, per time period and per WFO cylce. Only real trades are counted, not phantom or pending trades. The number of trades used for training can be much higher when trade limits are not observed in the training process.  
**Percent winning** | Percentage of winning trades.   
**Max win / loss** | Maximum win and loss of all trades.   
**Avg trade profit** | Average return of a trade in account currency units and in volume-neutral pips; separately listed for winning (+) and losing (-) trades. A negative pip return and a positive currency return - or vice versa - is possible due to different pip costs and different trade volumes. Robust strategies should return a multiple of the spread. Avoid systems that generate many trades with very small average returns, or few trades with very large average returns.  
**Avg trade slippage** | Average slippage cost of a trade in account currency units and in pips; separately listed for positive (+) and negative (-) slippage. In test mode it's the simulated slippage, in trade mode the real slippage.   
**Avg trade bars** | Average number of bars of a trade; separately for winning (+) and losing (-) trades.  
**Max trade bars** | Maximum time a trade was open.   
**Time in market** | Total time of all trades compared to the backtest time. This can be more than 100% when several trades are open at the same time. The smaller the time in market, the less exposed is the capital to the market risk.   
**Max open trades** | Maximum number of simultaneously open trades.   
**Max loss streak /  
uncorrelated ** | Maximum number of consecutive losses during the test, and the theoretical number under the assumption of uncorrelated returns, i.e. equally distributed wins and losses. If the real number is noticeably higher, wins and losses tend to cluster with this strategy, and an [equity curve trading](tips.htm#equity) algorithm could improve the performance.   
|   
**Annual return (AR)** | Annualized profit divided by the required initial capital; main performance parameter for systems that don't reinvest profits. Equivalent to the Calmar Ratio. Depends on maximum drawdown and can thus be subject to random fluctuations (see remarks).   
**Return on margin  
(ROI)** | Annualized profit divided by maximum required margin. For accounts with no leverage, where the investment is mainly used for margin and not for buffering drawdowns.   
**Annual growth rate  
(CAGR) ** | Compound annual growth rate of the investment; the **n** th root of the total equity growth, where **n** is the number of years in the test period. Displayed for strategies that have the [Capital](lots.htm) variable set and are assumed to reinvest profits. The annual return on capital (**ROI**) is also displayed.  
**Profit factor / PRR** | Gross win divided by gross loss. The pessimistic return ratio (**PRR**) is the profit factor multiplied by **(1-1/sqrt(W))/(1+1/sqrt(L))** ; it gives a worse result when the number of trades is low.   
**Reward/Risk  
ratio ** | End profit divided by maximum drawdown.   
**Sharpe / Sortino  
ratio** | The Sharpe ratio is the annualized ratio of mean and standard deviation of the bar returns when the system is in the market; the Sortino ratio is the annualized ratio of mean and semi-deviation. Bar returns are bar profits divided by invested or required capital. Bars with no open trades do not contribute. The Sharpe ratio is a popular performance gauge (see remarks) and should be > 1 for good strategies.  
**Kelly criterion** | Ratio of mean and variance of the bar returns when the system is in the market; bars with no trades do not contribute. The kelly criterion is the optimal investment factor for a single-asset, single-algo, single-trade per bar strategy to maximize the profit.   
**R2 coefficient** | Coefficient of determination; the similarity of the equity curve with a straight line ending up at the same profit. The closer **R2** is to **1** , the steadier are the profits and the better they will be possibly reproduced in real trading (see remarks).   
**Ulcer index** | Average percentage of equity retracements from their preceding equity peaks ( <https://en.wikipedia.org/wiki/Ulcer_index>) The higher the ulcer index, the stronger your stomach should be for trading the strategy.   
**Scholz tax** | 26.375% of the gross win, minus EUR 10,000 for any trading year. Only displayed when tax is due and [ScholzBrake](lots.htm#scholz) is not set.  
**Sample cycles** | Separate profit factors of the [oversampling cycles](numsamplecycles.htm). Large profit differences between cycles are an indicator of an unstable strategy.   
|   
**WFO Cycles  
Sample Cycles** | Net profit, profit factor, number of trades, and win rate of the best, the worst, and the average WFO or sample cycle, and their standard deviations. Large performance differences between WFO cycles are an indicator of too-short cycles; large differences betwen sample cycles of an unstable strategy.   
|   
**Year** | Annual and monthly returns, either in percent of the **Capital required** for non-reinvesting strategies, or as percent change from the preceding month or year for reinvesting strategies with nonzero [Capital](lots.htm) variable.  
|   
**Monte Carlo  
analysis** |  Performance analysis (see [Monte Carlo Method](montecarlo.htm); non-reinvesting strategies only) by evaluating many possible equity curves with different distributions of trades and returns. A strong serial correlation of trade returns can cause Monte Carlo results higher than the result from the real equity curve.   
**Confidence level** | Confidence level of the following performance parameters. F.i. at 95% confidence level, 95% of all simulations generated the same or better results, and 5% generated worse results.   
**AR** |  Annual return at the given confidence level.   
**DDMax** | Maximum drawdown (not normalized) at the given confidence level.   
**Capital** | Capital requirement at the given confidence level.   
|   
**Portfolio  
analysis** | Performance analysis per asset, per algorithm, and per component. Only components with trades are listed. The figures are taken from the last [oversampling cycle](numsamplecycles.htm); when the [ALLCYCLES](mode.htm) flag is set, they are taken from all cycles.   
**Perf** |  OptimalF factors (see [Money Management](optimalf.htm), only when no [Capital](lots.htm) is set) or user defined Performance metric for any portfolio component. When **0** , the component was unprofitable in the test. Calculating OptimalF factors can take a long time when many trades were opened. When the [NOFACTORS](mode.htm) flag is set or [Capital](lots.htm) is nonzero with a reinvesting strategy, no factors are calculated.   
**ProF** | Profit factor (gross win divided by gross loss, including phantom trades). A '++++' in the column indicates that there were only winners, '----' indicates that there were no winners.   
**Win / Loss** | Number of winning and losing trades, including phantom trades.   
**Wgt%** | Weight of the component in percent; component profit divided by total profit. Indicates the contribution of the component to the whole strategy. The weight can be negative, f.i. with a losing component and a positive overall result.  
**Result** | Current profit or loss of the component in live trading.  
**Cycles** | Profit separated by WFO cycles. '/' is a winning cycle, '\' a losing cycle, 'X' is a cycle with both winning and losing components, and '.' is a cycle without trades.   
  
Additionally, the performance can be evaluated by user criteria or stored for further evaluation with the user-supplied [evaluate](evaluate.htm) function. 

### Remarks

  * When developing a strategy, don't look only into the performance report. Especially when results are unexpected or affected by parameters in strange ways, look into the log file and examine the single trades. This is often more informative than the performance summary.
  * To speed up the performance report generation especially on backtests with a large number of bars or trades, disable Montecarlo permutations (**MonteCarlo = 0;**) and OptimalF factor calculation (**set(NOFACTORS);**). 
  * Additional performance figures of any kind can be added to the report with a **[print(TO_REPORT, ...)](printf.htm)** command. All trades are stored and can be enumerated in a (**for(all_trades)**) loop in the [evaluate](evaluate.htm) function. The daily equity or balance curve is available in the [ResultsDaily](statistics.htm) array for further calculations. Graphical presentations of performance parameters can be produced with [plotBar](plot.htm) statements. Examples can be found in the [include\profile.c](profile.htm) file. If you need a certain performance parameter that can not be easily generated by script, please ask for it on the Zorro user forum.
  * Most performance parameters above are only valid when the result is positive. and  profits are not reinvested. If the **Capital** variable is set, Zorro assumes profit reinvesting. This affects the maximum drawdown and invalidates all drawdown-dependent performance parameters such as required capital, annual return, OptimalF factors, Monte Carlo analysis, and to some degree, Sharpe ratio. Therefore, have **Capital** at zero when testing the basic performance of your strategy; and enable reinvesting it only at the end for testing the money management. 
  * In backtest and training, performance is normally calculated under the assumption of constant spread, commission, slippage, rollover, and account currency exchange rate, which are all taken from the [asset list](account.htm). Theoretically the performance could be adapted to variations in those parameters as in the example under [PipCost](pip.htm). But this is only recommended in special cases, because the result differences are not relevant for the strategy performance, but would add artifacts to the test and training process. 
  * When the backtest runs over several [sample cycles](numsamplecycles.htm), the price curve in the chart only shows the trades of the last cycle, but the performance report and the equity curve are calculated from all cycles. If some cycles win and others lose, the summing up can generate results that appear inconsistent, for instance a small positive **Sharpe ratio** , but a small negative **Annual return**.
  * The performance report differentiates between Maximum Drawdown and Maximum Adverse Excursion (MAE). The drawdown is the important parameter; it reduces the free margin on your account and can cause a margin call. The MAE has no direct effect on your account. Drawdown and MAE can be substantially different, especially when open trades undergo high volatilty periods.   
Example: A single trade first goes up to $3000 profit, then falls down to -$1000 loss, then goes up again and is closed at $500 profit. This trade has a MAE of -$4000, but only a drawdown of -$1000. You need a capital of at least $1000 plus the position margin for avoiding a margin call. Drawdown is dangerous, MAE is not (except for your nerves). Your capital requirement normally depends on the maximum drawdown. But when you observe a high difference between MAE and maximum drawdown, try improving the trade exit algorithms.
  * **Maximum drawdown** and **MAE** depend on the test period; longer test periods cause higher maximum drawdown. On a break-even system the maximum drawdown is proportional to the square root of the test period*. For this reason, drawdown dependent performance figures such as **Annual return** and **Capital required** are calculated from a normalized drawdown, which adjusts the drawdown to 3 years. For this, the drawdown is multiplied with sqrt(3/test period in years)..This results in performance figures that are largely independent on the test period. 
  * **Maximum drawdown** and **MAE** are a result of the random win/loss pattern of trades. They can change greatly with minor changes to the strategy or test period; therefore all performance parameters calculated from the original equity curve are subject to randomness. For better accuracy, use [Monte Carlo Simulation](montecarlo.htm). 
  * The **Capital required** is a statistical parameter calculated from the simulation. It is unrelated to the **Risk** displayed by Zorro for single trades, which is the maximum loss when hitting the initial stop. The **Max open risk** parameter and even an individual trade **Risk** can be higher than the required capital. Do not take the **Capital required** parameter as a guaranteed loss limit! 
  * The **Sharpe ratio** takes risk into account. It is supposed to compare the strategy performance with a risk-free return as from a savings account. A Sharpe ratio of 1.5 would be equivalent to a savings account with 50% risk-free interest. Traditionally, 4% are subtracted from the Sharpe ratio for obtaining the difference to the usual 4% interest of risk-free investment returns. This would make no sense for comparing trade strategies, thus risk-free interest is not subtracted here.
  * The **Sortino ratio** differs from the Sharpe ratio in using only the standard deviation of below-mean returns (semi-deviation). It is often considered to measure risk better than the Sharpe ratio. 
  * The **R2** coefficient is a measure of the upwards tendency of the equity curve. For this the equity curve is compared with a straight line through its start and end points (not with its own regression line as in some applications, as this could feign a high linearity even with a falling equity curve). R2 is the squared correlation coefficient of real curve and straight slope. A equity curve with no significant upwards tendency produces a R2 coefficient of zero. 
  * Large portfolio strategies with thousands of assets and trades can take a long time for calculating **OptimalF** factors. If not needed, use **setf(**[TrainMode,SETFACTORS](opt.htm)) for disabling the **OptimalF** calculation..
  * The per-asset and per-algo figures in the portfolio analysis are simple non-weighted averages of the components. Thus, low-volume components contribute overproportionally and the weights won't add up to 100%. 

Keep in mind that that all those performance figures are derived from historical data (even when it's out-of-sample data). The future is unknown, so there is no guarantee to achieve the same performance in live trading. Many figures - f.i. Sharpe ratio, Monte Carlo analysis, drawdown extrapolation, R2 coefficient - are based on mathematical models that assume a Gaussian distribution of returns. However there is no guarantee that real returns always follow a Gaussian distribution. For those reasons, don't interpret too much into performance figures. Even a system with excellent theoretical performance can cause real loss of money. 

* See Malik Magdon-Ismail / Amir Atiya, "Maximum Drawdown", 2004.

### See also:

[Testing](testing.htm), [Chart](chart.htm), [evaluate](evaluate.htm), [report](report.htm), [Monte Carlo Simulation](montecarlo.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
