---
title: Workshop 8 - Options trading
url: https://zorro-project.com/manual/en/tutorial_options.htm
category: Strategy Coding, 1-8
subcategory: None
related_pages:
- [W7 - Machine Learning](tutorial_pre.htm)
- [Asset and Account Lists](account.htm)
- [IB / TWS Bridge](ib.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [contract, ...](contract.htm)
- [enterLong, enterShort](buylong.htm)
- [combo, ...](combo.htm)
- [Links & Books](links.htm)
- [Tips & Tricks](tips.htm)
- [Troubleshooting](trouble.htm)
- [Migrating Scripts](conversion.htm)
---

# Workshop 8 - Options trading

[Previous: Machine Learning ](tutorial_pre.htm)

# Workshop 8: Options Trading

An **option** is a contract that gives its owner the right to buy (**call option**) or sell (**put option**) a financial asset (the **underlying**) at a fixed price (the **strike** price) at or before a fixed date (the **expiration** date). Options trading has the reputation to be more rewarding, but also more complex than trading currencies or stocks. At least for writiung the script, the latter is not true; Zorro allows easy and straightforward coding of options strategies. Here's an example of a simple options selling system (**Workshop8.c**): 

```c
#include <contract.c>

#define PREMIUM  3.00 // $300 per contract 
#define DAYS     6*7  // 6 weeks to expiration

void run() 
{
  set(PLOTNOW,LOGFILE);
  BarPeriod = 1440;

  History = ".t8"; // use options for unadjusted price history
  assetList("AssetsIB");
  asset("SPY");
  Multiplier = 100;

// load today's contract chain
  if(!contractUpdate(Asset,0,CALL|PUT)) return; 

  if(!NumOpenShort) { 
// find contracts with 6 weeks expiration and $2 premium
    if(combo(
      contractFind(CALL,DAYS,PREMIUM,2),1, 
      contractFind(PUT,DAYS,PREMIUM,2),1,
      0,0,0,0)) 
    { // sell a Strangle
      MarginCost = comboMargin(-1,3);
      enterShort(comboLeg(1));
      enterShort(comboLeg(2));
    }
  }
}
```

You need to [ download SPY options data](https://zorro-project.com/download.php) for backtesting this system. There are a few differences to a 'normal' trading system. We're using an [asset list](account.htm) for the broker [IB](ib.htm) that contains the underlying stock, SPY, a fund following the S&P500 index. Option contracts usually cover batches of N stocks, often 100, which must be set through [Multiplier](contracts.htm). The [contractUpdate](copntract.htm) function loads the current **options chain** \- the list of options with all available combinations of expirations and strikes - either from historical data, or from the broker API. If no positions are open, the strategy attempts to sell short a put and a call contract with 6 weeks expiration at a bid price of 2 dollars per stock. The [contractFind](contract.htm) function scans the options chain for fitting contracts.

The two contracts that match the desired bid price and expiration are now combined to a combo. Options and option combos can be traded with the usual [ enter](buylong.htm) functions, they must only be selected before with [contract()](contract.htm) or [comboLeg()](combo.htm) to distinguish the order from a trade with the underlying. The earned price - the **premium** , in total about 400 dollars minus commission - is booked on the account. That's not yet our profit, since we might be later obliged to sell or buy the underlying SPY stock at the strike price. The broker - or Zorro, respectively - will do that automatically when the underlying price at expiration date is not in our favor, and will deduct the difference of price and strike from our account. Otherwise, the contracts will expire worthlessly and we can keep the premium.

This combination of a put and a call contract is called a **Strangle**. Because we aimed for small premiums, our contracts start **out of the money** , meaning that the underlying price is below the call strike and above the put strike. This way we will normally win the premium, or most of it, when the SPY price does not move too far away until expiration. Otherwise, we'll lose. If the price moves a lot, we'll lose a lot. 

Mind the **MarginCost** calculation with the [ comboMargin](combo.htm) function. It is needed for a realictic backtest, since margin cost determines the required capital. 

### The result  

![](../images/workshop8.png)

We can see that in the test period 2012-2018 the strategy achieves not spectacular, but relatively constant annual returns in the 10% area. About 70% of trades are won. Still, it is not advisable to trade this system unchanged - you'll see that when you extend the test period beyond 2018. The system depends on a rising market. When the market tanks, it will fail bad since it has no mechanism for filtering unprofitable periods and limiting losses. We could place a **stop loss** just like for a normal trading system. The contracts are then automatically bought back when the underlying price moves dangerously far. Or we could buy 'insurance' in the form of two additional contracts with more distant strikes. This long/short combination of 4 contracts reduces the profit, but also restricts losses to a limit that can be determined with the strike prices. The **Payoff** script can display the risk and profit curves of arbitrary option combinations. In the [Black Book](links.htm) you can find a different options selling system. It survives market crashes with a filter that prevents trading in volatile situations. 

We're now at the end of the strategy coding course. For writing your own systems, it can save you a lot of time when you flip through this manual and make yourself familiar with Zorro's math and statistics functions. Often-used code snippets for your own scripts and strategies can be found on the [Tips & tricks](tips.htm) page. Writing good code and fixing bugs is described under [ Troubleshooting](trouble.htm). If you worked with a different trade platform before, read the [Conversion](conversion.htm) page about how to convert your old scripts or EAs to C. For serious strategy development, some knowledge of the leading data analysis software **R** can be of advantage - check out the [R lectures](Lecture%201.htm).

### What have we learned in this workshop?

  * Selling options can produce small, but steady profits. 
  * The [contract](contract.htm) functions are used for analyzing and trading options and futures. 
  * Risk can be limited with market filters and options combinations.
  * Historical data for backtesting simple options systems can be artificially generated.

* * *

### Further reading: ►[contract](contract.htm), [combo](combo.htm)
