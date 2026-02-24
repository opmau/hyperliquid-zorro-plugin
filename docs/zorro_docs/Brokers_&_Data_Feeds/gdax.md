---
title: Coinbase Plugin
url: https://zorro-project.com/manual/en/gdax.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [Binance](binance.htm)
---

# Coinbase Plugin

# Coinbase Pro Plugin (deprecated)

**[Coinbase](https://pro.coinbase.com)** alias **GDAX** is an insurance backed cryptocurrency exchange. There are two Zorro plugins that use the Coinbase Pro API, one developed by a Zorro user and available on [ Github](https://github.com/kzhdev/gdax_zorro_plugin), one included in the Zorro S distribution (Zorro 2.41 or above).

For using the plugin, first generate an API Key on the Coinbase Pro website. In Zorro, select **CoinbasePro** from the scrollbox, enter the API key and the passphrase (with a space in between) in the **User ID** input box, and the secret in the **Password** input box.

**User** | Key Passphrase  
---|---  
**Password** | Secret  
  
[Accounts.csv](account.htm) example entry:

**Name** | **Server** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
Coinbase | Coinbase | USD |  ap2412057834 xf9qp |  akh3dfE3webnF++ | AssetsCB | USD | 1 | 14 | CoinbasePro.dll  
  
Please note: Coinbase no longer supports their **CoinbasePro** API, so the plugins are deprecated.

### Supported broker commands

The Coinbase Pro plugin supports the [brokerCommand](brokercommand.htm) function with the following standard commands:

  * **GET_MAXTICKS**
  * **GET_MAXREQUESTS**
  * **GET_LOCK**
  * **SET_SYMBOL**
  * **SET_AMOUNT**
  * **SET_DIAGNOSTICS**
  * **GET_COMPLIANCE**
  * **SET_ORDERTYPE** (1=FOK, 2=GTC, 3=IOC)
  * **SET_PRICETYPE** (1=Quotes, 2=Trades)
  * **GET_POSITION** (balance of the given coin symbol)
  * **GET_UUID**
  * **SET_UUID**

Some additional commands have been implemented:

  * **2000** (**CBP_ENABLE_POSTONLY_ORDERFLAG**) - Sets "post_only" flag for orders. 1: enabled (default), 0: disabled.
  * **2001** (**CBP_GENERATE_ASSETLIST_TEMPLATE**) \- Generates a template asset list in "Log\AssetsCoinbaseProTemplate.csv"..   
Arguments: Comma-separated list of Coinbase Pro Symbols, e.g. "BTC-USD,ETH-USD", or 0 for all assets  
Note: LotAmount can be reconfigured to a larger value because SET_AMOUNT is supported.  
Note: Generated PipCost field assumes quoted currency is account currency. User is advised to reconfigure based on account currency and LotAmount.  

### Remarks

  * **Symbols** are of the type **"AAA-BBB"** , where **AAA** is the coin and **BBB** the counter currency. Example asset list entries:  

```c
Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,MarginCost,Market,LotAmount,Commission,Symbol
BTC/USD,30784.41000000,0.01000000,0.0,0.0,1.000000e-02,1.000000e-10,-100,0,1.000000e-08,0.000,BTC-USD
ETH/USD,1827.64000000,0.01000000,0.0,0.0,1.000000e-02,1.000000e-10,-100,0,1.000000e-08,0.000,ETH-USD
```

  * **Asset parameters.** All data besides spread and price must be manually entered in the asset list. PIP size and LotAmount can be set arbitrarily, but Coinbase Pro has minimum lot sizes for some assets. Lots sizes can be taken from the Coinbase Pro website and converted to the corresponding lot amounts.
  * Market and limit orders are supported. Trades use UUID identifiers; the trade ID is the hash of the UUID.

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [Binance](binance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
