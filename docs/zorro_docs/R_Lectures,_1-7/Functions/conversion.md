---
title: Migrating Algorithmic Trading Code | Zorro Project
url: https://zorro-project.com/manual/en/conversion.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [R Bridge](rbridge.htm)
- [Python Bridge](python.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [BarMode](barmode.htm)
- [Fill modes](fill.htm)
- [TrainMode](opt.htm)
- [Testing](testing.htm)
- [Performance Report](performance.htm)
- [series](series.htm)
- [tick, tock](tick.htm)
- [run](run.htm)
- [Tips & Tricks](tips.htm)
- [DLLs and APIs](litec_api.htm)
- [Strategy Coding, 1-8](tutorial_var.htm)
- [Pointers](apointer.htm)
- [Structs](structs.htm)
- [Functions](function.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Conversion from C++, C#](litec_c.htm)
- [Visual Studio](dlls.htm)
---

# Migrating Algorithmic Trading Code | Zorro Project

#  Converting vom EasyLanguage, MQL4/5, C#, AFL, PineScript...

Professional trading firms use normally C or C++ for their trading systems, due to their backtest and execution speed. But before starting serious algotrading with C/C++ and [Zorro](https://zorro-project.com), you have maybe used a retail [ trading platform](https://zorro-project.com/algotradingtools.php). So you want to migrate your trading strategies, "Expert Advisors", algorithms, or indicators to C or C++. Although the Zorro C/C++platform can directly run [R](rbridge.htm) and [ Python](python.htm) functions or Windows DLLs, it won't understand scripts from other platforms without adaption. Even when the syntax is also C or C# based, their trading functions are normally too different for an automated conversion. It's still a programmer's task. You can either hire our [algo conversion service](https://zorro-project.com/development.php) or do it yourself. Below you'll find some useful hints and examples.

Generally, Zorro can replicate any indicator and any strategy or trading algorithm from any platform. The conversion will normally result in shorter, much faster, better readable, and more robust code. Here are some examples of [algorithmic trading scripts in C/C++](https://zorro-project.com/code.php). Many more examples for code conversion from Amibroker, TradeStation, MetaStock, or other platforms can be found in Petra Volkova's articles on [Financial Hacker](https://financial-hacker.com/). 

### Comparing backtests and charts between different platforms

Even perfectly converted [algo trading strategies](https://zorro-project.com/algotrading.php) can produce very different backtests and charts on different platforms. Especially MT4/MT5 backtests are often not reproducible with other platforms. When platform A produces a positve and platform B a negative backtest result of the same strategy - which one should you trust? Here's what to check for determining the reason of backtest and chart differences:

  * **Compare the price data.** Print out several candles with the same time stamps and check. Have they the same OHLC prices? Are they from ask, bid, or trade prices? Is the timestamp from the open, the center, or the close of the bar? UTC or a local time zone? Unadjusted or split and dividend adjusted? When using tick data, does it contain the same quotes? Forex, CFD, and cryptocurrency prices from different sources are often very different on the minute or tick level. Even data from the same source can look very different dependent on price type (ask/bid/last) and time zone. Some platforms, such as Tradingview, do not synchronize bars to full hours or days, but to the start time of the script. It is therefore perfectly normal that even charts of the same date, asset, and price source, but from two different platforms, contain different candles.
  * **Compare the indicators.** Are they based on the same algorithm, and return the same value? Some platforms, such as MT4 or TradeStation, use different algorithms for some indicators and thus get different results. For instance, the average true range is normally calculated with an EMA, bur some platforms use an SMA, so you need to replace ATR with ATRS for reprodcing the results. Even with an identical algorithm, cumulative indicators such as EMA or MACD can return different results especially at the begin of the simulation when the platform has a different [unstable period](lookback.htm) or does not use lookback or unstable periods at all.
  * Check the **script structure** and **bar creation**. Does it run on any tick or on any bar? Are the bars aligned on the same time? Does it trade with the same [market hours and weekend](barmode.htm) settings?
  * **Compare the account simulation**. Are lot size, leverage, margin, rollover fees, and commission considered, and are they identical? Does the platform simulate realistic order filling, or a 'naive' [fill mode](fill.htm) that enters and exits trades at the current price or at the pre-set stop or entry levels? Some platforms, even high priced options tools, do not consider ask-bid spreads, transaction costs, or slippage in their results. 
  * **Compare[optimization](opt.htm) and [backtest](testing.htm) methods**. Is the test in-sample, out-of-sample, or walk-forward? Are parameters optimized with genetic, brute force, or platform specific algorithms? In-sample backtests, as with MT4, produce normally meaningless results.
  * **Compare the**[performance metrics](performance.htm). Win rate and profit factor are normally unsuspicious, but other figures such as drawdown, annual return, Sharpe ratio, or volatility are often calculated with very different methods. Do not compare apples with oranges.   

### TradeStation™, MultiCharts™, TradeSignal™

**TradeStation** was the first platform that supported automated trading. Its **EasyLanguage** ™, also used by **MultiCharts** and in a variant named 'Equilla' by **TradeSignal** , has a similar design philosophy as Zorro's lite-C. Although the EasyLanguage syntax is a mix of C and Pascal, conversion to C is relatively straightforward. EasyLanguage variables are equivalent to C [data series](series.htm). The first element of a series has no index, thus **MyData** in EasyLanguage is the same as **MyData[0]** in C. Keywords are case insensitive. Local variables are preserved between function calls, do declare them with as **static** for replicating that. Array indices start with **0** as in C, but in many EasyLanguage code examples you find them starting with **1**. So the arrays are probably allocated with 1 extra element. Trigonometric functions (**Sine** , **Cosine** etc) expect angles in degrees (**0..360**), while in C and in most other languages angles are in radians (**..2*PI**). **Log** is the logarithm base **_e_** as in C.

At the time of this writing, EasyLanguage did not support functions like other languages, but requires separate scripts for them. The execution of an EasyLanguage script is similar to a lite-C script, with a lookback period determined by the **MaxBarsBack** variable. Aside from the missing functions, EasyLanguage strategies and lite-C strategies look very similar.

```c
{ Easylanguage version }
{ Choppy Market Index Function by George Pruitt }Inputs: periodLength(Numeric);
Vars: num(0),denom(1);
if(periodLength <> 0) thenbegin  denom = Highest(High,periodLength) – Lowest(Low,periodLength);  num = Close[periodLength-1] – Close;  ChoppyMarketIndex = 0.0;  if(denom <> 0) then 
    ChoppyMarketIndex = AbsValue(num/demon)*100;end;
```

```c
// Zorro version (lite-C)
// Choppy Market Index Function by George Pruittvar ChoppyMarketIndex(int periodLength) 
{
  if(!periodLength) return 0;  var Denom = HH(periodLength,0) – LL(periodLength,0);  var Num = priceC(periodLength-1) – priceC(0);  return abs(Num/fix0(Denom))*100;
}
```

```c
{ Easylanguage version }
{ enter a trade when the RSI12 crosses over 75 or under 25 }
Inputs:  
    Price(Close),LengthLE(20),LengthSE(20),
    OverSold(25),OverBought(75),StoplossPips(100),ProfitTargetPips(100);
 
variables:  
    var0(0),var1(0);
 
{ get the RSI series }
var0 = RSI( Price, LengthLE );
var1 = RSI( Price, LengthSE );
 
{ if rsi crosses over buy level, exit short and enter long }
condition1 = Currentbar > 1 and var0 crosses over OverBought ;
if condition1 then                                                                    
    Buy( "RsiLE" ) next bar at market;
 
{ if rsi crosses below sell level, exit long and enter short }
condition2 = Currentbar > 1 and var1 crosses under OverSold ;
if condition2 then                                                                    
    Sell Short( "RsiSE" ) next bar at market;
 
{ set up stop / profit levels }
SetStoploss(StoplossPips);
SetProfitTarget(ProfitTargetPips);
```

```c
// Zorro version (lite-C)
// enter a trade when the RSI12 crosses over 75 or under 25
function run()
{
  int LengthLE = 20,LengthSE = 20;
  var OverSold = 25, OverBought = 75, 
    StoplossPips = 100, ProfitTargetPips = 100;

// get the RSI series
  vars Var0s = series(RSI(seriesC(),LengthLE));
  vars Var1s = series(RSI(seriesC(),LengthSE));
 
// set stop / profit levels
  Stop = StoplossPips*PIP;
  TakeProfit = ProfitTargetPips*PIP;
 
// if rsi crosses over buy level, exit short and enter long
  if(crossOver(Var0s,OverBought))
    enterLong();
// if rsi crosses below sell level, exit long and enter short
  if(crossUnder(Var1s,OverSold))
    enterShort();
}
```

```c
// calculate frequently used EasyLanguage parameters in lite-C
int MarketPosition = sign(NumOpenLong-NumOpenShort);
var PriceScale = 100; // always 100 for stocks and futures
var MinMove = PIP * PriceScale; // Stock: 0.01 * 100, Emini: 0.25 * 100
var PointValue = LotAmount / PriceScale; // Stock: 0.01, Emini: 0.50
var BigPointValue = LotAmount;
var EntryPrice = 0;
for(current_trades) if(TradeIsOpen) EntryPrice = TradeFill;
```

  

### "Meta"-Trader 4 and 5 

MT4™ and MT5™ are popular platform for retail traders and provided by many brokers. The **MQL4** / **MQL5** script language of their "Expert Advisors" (EAs) is based on C or C++, which would theoretically allow easy conversion to Zorro. Unfortunately, MQL4 has some issues that make "Expert Advisors" more complex and difficult to convert than scripts of other platforms. The successor MQL5 has even more issues, and consequently has never managed to replace the older MQL4.  
MQL4 and MQL5 trades require a lot of managing by script. Series are not natively supported, but must be emulated with loops and functions. Some indicators - for instance, ATR - produce different results in MQL4 than in other platforms because they are not based on the standard algorithms, but on special MQL4 variants. Candles are based on local server time, and account parameters are not normalized, so any EA must be individually adapted to the country, broker, and account. To complicate matters further, MQL4 and MQL5 do not use common trade units such as lots and pips, but calculates with "standard lots" and "points" that need to be multiplied with account-dependent conversion factors. Most code in an EA is therefore not used for the trade logic, but for patching all those issues. This results in the long and complex 'spaghetti code' that is typical for MT4 and MT4 EAs.  
For conversion, first remove the MQL4/MQL5 specific code that is not needed in lite-C, such as trade management loops, broker dependent pip and trade size calculations, and array loops that emulate series. Then the rest can be converted by replacing the MQL4 indicators and trade commands by their lite-C equivalents. Replace **OnTick** either with [tick](tick.htm) or [run](run.htm), dependent on whether the code is bar or tick based. Bar indexes are normally shifted by 1, since they refer to the new (incomplete) bar, not to the last (complete) bar. Different indicator algorithms have to be converted or replaced. 

```c
// MQL4 version of the RSI strategy (see above)
// enter a trade when the RSI12 crosses over 75 or under 25
int start()
{
// get the previous and current RSI values
   double current_rsi = iRSI(Symbol(), Period(), 12, 
     PRICE_CLOSE, 1); // mind the '1' - candle '0' is incomplete!!
   double previous_rsi = iRSI(Symbol(), Period(), 12, PRICE_CLOSE, 2);
 
// set up stop / profit levels
   double stop = 200*Point;
   double takeprofit = 200*Point;

// correction for prices with 3, 5, or 6 digits
   int digits = MarketInfo(Symbol(), MODE_DIGITS);   if (digits == 5 || digits == 3) {     stop *= 10;
     takeprofit *= 10;
   } else
   if (digits == 6) {     stop *= 100;
     takeprofit *= 100;
   }

// find the number of trades 
   int num_long_trades = 0;
   int num_short_trades = 0;
   int magic_number = 12345;
// exit all trades in opposite direction
   for(int i = 0; i < OrdersTotal(); i++)
   {
// use OrderSelect to get the info for each trade
     if(!OrderSelect(i, SELECT_BY_POS, MODE_TRADES)) 
       continue;
// Trades not belonging to our EA are also found, so it's necessary to
// compare the EA magic_number with the order's magic number
     if(magic_number != OrderMagicNumber()) 
       continue;

     if(OrderType() == OP_BUY) {
// if rsi crosses below sell level, exit long trades
       if((current_rsi < 25.0) && (previous_rsi >= 25.0))
         OrderClose(OrderTicket(),OrderLots(),Bid,3,Green);
       else
// otherwise count the trades
         num_long_trades++;
     }
 
     if(OrderType() == OP_SELL) {
// if rsi crosses over buy level, exit short trades 
       if((current_rsi > 75.0) && (previous_rsi <= 75.0))
         OrderClose(OrderTicket(),OrderLots(), Ask,3,Green);
       else
// otherwise count the trades
         num_short_trades++;
     }
   }
 
// if rsi crosses over buy level, enter long 
   if((current_rsi > 75.0) && (previous_rsi <= 75.0) 
     && (num_long_trades == 0)) {
       OrderSend(Symbol(),OP_BUY,1.0,Ask,3,Ask-stop,Bid+takeprofit,"",magic_number,0,Green);
   }
// if rsi crosses below sell level, enter short
   if((current_rsi < 25.0) && (previous_rsi >= 25.0) 
     && (num_short_trades == 0)) {
       OrderSend(Symbol(),OP_SELL,1.0,Bid,3,Bid+stop,Ask-takeprofit,"", magic_number,0,Green);
   }
 
   return(0); 
}
```

Under [Tips & Tricks](tips.htm) you can find an example how to replicate MQ4-style indicator parameters with Zorro. The above mentioned issues also apply to **MQL5** , the "Meta"-Trader 5 script language. It has some similarities to MQL4, but offers C++ language elements and requires even more complex code for opening and managing trades.  
  

### NinjaTrader™

**NinjaScript** ™ is based on C# and thus similar in syntax to Zorro's lite-C. NinjaScript also supports data series in the same way as lite-C, and its basic function list is very similar; this makes script migration rather easy. One major difference is that all NinjaTrader indicator functions return data series, while Zorro indicators return single values. Use the [series](series.htm) function (f.i. **series(indicator(..))**) for making Zorro indicators also return series. 

```c
// NinjaTrader version
// Trade when a fast SMA crosses over a slow SMAprotected override void Initialize(){
// Run OnBarUpdate on the close of each bar  CalculateOnBarClose = true;

// Set stop loss and profit target at $5 and $10
  SetStopLoss(CalculationMode.Ticks,5);
  SetProfitTarget(CalculationMode.Ticks,10);
}

protected override void OnBarUpdate(){
// don't trade during the LookBack period
   if(CurrentBar < 20)      return;

   double Fast = 10;
   double Slow = 20;

// Exit short and go long if 10 SMA crosses over 20 SMA    if(CrossAbove(SMA(Close,Fast),SMA(Close,Slow),1)) {
     ExitShort();     EnterLong();
   }  
// Exit long and go short if 10 SMA crosses under 20 SMA    else if(CrossBelow(SMA(Close,Fast),SMA(Close,Slow),1)) {
     ExitLong();     EnterShort();
   }  
}
```

```c
// Zorro version
// Trade when a fast SMA crosses over a slow SMAvoid run(){
// Set stop loss and profit target at $5 and $10
  Stop = 5;
  TakeProfit = 10;

  vars SMAFast = series(SMA(seriesC(),10)); 
  vars SMASlow = series(SMA(seriesC(),20)); 

// Exit short and go long if 10 SMA crosses over 20 SMA    if(crossOver(SMAFast,SMASlow))     enterLong();
// Exit long and go short if 10 SMA crosses under 20 SMA    else if(crossUnder(SMAFast,SMASlow))
     enterShort();
}
```

  

### Amibroker™

Amibroker's **AFL™** language is a C dialect, with similar syntax as lite-C. But the script structure is different. Amibroker uses "**Buy** " and "**Sell** " variables for entering trades, instead of a function call. And Amibroker calls functions for setting system parameters, instead of using variables. So it's just the opposite of what you would expect. This unusual concept has its reason in a sort of vectorized approach to a trading system, where the code mainly initializes parameters and signal conditions. Variables need not be declared, and they are usually series, as in EasyLanguage. Functions are often very similar to lite-C - for instance, **Plot()** or **Optimize()**. Others can be easily converted, f.i. Amibroker's "**Explore** " feature is equivalent to Zorro's **print(TO_CSV)**. 

```c
// Amibroker version of the SMA crossing system (see above)
// Trade when a fast SMA crosses over a slow SMA
// Set stop loss and profit target at $5 and $10
   ApplyStop(stopTypeLoss,stopModePoint,5,0,True,0,0,-1);
   ApplyStop(stopTypeProfit,stopModePoint,10,0,True,0,0,-1);

   SMAFast = MA(C,10);
   SMASlow = MA(C,20);

// Buy if 10 SMA crosses over 20 SMA    Buy = Cross(SMAFast,SMASlow);
// Sell if 10 SMA crosses under 20 SMA       Sell = Cross(SMASlow,SMAFast);
```

  

### TradingView™

**TradingView** is a charting tool with a proprietary language named **PineScript™** for defining indicators. Variables are declared by assigning a value to them, and language blocks are defined by indentation, as in Python. Functions are defined with '**= >**'. Data series are in reverse order compared to other platforms: the **[0]** element is the oldest. Aside from that, conversion to C is normally pretty straightforward. Example for a Simple Moving Average: 

```c
// PineScript version
// SMA definition
study("My sma")
my_sma(Prices, Length) =>
  Sum = Prices
  for i = 1 to Length-1
    Sum := Sum + Prices[i]
  Sum / Length
```

```c
// Zorro version (lite-C)
// Choppy Market Index Function by George Pruittvar ChoppyMarketIndex(int periodLength) 
{
  if(!periodLength) return 0;  var Denom = HH(periodLength,0) – LL(periodLength,0);  var Num = priceC(periodLength-1) – priceC(0);  return abs(Num/fix0(Denom))*100;
}
```0

### Neuroshell Trader™

**Neuroshell Trader** is a platform specialized in employing a linear neural network for automated trading. Neuroshell indicators are functions added through DLLs. They take an input array, an output array, the array size, and additional parameters. Many other trade platforms use similar DLL based indicators. Such indicators are often based on normal C, thus conversion to Zorro is very easy - especially when you don't have to convert it at all and can [call the DLL function](litec_api.htm) directly.   
When using an indicator made for a different platform, the array order convention must be take care of. Neuroshell stores time series in ascending order (contrary to most other platforms that store them in reverse order) and passes the end of the array, not its begin, to the indicator function. Neuroshell indicators normally return an output series instead of a single value. Below both methods of indicator conversion are shown.

```c
// Zorro version (lite-C)
// Choppy Market Index Function by George Pruittvar ChoppyMarketIndex(int periodLength) 
{
  if(!periodLength) return 0;  var Denom = HH(periodLength,0) – LL(periodLength,0);  var Num = priceC(periodLength-1) – priceC(0);  return abs(Num/fix0(Denom))*100;
}
```1

```c
// Zorro version (lite-C)
// Choppy Market Index Function by George Pruittvar ChoppyMarketIndex(int periodLength) 
{
  if(!periodLength) return 0;  var Denom = HH(periodLength,0) – LL(periodLength,0);  var Num = priceC(periodLength-1) – priceC(0);  return abs(Num/fix0(Denom))*100;
}
```2

```c
// Zorro version (lite-C)
// Choppy Market Index Function by George Pruittvar ChoppyMarketIndex(int periodLength) 
{
  if(!periodLength) return 0;  var Denom = HH(periodLength,0) – LL(periodLength,0);  var Num = priceC(periodLength-1) – priceC(0);  return abs(Num/fix0(Denom))*100;
}
```3

  

### MatLab™

MatLab is a commercial computing environment and interactive programming language by **MathWorks, Inc**. It has some similarity to [R](rbridge.htm), but is not specialized on data analysis and machine learning. It allows symbolic and numerical computing in all fields of mathematics. With interpreted code and accordingly slow execution, it is not suited for backtesting trading strategies (unless they are converted to a vectorized structure, which is however an awkward process). Converting MatLab code to C is also a lot of work due to the very different language syntax. Fortunately there is a better solution: MatLab has an integrated compiler that compiles a MatLab trading algorithm to a C/C++ DLL. The DLL function can then be directly [called from a lite-C script](litec_api.htm).  
  

### See also:

[Introduction to lite-C](tutorial_var.htm), [Pointers](apointer.htm), [Structs](structs.htm), [Functions](function.htm), [DLLs](litec_api.htm), [MT4 bridge](mt4plugin.htm), [R bridge](rbridge.htm), [Python bridge](python.htm), [C++ to lite-C](litec_c.htm), [lite-C to C++](dlls.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
