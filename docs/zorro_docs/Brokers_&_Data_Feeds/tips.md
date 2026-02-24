---
title: Tips & Tricks
url: https://zorro-project.com/manual/en/tips.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Trading Strategies](strategy.htm)
- [Indicators](ta.htm)
---

# Tips & Tricks

# Tips & Tricks

In the following you'll find short code snippets for common tasks. 

# I. Trade Management 

### Change stops and profit targets of all open long trades with the current algo and asset 

```c
exitLong(0,NewStop);
exitLong(0,-NewTakeProfit);
```

### If a trade is opened, close all other pending trades

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```

### Lock 80% profit of all winning trades

```c
for(open_trades)
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80; // 80% profit (minus trade costs)
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
```

### Iceberg trade: split a large trade into 10 small trades, one every 30 seconds

```c
for(OrderDelay = 0; OrderDelay < 10*30; OrderDelay += 30)
  enterLong(Lots/10);
```

### Calculate the value of all open trades with the current asset

```c
var valOpen(){
  var val = 0;  for(open_trades)        if(strstr(Asset,PrevAsset) && TradeIsOpen)      val += TradeProfit;  return val;}
```

### Monitoring and modifying a certain trade

```c
...
TRADE* MyTrade = enterlong();
...ThisTrade = MyTrade; // connect trade variables to MyTrade
var MyResult = TradeProfit; // evaluate trade variables
...
exitTrade(MyTrade,0,TradeLots/2); // exit half the trade
```

### Basket trading (creating an artificial asset from a 'basket' of real assets) 

```c
// generate a snythetic asset "USD" combined from the USD value of EUR, GBP, and AUD
var priceUSD()
{
  var p = 0;
  asset("GBP/USD"); p += price();
  asset("AUD/USD"); p += price();
  asset("EUR/USD"); p += price();
  return p;
}

// basket trade function with stop limit
int tradeUSD(var StopUSD)
{
  if((TradeIsLong && priceUSD() <= StopUSD) 
    or (TradeIsShort && priceUSD() >= StopUSD)) 
      return 1;   // exit the trade
  else return 0;  // continue the trade
}

// open a trade with the synthetic asset and a stop loss  
void enterLongUSD(var StopDistUSD)
{
  var StopUSD = priceUSD()-StopDistUSD;
  asset("GBP/USD"); enterLong(tradeUSD,StopUSD);
  asset("AUD/USD"); enterLong(tradeUSD,StopUSD);
  asset("EUR/USD"); enterLong(tradeUSD,StopUSD);
}

void enterShortUSD(var StopDistUSD)
{
  var StopUSD = priceUSD()+StopDistUSD;
  asset("GBP/USD"); enterShort(tradeUSD,StopUSD);
  asset("AUD/USD"); enterShort(tradeUSD,StopUSD);
  asset("EUR/USD"); enterShort(tradeUSD,StopUSD);
}
 
// plot a price curve of the synthetic asset
// (the plot command is linked to the last used asset -
// so "EUR/USD" must be selected in the scrollbox)
function run() 
{
  set(PLOTNOW);
  plot("USD",priceUSD(),0,RED);
}
```

# II. Indicators & Signals 

### Generate an indicator with a different asset, time frame, and shift 

```c
//extended ATR function with individual asset and timeframe (in minutes)
var extATR(string symbol,int period,int length,int shift){
  ASSET* previous = g->asset; // store previous asset
  if(symbol) asset(symbol);   // set new asset
  if(period) TimeFrame = period/BarPeriod;
// create price series with the new asset / timeframe  vars H = series(priceHigh()),     L = series(priceLow()),    O = series(priceOpen()),
    C = series(priceClose());
  TimeFrame = 1; // set timeframe back
  g->asset = previous; // set asset back  return ATR(O+shift,H+shift,L+shift,C+shift,length);
}
```

### Calculate the weekend price change for gap trading 

```c
// use 1-hour bars, wait until Sunday Sunday 5pm ET, // then get the price change from Friday 5pm ET
if(dow() == SUNDAY && lhour(ET) == 5) {   int FridayBar = timeOffset(ET,SUNDAY-FRIDAY,5,0);  var PriceChange = priceClose(0) - priceClose(FridayBar);
  ...}
```

### Use a series to check if something happened within the last n bars

```c
// buy if Signal1 crossed over Signal2 within the last 7 bars
...
vars crosses = series(0); // generate a series and set it to 0
if(crossOver(Signal1,Signal2)
  crosses[0] = 1; // store the crossover in the series
if(Sum(crosses,7) > 0) // any crossover within last 7 bars?
  enterLong();
...
```

### Use a loop to check if something happened within the last n bars

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```0

### Align a time frame to a certain event

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```1

### Shift a series into the future

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```2

### Use a function or indicator from an external DLL

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```3

# III. Auxiliary

### Find out if you have a standard, mini, or micro lot account 

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```4

### Download historic price data

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```5

### Export historical price data to a .csv file 

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```6

### Export historical price data from MT4 to a .csv file 

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```7

### Print the description of a trade (like "**[AUD/USD:CY:S1234]")** in the log

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```8

### Plot equity curves of single assets in a multi-asset strategy

```c
if(NumOpenTotal > 0) 
  for(open_trades)
    if(TradeIsPending)
      exitTrade(ThisTrade);
```9

### Set up strategy parameters from a .ini file at the start 

```c
for(open_trades)
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80; // 80% profit (minus trade costs)
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
```0

### Check every minute in [Trade] mode if an .ini file was modified

```c
for(open_trades)
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80; // 80% profit (minus trade costs)
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
```1

# IV. Get Rich Quick

### 90% win rate

```c
for(open_trades)
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80; // 80% profit (minus trade costs)
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
```2

### Martingale

```c
for(open_trades)
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80; // 80% profit (minus trade costs)
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
```3

### Grid Trading 

```c
for(open_trades)
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80; // 80% profit (minus trade costs)
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
```4

### See also:

[ Strategy](strategy.htm), [Indicators](ta.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
