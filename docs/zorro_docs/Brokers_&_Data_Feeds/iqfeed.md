---
title: DTN IQFeed for Zorro
url: https://zorro-project.com/manual/en/iqfeed.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [Asset and Account Lists](account.htm)
- [Asset Symbols](symbol.htm)
- [brokerCommand](brokercommand.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [IB / TWS Bridge](ib.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
---

# DTN IQFeed for Zorro

# DTN IQFeed

[DTN IQFeed](http://www.iqfeed.net)™ is a tick-by-tick unfiltered datafeed for almost all available symbols, with direct connection to the exchanges. The IQFeed plugin ([Zorro S](restrictions.htm) required) connects to the IQFeed client application and can retrieve live and historical data. Please note: for using the IQFeed plugin you need to activate it by email to [Support](mailto:support@opgroup.de). The annual activation fee is EUR 65. 

Installing IQFeed:

  * Open an account and subscribe the desired markets on [ http://www.iqfeed.net](http://www.iqfeed.net/). You will get a login ID - normally a 6 digit number \- and a password. 
  * Download the IQFeed Client from the [ DTN Download Page](http://www.iqfeed.net/index.cfm?displayaction=support&section=download) and install it into its default folder. Start one of the included apps to make sure that it works.
  * On Zorro S, select IQFeed and enter your login ID and password.  

**User** | IQFeed login ID  
---|---  
**Password** | IQFeed password  
  
[Accounts.csv](account.htm) example entry

**Name** | **Broker** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
IQFeed | IQFeed | 0 | 123456 | asdfghjk | MyAssetsIQF | 0 | 0 | 0 | IQFeed.dll  
  
### Remarks

  * For looking up an asset symbol, enter the root name on the [ IQfeed Symbol Guide](https://iqfeed.net/symbolguide/index.cfm?symbolguide=lookup&displayaction=support&section=guide&web=iqfeed) web page.
  * For getting prices from IQFeed and trading with a broker, use an [Account list](account.htm) with a composed [symbol](symbol.htm) that sets up IQFeed for live and historical data, and the broker for trading.

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **GET_MAXTICKS**
  * **SET_SYMBOL**
  * **GET_OPTIONS**
  * **GET_FUTURES**
  * **GET_FOP**
  * **GET_BOOK**

### See also:

[Brokers](brokers.htm), [ broker plugin](brokerplugin.htm), [IB plugin](ib.htm), [MT4 bridge](mt4plugin.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
