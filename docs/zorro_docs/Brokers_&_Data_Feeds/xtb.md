---
title: XTB
url: https://zorro-project.com/manual/en/xtb.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [brokerCommand](brokercommand.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# XTB

# XTB Plugin 

XTB is a low-commission broker offering a wide range of tradable instruments. For opening a demo account, go to: [ https://www.xtb.com/en/demo-account](https://www.xtb.com/en/demo-account). XTB offers free access to their API. The connection is made through TCP sockets. Link to API their documentation: [ http://developers.xstore.pro/documentation/#introduction](http://developers.xstore.pro/documentation/#introduction).  
  
This plugin allows to connect to their API from Zorro. It was written by Olivier Mallet in Win32 C++ under Visual Studio 2017. It can be downloaded from [ https://github.com/olmallet81/XTBZorroPlugin](https://github.com/olmallet81/XTBZorroPlugin). The project is MIT-licensed; see the contained LICENSE.md file for more details. To install the plugin, simply place the binary file **XTB.dll** in the **Plugin** folder where Zorro is installed. 

Zorro login fields for XTB:

**User:** | XTB User  
---|---  
**Password:** | XTB Passwordt  
  
### Remarks

  * History is not supported, since XTB does not provide any historical data download service.
  * Market and limit orders are supported. The plugin caps limit/stop orders to 50% of the opened trade current position, otherwise the API does not close a trade with such an order attached.
  * A log file is automatically generated at each connection to XTB and dumped into the folder **C:\Zorro\Log\XTB** by default. To adjust this path to your own Zorro folder path, please modify the line 63 in the header file **XTB.h**.
  * The source code of the plugin is available on the author's GitHub page.
  * This plugin was not developed by oP group. For support or reporting problems, please contact the author on his GitHub account or on the [Zorro user forum](http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=465410#Post465410).

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **GET_TIME**
  * **GET_DIGITS**
  * **GET_MINLOT**
  * **GET_LOTSTEP**
  * **GET_MAXLOT**
  * **GET_SERVERSTATE**
  * **GET_BROKERZONE**
  * **GET_DELAY**
  * **GET_NTRADES**
  * **GET_POSITION**
  * **GET_AVGENTRY**
  * **GET_TRADES**
  * **GET_TRADEPOSITION** (new command added for getting the current position on a trade by its id)
  * **GET_WAIT**
  * **SET_DELAY**
  * **SET_PATCH**(returns 16 as rollover and commission computation are not implemented yet)
  * **SET_WAIT**
  * **SET_LASTCONNECTION**(new command added for setting the last connection date for getting all closed trades since the last connection)

### See also:

[Brokers](brokers.htm), [broker plugin](brokerplugin.htm),**[brokerCommand](brokercommand.htm) **

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
