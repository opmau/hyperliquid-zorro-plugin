---
title: TD Ameritrade
url: https://zorro-project.com/manual/en/tdamtrade.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset Symbols](symbol.htm)
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# TD Ameritrade

# TD Ameritrade / Schwab Plugin 

[TD Ameritrade](http://alpaca.markets/), now acquired by **Schwab** , is a commision-free brokerage for stocks, ETFs, and options. The TDAmTrade plugin allows the Zorro trading engine to communicate with TD Ameritrade through the TD Ameritrade REST API. 

The TDAmTrade plugin was developed by Clyde W. Ford and can be downloaded from the author's GitHub page [ https://github.com/cwford/TDAmTrade_Zorro_Plugin](https://github.com/cwford/TDAmTrade_Zorro_Plugin), It is free for non-commercial use. The license conditions and step-by-step installation instructions can also be found on GitHub. 

Zorro login fields for TD Ameritrade:

**User:** | Consumer key  
---|---  
**Password:** | (empty)  
  
### Remarks

  * You can trade stocks, ETFs, and options, but no futures, future options, or currency pairs (Forex). The TD Ameritrade API does not allow trading these instruments at the present time.
  * TD Ameritrade does not have a true Demo mode. The API does not support "paper trading". In demo mode the plugin will authenticate the user and run through a number of self-diagnostic tests.
  * The plugin uses asset designators as described on the [Symbols](symbol.htm) page.
  * The plugin was written in C#. The code base is well-documented and free for non-commercial use to modify and adapt. The source code can be downloaded from the Github page. 
  * This plugin was not developed by oP group and is not subject to Zorro support. For questions or reporting problems, please contact the author on his GitHub account or on the [Zorro user forum](http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=465410#Post465410).

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **GET_COMPLIANCE**
  * **GET_POSITION**
  * **GET_OPTIONS**
  * **SET_SYMBOL**
  * **SET_COMBO_LEGS**

Supplemental commands:

  * **SHOW_RESOURCE_STRING** (Show a globalized resource string from the plug-in)
  * **REVIEW_LICENSE** (Display the plug-in license and force re-acceptance)
  * **GET_ASSET_LIST** (Retrieve a list of subscribed assets)
  * **GET_TEST_ASSETS** (Get the test assets included in the Settings file. See Setting file for more.)
  * **SET_VERBOSITY** (Set the diagnostic verbosity level. See Verbosity level for more.)
  * **SET_TESTMODE** (Set whether the plug-in is in testing mode.)
  * **SET_SELL_SHORT** (Set what to do if selling more shares of an asset than owned)
  * **SET_LANGUAGE** (Set the language used by the plug-in. See the section on globalization for more.)

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm),**[brokerCommand](brokercommand.htm) **

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
