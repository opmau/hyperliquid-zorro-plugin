---
title: IEX Plugin
url: https://zorro-project.com/manual/en/iex.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [BarMode](barmode.htm)
- [brokerCommand](brokercommand.htm)
- [price, ...](price.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [IB / TWS Bridge](ib.htm)
- [FXCM](fxcm.htm)
---

# IEX Plugin

# IEX Cloud Plugin

IEX is a stock exchange for US equities with a focus on fair and transparent trading. It applies technical measures for preventing fake orders or fore-running methods, and thus enforces equal chances for its traders.

The IEX plugin allows access to live and historical [data provided by IEX Cloud](https://iexcloud.io). For using it with Zorro, register on [https://www.iexcloud.io](https://www.iexcloud.io/) and get an API token.

Zorro login fields for IEX:

**User:** | IEX API Token  
---|---  
**Password:** | -  
  
[Accounts.csv](account.htm) example entry:

**Name** | **Broker** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
IEX_Cloud | IEX | 0 | pk_4711d2f6c6a4434babc67384ec6e25 | 0 | AssetsSP30 | USD | 0 | 0 | IEX  
  
### Remarks

  * The IEX plugin is a mere price source. It cannot be used for trading on the IEX exchange. Prices are only available for US stocks.
  * The IEX cloud has a limit on the number of requests per month. If your allowance goes to zero, you won't be able to request live prices or download historical data. For limiting the number of requests, increase the [TickTime](ticktime.htm) for reducing the quote frequency, and switch off price requests with [BR_SLEEP](barmode.htm) or with the [SET_PRICETYPE](brokercommand.htm) broker command when you don't need them.
  * The IEX cloud has a limit on M1 historical data. Only the last 2-3 months are available, so it can be used for a lookback period, but not for historical data. Historical EOD data can be loaded with **assetHistory(FROM_IEX,...)** ,

### Additional data and commands

The IEX plugin supports the following additional data streams: 

  * [marketVal](price.htm): Not supported in historical data, bid-ask spread in live data.
  * [marketVol](price.htm): Trade volume per minute in historical data; accumulated volume since market open in live data.

The IEX plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PRICETYPE - 1** for IEX ask/bid, **2** for last traded price, **4** for stopping price requests.
  * **GET_DATA**

### See also:

[Brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MT4 bridge](mt4plugin.htm), [IB bridge](ib.htm), [FXCM plugin](fxcm.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
