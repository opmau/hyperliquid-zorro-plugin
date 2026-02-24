---
title: Algo Trading with Kraken
url: https://zorro-project.com/manual/en/kraken.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [price, ...](price.htm)
- [brokerCommand](brokercommand.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [set, is](mode.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [BarMode](barmode.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [Binance](binance.htm)
---

# Algo Trading with Kraken

# Kraken

**Kraken** is a San Francisco based digital currency broker. It supports about 40 crypto currencies and allows algorithmic trading with leverage and shorting. It offers a REST API and a Websocket API. For speed reasons, the Kraken plugin for Zorro uses Websocket whenever possible. Real mode and Demo mode is available, the latter for simulated trading only. 

**User** | Primary account currency, f.i. USD or XBT  
---|---  
**Password** | PublicKey(space)PrivateKey  
  
[Accounts.csv](account.htm) example entry:

**Name** | **Server** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
Kraken-XBT | Kraken | 0 |  XBT |  PublicKey PrivateKey | AssetsKraken | XBT.B8 | 1 | 14 | Kraken.dll  
  
### Kraken asset symbols

The Kraken plugin requires symbols in the usual form XXX/YYY, where YYY is the counter currency and XXX the currency to trade. 

Volume is per-bar volume in historical data, accumulated session volume in live data. 

### Supported data and commands

The Kraken plugin supports the following additional data streams: 

  * [marketVal](price.htm): Not supported in historical data, bid-ask spread in live data.
  * [marketVol](price.htm): Trade volume per minute in historical M1 data; accumulated volume since midnight in live data. 

The Kraken plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_AMOUNT**
  * **GET_MAXTICKS**
  * **SET_SYMBOL**
  * **GET_BOOK**
  * **SET_DIAGNOSTICS**
  * **GET_COMPLIANCE**
  * **GET_DELAY**
  * **SET_DELAY**
  * **GET_WAIT**
  * **SET_WAIT**
  * **DO_CANCEL**
  * **SET_ORDERTYPE** (0=IOC, 2=GTC)
  * **SET_LEVERAGE** (some coins only, see website)
  * **GET_POSITION** (currency only; returns the balance of the given currency. See **GET_ALL_BALANCES**).

The following additional custom commands are supported:

  * **6001 (DO_CANCEL_ALL_ORDERS)** \- Indiscriminately closes all open orders on the account.
  * **6000 (GET_ALL_BALANCES)** \- Reads all currency balances, see below.

Usage of the custom commands in Zorro scripts:  

```c
#define GET_ALL_BALANCES 6000#define DO_CANCEL_ALL_ORDERS 6001
typedef struct KRAKEN_BALANCE {   char sCurrency[8];   double vBalance;} KRAKEN_BALANCE;
...

KRAKEN_BALANCE KrakenBalances[100]; 
brokerCommand(GET_ALL_BALANCES,KrakenBalances);
```

  

### Known Kraken API issues

  * **Rate upper limit.** Kraken has a strange and complex rate limiting policy. To avoid bans, **SET_DELAY** was preconfigured to sufficiently slow responses. **SET_DELAY** affects all REST requests, but no open websocket streams.   
  

  * **Rate lower limit.** The Kraken server will time out when no price requests are received for a certain time. On scripts with long bar periods, make sure to reduce [TickTime](ticktime.htm) so that a price request is sent at least every 1..5 seconds (f.i.**TickTime = 250;**).  

  * **Asset parameters.** All data besides spread and price must be manually entered in the asset list. **PIP** size and **LotAmount** can be set arbitrarily, but Kraken has minimum lot sizes for some assets. Lots sizes and leverages can be taken from the [ Kraken website](https://support.kraken.com/hc/en-us/articles/227876608-Margin-trading-pairs-and-their-maximum-leverage) and converted to the corresponding lot amounts and margin percentages (f.i. Leverage 5 = 20%). Example [asset list](account.htm) entries with leverages:  

```c
BCH/USD,0.53794,0.0001,0,0,0.0001,0.0001,-33.33,0,0.00001,-0.25,*
ETH/USD,450.2,0.01,0,0,0.01,0.01,-20,0,0.00001,-0.25,*
ETH/XBT,0.0292241,0.00001,0,0,0.00001,0.00001,-20,0,0.00001,-0.25,*
LTC/ETH,0.13093,0.00001,0,0,0.00001,0.00001,0,0,0.00001,-0.25,*
LTC/USD,179,0.01,0,0,0.01,0.01,-33.33,0,0.00001,-0.25,*
LTC/XBT,0.003834,0.000001,0,0,0.000001,0.000001,-33.33,0,0.00001,-0.25,*
XBT/USD,44785.8,3.3,0,0,0.01,0.01,-20,0,0.00001,-0.25,*
```

  * **Orders.** Kraken requires the [NFA](mode.htm) flag. Lot amounts are not fixed, but assets have individual minimum order sizes that can be found on the Kraken website. If less than the minimum size is ordered, the order is rejected with an error message like **"Invalid arguments:volume"**. Some assets support variable leverage that can be set up in the asset list as above, or with the [ Leverage](pip.htm) variable directly before entering the trade. Shorting an asset is usually only possible at leverage 2 or above.   
  

  * **Trading hours.** Kraken trades 24/7. For trading on the weekend, set [BarMode](barmode.htm) accordingly in your script.  

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [Binance](binance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
