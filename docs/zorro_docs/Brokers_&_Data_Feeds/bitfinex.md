---
title: Bitfinex Plugin | Zorro Project
url: https://zorro-project.com/manual/en/bitfinex.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [Asset and Account Lists](account.htm)
- [brokerCommand](brokercommand.htm)
- [trade management](trade.htm)
- [set, is](mode.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [IB / TWS Bridge](ib.htm)
- [Bittrex](bittrex.htm)
- [Binance](binance.htm)
---

# Bitfinex Plugin | Zorro Project

# Bitfinex Plugin

Bitfinex™ is a Hong Kong based digital currency exchange that supports about 200 crypto currencies and partially free API access. With [Zorro S](restrictions.htm), the Bitfinex API plugin can be used with or without a Bitfinex account; in the latter case only price data and history is available. 

For opening a Bitfinex account, visit [ bitfinex.com](https://bittrex.com/) and apply. Demo accounts are not available - you must really deposit some bitcoin or dollar amount. You can set up the bitcoin symbol and the number of decimals to display in prices in the [account list](account.htm). For acessing your account via API, you will need a public and a private key since all API commands must be hash signed. The steps:

  * Login to Bitfinex, in locate the gear icon in the upper-right of the page and click it.
  * Select API Keys and click on Create New Key to create your API key.
  * Select the permissions of the key. For trading, enable "ACCOUNT INFO", "ACCOUNT HISTORY", and "ORDERS" (or just enable everything). 

You're now all set to trade with the Bitfinex plugin. The plugin uses API version 1.1 for trading and prices, and API 2.0 for history.

**User** |  Bitfinex API key, or empty for accessing live prices only  
---|---  
**Password** |  Bitfinex Secret, or empty for accessing live prices only  
  
### Bitfinex asset symbols

The Bitfinex plugin requires symbols in the usual forms XXX/BTC or XXXBTC, where BTC is the exchange account currency and XXX the currency to trade. An asset list **AssetsBitfinex.csv** with about 100 main cryto currencies is included. 

### Supported broker commands

The Bitfinex plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **SET_WAIT**
  * **SET_LIMIT**
  * **GET_POSITION**
  * **GET_MAXREQUESTS**

More commands can be implemented on user request. 

### Known Bitfinex API issues

  * **Price data.** Only very limited historical dafa is available. For getting live volume, use the [SET_VOLTYPE](brokercommand.htm) command with parameter 4.  

  * **Asset parameters.** All data besides spread and price must be manually entered in the asset list. The **PIP** size and **LotAmount** can be set arbitrarily, since Bitfinex has no minimum lot size, but a minimum trade volume; on error messages like **DUST TRADE DISALLOWED MIN VALUE 50K** increase the trade volume or the **LotAmount** in the asset list. Make sure that **PIPCost** is adapted to **LotAmount**. Example asset list entry:  
**ETH/BTC, 0.01, 0.0001, 0, 0, 0.000001, 0.000001, 0, 1, 1, 0,ETHBTC**  
  

  * **Order filling.** The API supports exchange market orders and exchange limit orders. If the order is not filled within a certain time (to be defined with [SET_WAIT](brokercommand.htm)), it is cancelled and the limit can be adapted for the next try. If it is partially filled, the [TradeLots](trade.htm) and [TradeUnits](trade.htm) values of the open trade are accordingly adapted. Positions can be read with the **GET_POSITION** command.  

  * **Compliance.** Bitfinex requires the [NFA](mode.htm) flag. Only long positions are supported; for short trades the account must already contain a sufficient position of the asset. Otherwise an error message like **INSUFFICIENT FUNDS** is issued.  

  * **Trading hours.** Bitfinex trades 24/7.   

  * **Trade and account parameters.** Trade profit is not available via API and estimated by Zorro from the trading costs entered in the asset list. Account requests return the BTC balance by default (the account currency can be set up in the Account column of the account list). The balance is reduced by opening a long position, and increased by closing the position. Equity is estimated by Zorro through summing up the open trades. Example account list entry:   
**Bitfinex, Bitfinex, BTC, 1234567890abcdef, fedcba0987654321, AssetsBitfinex, BTC.B8, 1, 14, Bitfinex.dll  
**
  * **Request rate.** Bitfinex writes on their website: _In order to offer the best service possible we have added a rate limit to the number of REST requests.  
Our rate limit policy can vary in a range of 10 to 90 requests per minute depending on some factors (e.g. servers load, endpoint, etc.)_. That's no idle words: By default, the request rate on a Bitfinex connection is set to 30 requests per minute, but sometimes you need to reduce it indeed to 10 requests per minute (**MaxRequests = 10./60;**). Otherwise price requests and orders return occasionally a **RATE LIMIT** error message.

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MTR4 bridge](mt4plugin.htm), [IB](ib.htm), [Bittrex](bittrex.htm), [Binance](binance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
