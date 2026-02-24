---
title: Strategy statistics
url: https://zorro-project.com/manual/en/statistics.htm
category: Functions
subcategory: None
related_pages:
- [objective, parameters](objective.htm)
- [evaluate](evaluate.htm)
- [Trade Statistics](winloss.htm)
- [Performance Report](performance.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [CBI](ddscale.htm)
- [Bar, NumBars, ...](numbars.htm)
- [set, is](mode.htm)
- [TrainMode](opt.htm)
- [for(open_trades), ...](fortrades.htm)
- [trade management](trade.htm)
- [loadStatus, saveStatus](loadstatus.htm)
---

# Strategy statistics

# Strategy performance statistics 

The following system variables can be evaluated in the script or the [objective](objective.htm) or [evaluate](evaluate.htm) function. Some become available only at the end of a simulation or a live trading session, so they are only available to [objective](objective.htm) or [evaluate](evaluate.htm). Other variabes are updated at any bar during the test or live session, and thus can be evaluated in the script and affect the algorithm. There is also a set of [trade statistics](winloss.htm) that can be evaluated anytime. The statistics variables can be used for calculating individual performance metrics, and printing them to the [performance report](performance.htm).   

## DrawDownMax

Maximum drawdown, the largest difference between a balance peak and the lowest subsequent equity valley during the simulation period, in account currency units. Updated at any bar.

## DrawDownPercent

Maximum drawdown as above, in percent of the preceding balance peak. Updated at any bar. 

## MAEDepth

Maximum adverse excursion, the largest difference between an equity peak and the lowest subsequent equity valley during the simulation period, in account currency units. Updated at any bar. 

## AEDepth

Current adverse excursion, the difference between the last equity peak and the current equity value, in account currency units. Updated at any bar. 

## MarginMax

Maximum margin sum of open trades during the simulation period, in account currency units. Updated at any trade. 

## RiskMax

Maximum possible loss sum of all open trades during the simulation period, in account currency units. The possible loss is calculated by the stop loss distance of a trade, or estimated at 10% of the price if no stop loss was used. Updated at any trade. 

## SpreadCost

Total loss by bid-ask spreads during the simulation period, in account currency units. Updated at any trade. 

## SlippageCost

Total win or loss by simulated slippage during the simulation period, in account currency units. Updated at any trade. 

## RollCost

Total win or loss by rollover and margin interest during the simulation period, in account currency units. Updated at any trade. 

## CommissionCost

Total loss by commissions during the simulation period, in account currency units. Updated at any trade. 

## InterestCost

Total loss by interest charged by the broker. Updated any day. 

## ReturnMean

Mean of all bar returns on investment. The bar return on investment is the equity difference to the previous bar, divided by [Capital](lots.htm). If **Capital** is not set, the sum of normalized maximum drawdown and maximum margin is used. Only for bars with open positions, and only available at the end. 

## ReturnStdDev

Standard deviation of all bar returns on investment. Can be used together with **ReturnMean** for calculating performance metrics, such as Sharpe Ratio (see examples). Only available at the end. 

## ReturnUlcer

Ulcer Index in percent; worst drawdown after an equity peak. Only available at the end. 

## ReturnR2

R2 coefficient; the similarity of the equity curve with a straight line ending up at the same profit.Only available at the end. 

## ReReturnLR

Theoretical net profit by linear regression of the equity curve. Only available at the end.  

## ReturnCBI

The last [CBI](ddscale.htm) value based on previously stored backtest data.Only available at the end.  

## DrawDownBars

Total number of bars spent in drawdown, updated at any bar. For the percentage spent in drawdown, divide by the duration of the test, **([Bar-StartBar](numbars.htm))**. 

## DrawDownBarsMax

Maximum length of a drawdown in bars. Only available at the end. 

## LossStreakMax

Maximum number of losses in a row. Only available at the end. 

## NumOpenMax

Maximum number of simultaneously open trades. Updated at any bar. 

## InMarketBars

Total number of bars with open trades. Updated at any bar. 

## InMarketSum

Sum of the durations of all trades, in bars. Updated at any trade. Can be bigger than the duration of the test when several positions are simultaneously open. 

## NumMinutes

Total duration of the backtest period; can be used for normalizing metrics to 3 years or for calculating profit per period. The backtest years are **NumMinutes/(365.25*24*60)**. . For annualizing a result, divide it by this formula. 

### Type:

**int** for numbers that count something, otherwise**var**. Read/only.   

## ResultsAtBar

Array in chronological order containing the sums of wins and losses of all open and closed trades at every bar, in account currency units. **(var)ResultsAtBar[Bar]** is the end result of the simulation. 

### Type:

**float*** pointer, or **0** when no results were calculated. 

## ProfileAtBar

Array in chronological order containing the blue equity, balance, or pip return profile from the chart, at every bar. 

## DrawDownAtBar

Array in chronological order containing the red underwater profile from the chart, at every bar. 

### Type:

**var*** pointer, or **0** when no profile was calculated. 

## ResultsDaily

Array in chronological order containing the balance or equity (dependent on the [BALANCE](mode.htm) flag) at the end of every day, in account currency units. **ResultsDaily[Day]** is the end result. This array is at the end of the simulation automatically saved to a **.dbl** file for the [CBI](ddscale.htm) calculation, and to a **.csv** file for further evaluation. 

### Type:

**var*** pointer, or **0** when no daily results were calculated. 

### Remarks:

  * In [Train] mode the parameters are based on trades with 1 lot and include phantom trades, unless set up otherwise with [ TrainMode](opt.htm).
  * More statistics parameters can be retrieved from the [win/loss statistics](winloss.htm) at the end of the simulation.
  * Trade dependent metrics can be calculated by enumerating all trades of the simulation with a [for(all_trades)](fortrades.htm) loop, and summing up the desired [trade parameters](trade.htm). Equity curve dependent metrics can be calculated from the **ResultsAtBar** or **ResultsDaily** arrays. 
  * All statistics variables are normally reset when the strategy is restarted or a new session is started. If the [SAV_STATS](loadstatus.htm) flag is set, statistics is continued from the previous session.

### Example:

```c
// calculate some performance metrics
function evaluate()
{ 
  if(NumWinTotal == 0) return; // nothing worth to calculate
  var Years = NumMinutes/(365.25*24*60);
  int NumTrades = NumWinTotal+NumLossTotal;
  var NetProfit = WinTotal-LossTotal-InterestCost;
  var Sharpe = ReturnMean/ReturnStdDev*sqrt(InMarketBars/Years);
  var Calmar = NetProfit/fix0(DrawDownMax)/Years;
  var Expectancy = (WinTotal-LossTotal)/NumTrades;
  var GAGR = 100.*(pow((NetProfit+MarginMax)/MarginMax,1./Years)-1.);
  ...
}
```

### See also:

[Trade statistics](winloss.htm), [Cold Blood Index](ddscale.htm), [for(trades)](fortrades.htm), [NumBars](numbars.htm), [performance report](performance.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
