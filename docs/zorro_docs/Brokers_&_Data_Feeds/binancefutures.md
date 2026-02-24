---
title: Binance Futures
url: https://zorro-project.com/manual/en/binancefutures.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Binance](binance.htm)
- [Licenses](restrictions.htm)
- [Asset and Account Lists](account.htm)
- [contract, ...](contract.htm)
- [brokerCommand](brokercommand.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [tick, tock](tick.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# Binance Futures

# Binance Futures Plugin 

**Binance** is a Shanghai founded digital currency exchange that supports all major crypto currencies and crypto futures. The Binance Futures plugin is for trading cryptocurrency futures. For trading plain coins (spot market), use the [Binance](binance.htm) plugin.

If you already have a Binance account for the spot market, you can extend it to futures. You will need a new API key and secret for trading futures. This key will then also work for logging in to the spot market. Make sure that your account is in one-way mode (no hedging), since the plugin assumes single-side positions.

There are two Binance Futures plugins. One was developed by a Zorro user and is available on [ Github](https://github.com/mioxtw/BinanceFuturesZorroPlugin). The other one is included in the Zorro setup and described here. It requires [Zorro S](restrictions.htm) and an account list. It uses the Websocket API for streaming market data and the REST API for all other fuctions. There are two sorts of futures available that are treated slightkly differently: USDT futures, paid in dollar equivalents, and COIN futures, paid in crypto coins. Real mode uses the Binance production server, demo mode the test server. Thus, 4 different endpoints are available that can be set in the **Server** field of the account list:

USDT futures production endpoint: **wss://fstream.binance.com**  
COIN futures production endpoint: **wss://fstream.binance.com**  
USDT futures test endpoint: **wss://fstream.binancefuture.com**  
COIN futures test endpoint: **wss://fstream.binancefuture.com**

Zorro login fields for Binance Futures:

**User** | API Key  
---|---  
**Password** | API Secret  
  
[Accounts.csv](account.htm) example entries:

**Name** | **Server** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
BinFut-USDT | wss://fstream.binance.com | USDT |  (Key) |  (Secret) | AssetsBF | USDT | 1 | 14 | BinanceFutures  
BinFut-COIN | wss://dstream.binance.com | BTC |  (Key) |  (Secret) | AssetsBF | BTC.8 | 1 | 14 | BinanceFutures  
  
### Binance Futures symbols

Binance Fututures has inconsistent symbols across all endpoints, but the plugin takes care of this. It uses the below symbology:  
  
Perpetual Futures: **AAABBB** , e.g. **"BTCUSDT"**  
Expiring Futures: **AAABBB-FUT-YYYYMMDD** , e.g. **"BTCUSDT-FUT-20210326"**  
Alternatively, use the [ contract](contract.htm) functions for futures, using the currency pair "BTCUSDT" as the underlying.

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) f function with the following commands:

  * **SET_PATCH**
  * **GET_COMPLIANCE**
  * **GET_MAXTICKS**
  * **GET_HEARTBEAT**
  * **GET_POSITION**
  * **SET_SYMBOL**
  * **GET_DELAY**
  * **SET_DELAY**
  * **GET_WAIT**
  * **SET_WAIT**
  * **GET_BOOK**
  * **SET_ORDERTYPE**(0:IOC, 1:FOK, 2:GTC)
  * **GET_FUTURES**
  * **DO_CANCEL**
  * **SET_SERVER**

There are 2 plugin specific commands:

  * **2001** : Reconnects the websocket at any time for any reason. 
  * **2002** : Returns number of milliseconds since the plugin received any message from the websocket server.

### Remarks

  * **Maximum request rate.** For limiting the request rate, use **SET_DELAY** for delaying client account and position requests. Don't use **MaxRequests** , since it would also throttle price requests that however have no limit due to the websocket implementation.  

  * **Minimum request rate.** The Binance server was reported to time out when no price requests are received for a certain time. On scripts with long bar periods, make sure to set [TickTime](ticktime.htm) so that a price request is sent every 5..10 seconds.   

  * **Asset parameters**. Spread, price, PIP, and LotAmount are available from the API. All other data must be manually entered as described under [asset list](account.htm). For assets with variable leverage, set up [Leverage](pip.htm) and [ MarginCost](pip.htm) by script before entering a trade. Example USDT asset list entries, assuming 100:1 leverage and 0.25% commission: 

```c
Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,Margin,Market,LotAmount,Commission,Symbol
BTCUSDT,57125,3.5,0,0,0.01,0.00001000,-1,0,0.001,-0.25,BTCUSDT
BTCUSDT-210326,57125,3.5,0,0,0.01,0.00001000,-1,0,0.001,-0.25,BTCUSDT-FUT-20210326
ETHUSDT,1991,0.05,0,0,0.01,0.00001000,-1,0,0.001,-0.25,ETHUSDT
ETHUSDT-210626,1991,0.05,0,0,0.01,0.00001000,-1,0,0.001,-0.25,ETHUSDT-FUT-20210326
```

Example COIN asset list entries:  

```c
Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,Margin,Market,LotAmount,Commission,Symbol
BTCUSD,57125,3.5,0,0,0.1,0.00017505,-1,0,1,-0.25,BTCUSD
BTCUSD-210326,57125,3.5,0,0,0.1,0.00017505,-1,0,1,-0.25,BTCUSD-FUT-20210326
ETHUSD,1991,0.05,0,0,0.01,0.00005023,-1,0,1,-0.25,ETHUSD
ETHUSD-210326,1991,0.05,0,0,0.01,0.00005023,-1,0,1,-0.25,ETHUSD-FUT-20210326
```

In the above examples, the **Name** column uses a shortened version of the futures contract name. The **Symbol** column must follow the above nomenclature.  

  * **Contract specifications.** For USDT Futures the order volume is in units of base currency. **LotAmount** can be less than or greater than 1. For COIN Futures the order volume is per contract. A contract is worth a multiple of the counter currency, such as 10 USD, 100 USD, depending on the contract. **LotAmount** is normally 1.  

  * **Funding.** On the USDT exchange, you must have the counter currency to trade; on the COIN exchange, you must have the base currency to trade.  

  * **Compliance.** Binance Futures requires the **NFA** flag.  
  

  * **Stability.** Users reported irregular breakdowns of the Binance Futures server, causing loss of connection with the websocket API. Two commands have been added for reconnecting automatically by script in such a case, for instance in a [tock](tick.htm) function. Example: **if(brokerCommand(2002,0) > 120000) brokerCommand(2001,0);** will reconnect when the connection was lost for more than 2 minutes.  

### See also:

[Brokers](brokers.htm), [broker plugin](brokerplugin.htm), [brokerCommand](brokercommand.htm), [Binance plugin](binance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
