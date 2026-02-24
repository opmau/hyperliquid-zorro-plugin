---
title: bar
url: https://zorro-project.com/manual/en/bar.htm
category: Functions
subcategory: None
related_pages:
- [Bars and Candles](bars.htm)
- [Indicators](ta.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [price, ...](price.htm)
- [set, is](mode.htm)
- [Spread, Commission, ...](spread.htm)
- [Performance Report](performance.htm)
- [series](series.htm)
- [Included Scripts](scripts.htm)
- [Bar, NumBars, ...](numbars.htm)
- [tick, tock](tick.htm)
- [trade management](trade.htm)
- [Functions](funclist.htm)
---

# bar

# User-defined bars

Instead of fixed bar periods, bars can also cover fixed price ranges or can be defined by any other criteria. Some traders believe that those special bars help their trading, because they make the chart look more predictive. Zorro supports a user-defined **bar** function for special bars of any kind. Predefined functions for Range, Renko, Haiken Ashi, or Point-and-Figure bars are included. The **bar** function can be used for single-asset or multiple-asset strategies, or for strategies that use multiple bar types simultaneously. In the case of single-asset strategies the individual bars are also plotted in the chart.

![](../images/rangebars.png)  
Range bars (see code)

  

## bar(var *Open, var *High, var *Low, var *Close, _var *Price, DATE Start, DATE Time_): int 

User-supplied function for defining a [bar](bars.htm). Used to create special bar types or to replace the mean price by a different algorithm such as [VWAP](ta.htm). The **bar** function is automatically called whenever a new price quote arrives or a price tick is read from a historical file. It can modify the candle and determine if and when the bar ends.  

### Parameters:

**Open, High,  
Low, Close,  
Price ** | Arrays with two elements of the corresponding price; f.i. **Close[1]** is the close price of the previous bar, and **Close[0]** is the most recent price. The prices of the current bar can be modified by the **bar** function. **Price** is the mean price and can be omitted when not used in the function.   
---|---  
**Start** | Optional start time of the bar in the OLE DATE format. Can be omitted when not used in the function.   
**Time** | Optional time of the most recent tick or price quote. Can be omitted when not used in the function.  
  
### Returns:

**0** \- close the bar at the normal end of the [BarPeriod](barperiod.htm).**  
****1** \- close the bar now, and call the **bar** function again at the next price tick.   
**4** \- keep the bar open, and call the **bar** function again at the next price tick.   
**8** \- call the **bar** function only at the normal end of the [BarPeriod](barperiod.htm).. 

### Remarks:

  * The **bar** function in a single asset strategy is automatically called during the history building process and has access to the current and previous candle. Indicators, series, price, or trade functions cannot be called from inside a **bar** function. If needed for a very special bar, calculate the candle in the **run** function and pass it to the **bar** function through global variables.
  * If prices are not otherwise modified by the **bar** function, they are set up as for a normal candle, i.e. **Open[0]** is the first price of the bar, **High[0]** is the highest price so far, **Low[0]** the lowest price and **Price[0]** the current mean price of the bar. 
  * Modified prices also determine the corresponding price functions ([priceO](price.htm), [priceH](price.htm), [priceL](price.htm), [priceC](price.htm), [price](price.htm)) and all price-dependent indicators. This can cause **data-snooping** and therefore unrealistic backtests when the bar open price is affected by its close price or by the high or low. Backtest with the [TICKS](mode.htm) flag for making sure that the trades open and close at the recent tick instead of the back candle,thus eliminating snooping bias.
  * Set [Slippage = 0](spread.htm) for backtests with user defined bars. Slippage simulation works only with normal bars.
  * The price history should have sufficient resolution so that any special bar covers at least one price quote. Special bars cannot have higher resolution than the price history.
  * Most special bars, such as Haiken Ashi or Range/Renko bars, are **affected by any preceding bar** and thus depend on the time stamp or price at the very first bar. Their candles therefore depends on start date, lookback period, bar period, bar offset, time zone, or any other settings that affect the first bar.
  * [BarPeriod](barperiod.htm) has no effect on range dependent bars except for the above mentioned start time and the pre-allocation of bar memory. For allocating enough bars without wasting memory, set [BarPeriod](barperiod.htm) to the approximate average bar duration that is displayed in the [performance report](performance.htm). If you don't know it, set **BarPeriod = 1**. Bar zones and bar offsets should not be set when using special bar types.
  * In **multi-asset strategies** , or in strategies with multiple bar types, the bar defining function must be called in the script individually per asset or algo, and thus needs a different name than **bar** for not being called automatically. Since any asset has different bars, [series](series.htm) must be static and shifted by script. An example for a multiple asset strategy with Renko bars can be found in the **[SpecialBars](scripts.htm)** script.
  * **Renko bars** are described slightly differently on different websites. The examples below contain 3 known variants. 
  * Traditional price-distance bars, such as Renko, Range, or Point-and-Figure bars, are not really recommended for serious trading systems, since they ignore the time component and have normally a negative effect on the performance. This may be different for special bars sampled by other criteria such as trade volume or tick rate. Experiment with your own bar types!

### Examples (see also SpecialBars.c):

```c
var BarRange = 0.0030; // 0.3 cents bar range

// Range Bars, standard (high-low)
int bar(var* Open,var* High,var* Low,var* Close)
{
  if(Open[0] != Close[1]) {
    High[0] = max(Open[0],Close[1]);
    Low[0] = min(Open[0],Close[1]);
    Open[0] = Close[1];
  }
  if(High[0]-Low[0] >= BarRange)
    return 1; // bar ends
  return 4;   // bar continues
}

// Renko Bars, standard
int bar(var* Open,var* High,var* Low,var* Close)
{
  Open[0] = roundto(Close[1],BarRange);
  if(Close[0]-Open[0] >= BarRange) {
    Close[0] = Open[0]+BarRange;
    High[0] = Close[0];
    Low[0] = Open[0];
    return 1;
  }
  if(Open[0]-Close[0] >= BarRange) {
    Close[0] = Open[0]-BarRange;
    High[0] = Open[0];
    Low[0] = Close[0];
    return 1;
  }
  return 4;
}

// Renko Bars with wicks
int bar(var* Open,var* High,var* Low,var* Close)
{
  Open[0] = roundto(Close[1],BarRange);
  if(Close[0]-Open[0] >= BarRange) { 
    Close[0] = Open[0]+BarRange;
    return 1;
  }
  if(Close[0]-Open[0] <= -BarRange) { 
    Close[0] = Open[0]-BarRange;
    return 1;
  }
  return 4;
}

// Renko Bars, variant
int bar(var* Open, var* High, var* Low, var* Close)
{
  var OpenDiff = abs(Close[0]-Open[1]);
  var CloseDiff = abs(Close[0]-Close[1]);
  if(OpenDiff < CloseDiff) // we have a valley or peak
     Open[0] = Open[1];
  else  // we are moving with the trend
     Open[0] = roundto(Close[1],BarRange);
  if(Close[0]-Open[0] >= BarRange) {  // going up
    Close[0] = Open[0]+BarRange;
    High[0] = Close[0];
    Low[0] = Open[0];
    return 1;
  }
  if(Open[0]-Close[0] >= BarRange) { // going down
    Close[0] = Open[0]-BarRange;
    High[0] = Open[0];
    Low[0] = Close[0];
    return 1;
  }
  return 4;
}

// Mean Renko Bars
int bar(var* Open, var* High, var* Low, var* Close){  Open[0] = 0.5*(Close[1]+Open[1]);  if(Close[0] <= Open[0] - BarRange) {    Close[0] = Open[0] - BarRange;    return 1;  } else if(Close[0] >= Open[0] + BarRange) {    Close[0] = Open[0] + BarRange;    return 1;  }  return 4;}
// Haiken Ashi Bars
int bar(var* Open,var* High,var* Low,var* Close)
{
  Close[0] = (Open[0]+High[0]+Low[0]+Close[0])/4;
  Open[0] = (Open[1]+Close[1])/2;
  High[0] = max(High[0],max(Open[0],Close[0]));
  Low[0] = min(Low[0],min(Open[0],Close[0]));
  return 8;
}

// Point-and-Figure Bars
int bar(var* Open,var* High,var* Low,var* Close)
{
  static int direction = 0;
  if(direction == 1 && High[0]-Close[0] >= BarRange) {
    Open[0] = round(Low[0],BarRange);
    Close[0] = round(High[0],BarRange);
    Low[0] = Open[0];
    High[0] = Close[0];
    direction = 0;
    return 1;
  }
  if(direction == 0 && Close[0]-Low[0] >= BarRange) {
    Open[0] = round(High[0],BarRange);
    Close[0] = round(Low[0],BarRange);
    High[0] = Open[0];
    Low[0] = Close[0];
    direction = 1;
    return 1;
  }
  return 4;
}

// Point-and-Line Bars
int bar(var *Open,var *High,var *Low,var *Close)
{
  static int gDir = -1; // initially down
  static var gCF = C, gCR = C;

  var Percentage = 1;
  int gReverse = 3; // 3 boxes for reversal

// box size = percentage of price
  var Box = Close[0]*Percentage/100;
  var CF = ceil(Close[0]/Box)*Box;
  var CR = floor(Close[0]/Box)*Box;

  if(CF < gCF && gDir < 0) {
    gCR = CF-Box; gCF = CF; 
    return 1; // continue down
  } 
  if(gCF+Box*gReverse <= CR && gDir < 0) {
    gCR = CR; gCF = CR+Box; 
    gDir = 1; return 1; // go up
  }
  if(gCR < CR && gDir > 0) {
    gCR = CR; gCF = CR+Box; 
    return 1;
  } 
  if(gCR-Box*gReverse >= CF && gDir > 0) {
    gCF = CF; gCR = CF-Box; 
    gDir = -1; return 1;
  }
  return 4; // keep bar open, call again on next tick 
}
```

### See also:

[Bars and Candles](bars.htm), [Bar](numbars.htm), [BarPeriod](barperiod.htm), [tick](tick.htm), [TMF](trade.htm), [user supplied functions](funclist.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
