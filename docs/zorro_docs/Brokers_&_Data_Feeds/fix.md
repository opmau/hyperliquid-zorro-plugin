---
title: Alpaca
url: https://zorro-project.com/manual/en/fix.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [FXCM](fxcm.htm)
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# Alpaca

# FIX Plugin 

FIX is an universal API shared by many brokers and trading services. Plain FIX supports prices and trading, but no account information and no historical market data. The Zorro FIX Plugin connects to brokers and simulators via the FIX 4.4 API. Currently supported are:

  * A FIX plugin for [FXCM](fxcm.htm) that pulls historical market data with FXCM ForexConnect SDK.
  * A FIX Simulator Plugin that connects to a market data simulator and matching engine.

The Zorro FIX project was developed by DSEC Capital and is available on the author's [Github page](https://github.com/dsec-capital/zorro-fix). It uses the **QuickFix** open source library. QuickFix is not as fast as commercial FIX implementations, but provides a straightforward API and application framework to develop FIX based server and client applications. Some of the core building blocks and the FIX simulation sserver are inspired from their examples.

Download the plugin from Github, compile it as described there, and put it into the **Plugin** or **Plugin64** folder, dependent on whether it's 32 or 64 bit. Start Zorro as usual and check if a plugin with the name **_FixPlugin** appears in the broker list. The FIX session configuration for the plugin is specified in **zorro_fix_client.cfg**. 

### Remarks

  * The plugin is currently work in progress. 
  * The source code of the plugin can be downloaded from [Github. ](https://github.com/dsec-capital/zorro-fix)
  * This plugin was not developed by oP group. For support or reporting problems, please contact the author on his GitHub account.

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **GET_COMPLIANCE**
  * **GET_POSITION**
  * **SET_ORDERTYPE**(1 - ICO, 2 - GTC, 3 - FOK)
  * **SET_PRICETYPE** (1- quotes, 2 - trades)

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm),**[brokerCommand](brokercommand.htm) **

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
