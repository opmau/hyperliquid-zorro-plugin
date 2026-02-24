---
title: Coinigy Plugin
url: https://zorro-project.com/manual/en/coinigy.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset Symbols](symbol.htm)
- [brokerCommand](brokercommand.htm)
- [assetHistory](loadhistory.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [set, is](mode.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [IB / TWS Bridge](ib.htm)
- [Bittrex](bittrex.htm)
---

# Coinigy Plugin

# Coinigy Plugin 

Coinigy™ offers an API access to many digital currency exchanges, so Zorro can trade with the same plugin on various exchanges, even simultaneously. The service is free for the first weeks, afterwards it's about $20 per month.

For opening a Coinigy account, visit [ https://coinigy.com/](https://coinigy.com/) and apply. You'll then receive a public and a private key for accessing the API. You can then add to your Coinigy account all digital exchanges where you want to trade, and enter their credentials. You need at least one exchange, or else the Coinigy plugin will not work. For placing orders with Coinigy, Two-Factor Authentication must be activated, and trading on the exchange must be enabled (both with the exchange and with Coinigy). The procedures are explained on the Coinigy website.

The Coinigy plugin uses API version 1.

**User** | Coinigy API key  
---|---  
**Password** | Coinigy Secret  
  
Example account list entry: 

**Coinigy,Coinigy,BTC,1234567890abcdef,fedcba0987654321,AssetsCoinigy,BTC.B8, 1,14,Coinigy.dll,**

### Coinigy asset symbols

Coinigy uses [symbols](symbol.htm) in the form XXX/YYY, where YYY is the counter currency and XXX the currency to trade. Bittrex symbols in the form YYY-XXX are also supported and automatically converted. Which assets you can trade depends on which exchanges you have added to your Coinigy account. The exchange symbol can be appended to an XXX/YYY symbol, for instance BTC/ETH-KRKN (Zorro 2.33 or above). At login, a string of supported exchange symbols is printed to the message window.

### Supported broker commands

The Coinigy plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **SET_WAIT**
  * **SET_AMOUNT**
  * **GET_POSITION**
  * **SET_BROKER**
  * **SET_ORDERTYPE**(0 or 2)
  * **SET_SYMBOL**
  * **GET_BOOK**
  * **GET_DATA**
  * **DO_CANCEL**

### Known Coinigy API issues

  * **Speed.** All commands and responses are transferred between Coinigy and the selected exchange or brokerage. This causes additional latency. The time for a balance or price request can be in the second range, so for high speed trading or large portfolios, a direct connection to the exchange is preferable. Coinigy has an internal limit of 2 requests per seconds.  
  

  * **Historical price data.** Coinigy API V1 does not provide sufficient historical data, so it must be downloaded from another source, f.i. Bittrex, CryptoCompare, or Coin.io. Use the [ assetHistory](loadhistory.htm) function or enter a price source in the **[Symbol](symbol.htm)** column of the asset list.   

  * **Asset parameters.** All data besides spread and price must be manually entered in the asset list. Example asset list entry, with receiving price history from Bittrex:  
**ETH/BTC,0.01,0.0001,0,0, 0.000001,0.000001, 0,1,1,-0.25,*!*!BITTREX:BTC-ETH**  
  

  * **Order filling.** All orders are limit orders. If [OrderLimit](stop.htm) is not set, the current price is used. If an order is not filled within a certain time (to be defined with [SET_WAIT](brokercommand.htm)), it is cancelled and the limit can be adapted for the next try. **GTC** orders are supported (Zorro 2.33 or above). Positions can be read with the **GET_POSITION** command.  

  * **Compliance.** Coinigy requires the [NFA](mode.htm) flag. The same restrictions as for the selected exchange applies.   

  * **Trade and account parameters.** Trade profit is not available via API and estimated by Zorro from the trading costs entered in the asset list. Account requests return the BTC balance by default (the account currency can be set up in the Account column of the account list). The balance is reduced by opening a long position, and increased by closing the position. Equity is estimated by Zorro through summing up the open trades. **  
**
  * **Order issues.** Conigy is reported to occasionally return error codes although the order was in fact correctly sent to the selected exchange. This causes orphaned trades. The coninigy developers have been informed about this issue.  

  * **Rate limit.** API requests are currently capped at 2 requests per second. Make sure to set [MaxRequests](ticktime.htm) to less than 2, increase the [TickTime](ticktime.htm) accordingly, and use no bar periods of less than one minute.

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MTR4 bridge](mt4plugin.htm), [IB bridge](ib.htm), [Bittrex plugin](bittrex.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
