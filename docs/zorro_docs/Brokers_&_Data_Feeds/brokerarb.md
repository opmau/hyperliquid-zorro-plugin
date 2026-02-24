---
title: Broker Arbitrage with Zorro
url: https://zorro-project.com/manual/en/brokerarb.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Brokers & Data Feeds](brokers.htm)
- [Licenses](restrictions.htm)
- [Asset and Account Lists](account.htm)
- [Asset Symbols](symbol.htm)
- [Historical Data](history.htm)
- [Equity Curve Trading](trademode.htm)
---

# Broker Arbitrage with Zorro

# Broker Arbitrage: Trading with multiple brokers

Trading simultaneously with multiple accounts, brokers, exchanges, or data feeds allows **broker arbitrage** , i.e. exploiting prices differences of the same asset between different market places. Arbitrage opportunities appear for instance when Forex [liquidity providers](brokers.htm) deviate from each other, or when [market makers](brokers.htm) do not precisely follow the market. There is no central exchange for currencies and CFDs, so their prices vary from broker to broker. If the price differences temporarily exceed trading costs, you can collect risk-free profits. 

[Zorro S](restrictions.htm) can simultaneously trade with up to 12 brokers or price data feeds. It allows trading strategies to compare currency or CFD prices between broker A and broker B, and enter a long position with the cheaper broker and a short position with the other. Here's the step by step setup and a code example for taking advantage of Forex/CFD price differences. But be aware that brokers dislike broker arbitrage practices, and may close accounts when they learn that they were exploited in this way.

  * Brokers A and B must not use the same broker plugin Dll. If they do, simply duplicate the DLL in the Plugin folder and give the copy a different name (f.i. **ZorroMT4b.dll**). When both brokers trade through MT4 or MT5, make also sure that you have two different client terminals versions installed, one for broker A and one for broker B, because neither MT4 nor MT5 supports simultaneous trading on multiple accounts. (MT4/MT5 are not really suited for broker arbitrage anyway due to their high latency - you'll normally need a Zorro plugin with a direct API connection).  

  * Set up an [account list](account.htm) with accounts from brokers A and B. Example:  
**Name** | **Broker** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
BrokerA | AAA | 0 | 4009876 | n4qw4ert7 | AssetsArb | USD | 1 | 0 | Broker1.dll  
BrokerB | BBB | 0 | 3456789 | ab234qrz2 | AssetsArb | USD | 1 | 0 | Broker2.dll  
  

  * Set up the account names in the [Symbol](symbol.htm) fields of the used [asset list](account.htm). If an account name is addedin front of a symbol, the account is used for prices, history, and trading of that asset:  
  
**Name** | **Price** | **Spread** | **Roll** | **Roll** | **PIP** | **Cost** | **Margin** | **Market** | **Amount** | **Comm** | **Symbol**  
---|---|---|---|---|---|---|---|---|---|---|---  
EURUSD_A | 1.17311 | 0.00005 | 0 | 0 | 0.0001 | 0.1 | -0.01 | 0 | 1000 | 0.6 | BrokerA:EUR/USD  
EURUSD_B | 1.17299 | 0.00007 | 0 | 0 | 0.0001 | 0.1 | -0.01 | 0 | 1000 | 0.5 | BrokerB:EUR/USD  
  
The symbols select **BrokerA** as price source and trading target for **EURUSD_A** , and **BrokerB** likewise for **EURUSD_B**. If you wanted an asset to trade with **BrokerB** , but to get prices from **BrokerA** , you had to split the symbol in two, separated by an exclamation mark: **BrokerB:EUR/USD!BrokerA:EUR/USD**. In the examples below you'll find a small script to automatically combine several asset lists to a multi-broker asset list.  

  * For backtesting, use the [Download](history.htm) script and get **.t1** price histories of the same asset from brokers A and B. Rename the history files to the names used in the asset list (f.i. **EURUSD_A_2017.t1** and **EURUSD_B_2017.t1**). Run a backtest in **.t1** mode. For an accurate backtest, the historical data must be identical to live data, and have the same resolution. Some brokers, such as **IB** and **Oanda** , deliver historical tick data only in reduced resolution. This data is not suited for broker arbitrage backtests.  

  * For live trading, select either **BrokerA** or **BrokerB** from the [Account] scrollbox. The selected broker determines the time and price displayed in the [Server] window, as well as the displayed balance and equity. For trades, use market orders, not limit orders. If a limit order is not filled with broker A, but filled with broker B, you won't have arbitrage, but are fully exposed to the market risk.  
  

### Example:

```c
// Simple broker arbitrage example ////////////////////////////

function tick()
{
  asset("EURUSD_A");
  var SpreadA = Spread, PriceA = priceClose(), 
    CommissionA = Commission*LotAmount/10000*PIP/PIPCost; // convert commission to price difference 
  asset("EURUSD_B");
  var SpreadB = Spread, PriceB = priceClose(), 
    CommissionB = Commission*LotAmount/10000*PIP/PIPCost;

  var Threshold = 1.5*(SpreadA+SpreadB+CommissionA+CommissionB); // arbitrage threshold
  var Difference = PriceA - PriceB;

  asset("EURUSD_A");
  if(NumOpenShort && Difference < 0)
    exitShort();
  else if(NumOpenLong && Difference > 0)
    exitLong();
  else if(!NumOpenShort && Difference > Threshold) // go short with the expensive asset
    enterShort();
  else if(!NumOpenLong && Difference < -Threshold) // go long with the cheap asset
    enterLong();

  asset("EURUSD_B");
  if(NumOpenShort && Difference > 0)
    exitShort();
  else if(NumOpenLong && Difference < 0)
    exitLong();
  else if(!NumOpenShort && Difference < -Threshold)
    enterShort();
  else if(!NumOpenLong && Difference > Threshold)
    enterLong();
}

function run()
{
  StartDate = EndDate = 2017;
  LookBack = 0;
  set(TICKS);
  History = "*.t1";
  assetList("AssetsArb.csv");
  asset("EURUSD_A");
  asset("EURUSD_B");
}
```

```c
// Generate a multi-broker asset list ///////////////////////

#define LISTS "FXCM","Oanda"
#define ASSETLIST "AssetsMulti"

function main() 
{
  char OutName[NAMESIZE2]; // to store a temporary string
  strcpy(OutName,strf("History\\%s.csv",ASSETLIST));
  file_delete(OutName);
  string Name, Source = 0;
  while(Name = of(LISTS))
  {
    static char Dest[40000]; 
    file_read(strf("History\\Assets%s.csv",Name),Dest,0);
    if(!*Dest) return quit("Source not found!");
    string Pos = strchr(Dest,'\n');
// skip header line after first file
    if(!Source) Source = Dest;
    else Source = Pos;
    while(Pos) {
      Pos = strchr(Pos+1,',');
      if(!Pos) break;
// append list name to asset name
      strcatf(Pos,strf("_%s",Name));
      int i;
      for(i=0; i<11; i++) // find Symbol column
        Pos = strchr(Pos+1,',');
// append list name as source to symbol
      strcatf(Pos+1,strf("%s:",Name));
      Pos = strchr(Pos,'\n');
    }
    file_append(OutName,Source,0);
  }
}
```

### See also:

[Account list](account.htm), [Symbols](symbol.htm), [Brokers](brokers.htm), [TradeMode](trademode.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
