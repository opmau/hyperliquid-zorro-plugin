---
title: Fill
url: https://zorro-project.com/manual/en/fill.htm
category: Functions
subcategory: None
related_pages:
- [run](run.htm)
- [enterLong, enterShort](buylong.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Virtual Hedging](hedge.htm)
- [price, ...](price.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [trade management](trade.htm)
- [Spread, Commission, ...](spread.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Trade Statistics](winloss.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Equity Curve Trading](trademode.htm)
- [set, is](mode.htm)
---

# Fill

# Order Filling and HFT Simulation

The **Fill** variable supports several modes for simulating naive or realistic order filling, as well as a HFT simulation mode. **HFT** (**H** igh **F** requency **T** rading) is a trading method that exploits inefficiencies on the millisecond or microsecond time scale. It is very different to normal algorithmic trading. Usual trading accessories such as candles or indicators, are not used. The deciding factor is speed. HFT systems do not run on a trading platform - even Zorro would be not fast enough - but are either programmed in a high speed language such as C, Pentium assembler, GPU language (HLSL or CUDA), or directly relized in hardware using a FPGA design language like VHDL. 

No retail trading platform supports HFT trading, but Zorro can be used for testing HFT systems under different latency conditions. An introduction to HFT testing with Zorro, and an example of a HFT arbitrage system can be found on [Financial Hacker](http://www.financial-hacker.com/hacking-hft-systems/). In Fill mode 8, only direct trading, receiving price quotes, and plotting data are supported; indicators, series, or the [run](run.htm) function are not used. Use the **main** function for initializing the system and for looping through bars. The maximum number of bars must be set up with **MaxBars**. Trades are opened and closed only with [enter](buylong.htm) and [exit](selllong.htm) commands; TMFs, limits, stops or targets are not used in HFT mode. [Hedge](hedge.htm) mode must be 2. Price quotes are entered to the system with the [priceQuote](price.htm) function. Any price quote must include the exchange time stamp, since the system is synchronized to it. The two-way latency between exchange and PC location is set up with [OrderDelay](timewait.htm) and determines the price quote at which the enter or exit order will be filled.

For determining the real latency between your system and an exchange, make sure that your PC clock is exactly in sync with the clock used at the exchange, then compare the exchange time stamp of incoming live price quotes with the arrival time stamp. Double this time for getting the roundtrip latency and add the latecy time of your hardware and software that you should know. As a rule of thumb, the time stamp difference is about 1 ms per any 300 km distance to the exchange (light travels at 300.000 km/sec), plus about 1..2 ms additional delay due to signal routing. **OrderDelay** can be set up with a precision of 1 microsecond. HFT tick data by data vendors has usually a precision of 1 ms. 

## Fill

Determines how order fills are simulated in [Test] and [Train] mode and how orders entries affect other open trades. 

### Modes:

**0** | 'Naive' order filling. Trades open or close at the current market price and at their [Entry](stop.htm), **[Stop](stop.htm)**, or **[TakeProfit](stop.htm)** limits, regardless of slippage and intrabar price movement. Accidental 'cheating' - generating unrealistic profits - is possible by modifying stop or profit limits in a [TMF](trade.htm). The naive fill mode, in combination with [Penalty](spread.htm), can be used for replicating backtest results by other platforms.   
---|---  
**1** | Realistic order filling (default). Trades open or close at the most recent price quote plus [Slippage](spread.htm) or [Penalty](spread.htm). If an entry or exit threshold is hit, the position is filled at a price replicating live trading as accurate as possible (see details below). With daily bars, this mode simulates trading right before the market closes.  
**2** | Alternative order filling. Like fill mode **1** , but entries and exits will be filled at limit even when the market price is better (see details below). This causes often a slightly more pessimistic backtest.  
**3** | Delayed realistic order filling. Trades do not fill immediately, but wait for the next price quote. With daily bars, this mode simulates trading at the market open price of the next day.   
**5** | Delayed alternative order filling.   
**8** | HFT fill mode. Trades open or close at the [priceQuote](price.htm) whose time stamp is most recent to the current time plus [OrderDelay](timewait.htm). This mode can be used for testing HFT systems at different latencies.  
  

### Type:

**int**

### Remarks:

  * High resolution price data for HFT simulation often has outliers. Use the [Outlier](ticktime.htm) variable to filter out extreme prices that can distort the backtest. 
  * Delayed order filling affects trade statistics separately for entering and for filling the order. As long as the order is not filled, it is assumed pending. [NumPendingTotal](winloss.htm) is increased when the order is entered, and [NumOpenTotal](winloss.htm) when it is filled.
  * Fill modes have no effect on the display of trade start or end points on the [chart](chart.htm), which are always placed on the first and last bar of the trade.
  * Fill prices are normally rounded to the point value of the asset. [TR_FRC](trademode.htm) enables unrounded fill prices.
  * When [Penalty](spread.htm) is set, the fill price always is the given threshold plus or minus **Penalty** , depending on trade direction. 
  * Fill prices by triggering an [Entry](stop.htm), [Stop](stop.htm), or [Takeprofit](stop.htm) threshold are determined by the current fill mode, the slippage, penalty, and the open, high, and low prices of the historical bar or tick that hit the price level. If [TICKS](mode.htm) is not set and thus intrabar ticks are not evaluated, an intrabar order fill is estimated with the following algorithm:  
  
**Event** | **Fill = 0** | **Fill = 1** | **Fill = 2**  
---|---|---|---  
Long Entry stop | Entry | Max of Entry and Open | Max of Entry and Open  
Long Entry limit: | Entry | Min of Entry and Open | Entry  
Short Entry stop | Entry | Min of Entry and Open | Min of Entry and Open  
Short Entry limit: | Entry | Max of Entry and Open  | Entry   
Long Stop | Stop | Min of Stop and Open | Min of Stop and Open  
Long TakeProfit | TakeProfit | Max of TakeProfit and Open | TakeProfit  
Short Stop | Stop | Max of Stop and Open | Max of Stop and Open  
Short TakeProfit: | TakeProfit | Min of TakeProfit and Open | TakeProfit  
Long OrderDelay | Open or Entry | Max of Entry and Midprice | Max of Entry and Midprice  
Short OrderDelay | Open or Entry | Min of Entry and Midprice | Min of Entry and Midprice  

### Example:

```c
function run()
{
  BarPeriod = 1440;
  Fill = 3; // enter trades at next day's open 
  ...
}
```

### HFT simulation framework:

```c
// Simulate a stock trading HFT system

#define LATENCY  2.5   // simulate 2.5 milliseconds latency

typedef struct QUOTE {
  char  Name[24];
  var  Time,Price,Size;
} QUOTE; // Quote struct by the NxCore plugin

int callback(QUOTE *Quote) // called by the plugin
{
  static int Counter = 0; // quote counter
  if(0 == (++Counter % 1000)) { // every 1000 quotes...
    if(!wait(0)) return 0; // check if [Stop] was clicked
  }
    
  asset(Quote->Name+1); // NxCore adds "e" to the asset name
  priceQuote(Quote->Time,Quote->Price);
  ...
// trade algorithm here, generate TradeSignalLong/Short
  ...
  if(!(NumOpenLong+NumPendingLong) && TradeSignalLong) {
    exitShort();
    enterLong();
  } else if(!(NumOpenShort+NumPendingShort) && TradeSignalShort) {
    exitLong();
    enterShort();
  }
  return 1;
}

function main()
{
  if(Broker != "NxCore") {
    quit("Please select NxCore plugin!");
    return;
  }

  StartDate = 20170103;
  EndDate = 20170131;
  LookBack = 0;
  set(LOGFILE);
  assetList("AssetsHFT");
  OrderDelay = LATENCY/1000.;
  Hedge = 2;
  Fill = 8;
  Lots = 1;
  Verbose = 3;

// process the NxCore history tape files, one for each day
  login(1);	// for initilizing the plugin
  int NxDate;
  for(NxDate = StartDate; NxDate = EndDate; NxDate++) {
    string NxHistory = strf("History\\NxTape%8d.nx2",NxDate);
    printf("\nProcessing tape %s..",NxHistory);
    brokerCommand(SET_HISTORY,NxHistory);
    if(!wait(0)) break; // check if [Stop] was clicked
  }
}
```

### See also:

[enter](buylong.htm), [exit](selllong.htm), [Hedge](hedge.htm), [Slippage](spread.htm), [Penalty](spread.htm), [mode](mode.htm), [priceQuote](price.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
