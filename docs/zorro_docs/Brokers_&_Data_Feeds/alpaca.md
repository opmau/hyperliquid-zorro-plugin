---
title: Alpaca
url: https://zorro-project.com/manual/en/alpaca.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# Alpaca

# Alpaca Plugin 

[Alpaca](http://alpaca.markets/) is a commision-free brokerage for algorithmic trading. The Alpaca API allows a trading algo to access real-time price, place orders and manage portofolios through a REST or a streaming interface.

The Alpaca plugin was developed by Kun Zhao and uses V2 websocket for live data, and Alpaca V1 or alternatively Polygon.io data for historical data. It can be downloaded from the author's [GithHub page](https://github.com/kzhdev/alpaca_zorro_plugin/releases), Place the **Alpaca.dll** file into the **Plugin** folder under Zorro's root path, and enter the Alpaca specific settings to Zorro.ini or ZorroFix.ini as described on Github. Generate an API key for your account on the Alpaca website.

Zorro login fields for Alpaca:

**User:** | Alpaca API key  
---|---  
**Password:** | Alpaca Secret  
  
### Remarks

  * History is supported in M1, M5, M15, and D1 resolution.
  * Market and limit orders are supported.
  * The source code of the plugin can be downloaded from [ https://github.com/kzhdev/alpaca_zorro_plugin](https://github.com/kzhdev/alpaca_zorro_plugin)
  * This plugin was not developed by oP group. For support or reporting problems, please contact the author on his GitHub account or on the [Zorro user forum](http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=465410#Post465410).

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **GET_COMPLIANCE**
  * **GET_MAXTICKS**
  * **GET_MAXREQUESTS**
  * **GET_LOCK**
  * **GET_POSITION**
  * **SET_ORDERTEXT**
  * **SET_SYMBOL**
  * **SET_ORDERTYPE**(1 - ICO, 2 - GTC, 3 - FOK (default), 4 - DAY, 5 - OPG, 6 - CLS)
  * **SET_PRICETYPE** (1- quotes, 2 - trades)
  * **SET_DIAGNOSTICS**

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm),**[brokerCommand](brokercommand.htm) **

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
