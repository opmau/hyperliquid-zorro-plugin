---
title: Deribit Plugin
url: https://zorro-project.com/manual/en/deribit.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [contract, ...](contract.htm)
- [brokerCommand](brokercommand.htm)
- [set, is](mode.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [Binance](binance.htm)
---

# Deribit Plugin

# Deribit Plugin 

**[Deribit](https://www.deribit.com/)** is a cryptocurrency derivatives exchange. It trades futures and options on Bitcoin (BTC) and Etherium (ETH). For speed reasons the Deribit plugin uses uses the Websocket API for most functionality and REST for historical data. Deribit has a production and a test API; the plugin connects in real mode to the production API and in test mode to the testnet API. 

**User** | Client ID  
---|---  
**Password** | Client Secret  
  
[Accounts.csv](account.htm) example entry:

**Name** | **Server** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
Deribit-Real | Production | 0 |  0 |  0 | AssetsDeribit | USD | 1 | 14 | Deribit.dll  
Deribit-Demo | Testnet | 0 |  0 |  0 | AssetsDeribit | USD | 0 | 14 | Deribit.dll  
  
### Deribit asset symbols

Deribit has 4 types of symbols: Index, Perpetual Futures, Futures, and Options.  
  
Index: Coin name, either "BTC" or "ETH".  
Perpetual Futures: [Coin]-PERPETUAL, e.g. "ETH-PERPETUAL".  
Futures: [Coin]-DDMMMYY, where MMM is a 3-letter capitalized string, e.g. "BTC-26MAR21".  
Options: [Coin]-DDMMMYY-[Strike]-[P/C], e.g. "ETH-25JUN21-340-C".  
For options and futures, the [contract](contract.htm) functions can be used with "BTC" or "ETH" as the underlying.

### Supported broker commands

The Deribit plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_AMOUNT**
  * **GET_MAXTICKS**
  * **SET_SYMBOL** ("BTC" or "ETH")
  * **GET_BOOK**(selected/subscribed assets only, otherwise no output)
  * **SET_DIAGNOSTICS**
  * **GET_COMPLIANCE**
  * **GET_DELAY**
  * **SET_DELAY** (affects only REST requests)
  * **GET_WAIT**
  * **SET_WAIT**
  * **DO_CANCEL**
  * **SET_ORDERTYPE** (default=GTC, 2=GTC, 10=FOK, 11=IOC)
  * **GET_POSITION**
  * **GET_OPTIONS**
  * **GET_FUTURES**
  * **GET_TRADES**

### Known Deribit API issues

  * **Rate limitation.** Deribit has a strange and complex rate limiting policy. To avoid bans, **SET_DELAY** was preconfigured to sufficiently slow responses. **SET_DELAY** affects all REST requests, but no websocket streams.   

  * **Asset parameters.** All data besides spread and price must be manually entered in the [asset list](account.htm). Example entries:  

```c
Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,MarginCost,Leverage,LotAmount,Commission,Symbol
BTC,57125,3.5,0,0,0.0001,0.0001,0,0,0.0001,0,*
BTC-PERPETUAL,57125,3.5,0,0,0.05,0.0000111643,0.0000022329,0,10,-0.25,*
BTC-26MAR21,57125,3.5,0,0,0.05,0.0000111643,0.0000022329,0,10,-0.25,*
ETH,1991,0.05,0,0,0.0001,0.0001000000,0,0,0.0001,0,*
ETH-PERPETUAL,1991,0.05,0,0,0.05,0.0000011164,0.0000004466,0,1,-0.25,*
ETH-26MAR21,1991,0.05,0,0,0.05,0.0000011164,0.0000004466,0,1,-0.25,*
```

  * **Orders.** Deribit requires the [NFA](mode.htm) flag. The default order type is GTC; other order types are supported, but rejected under many circumstances. You need BTC funds to trade BTC derivatives and ETH funds to trade ETH derivatives. For futures the order volume is in USD of the underlying, multiples of 10 for BTC and multiples of 1 for ETH. Quotes are in BTC/USD and ETH/USD. For options: the order volume is in BTC of underlying. Premiums are in BTC.

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [Binance](binance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
