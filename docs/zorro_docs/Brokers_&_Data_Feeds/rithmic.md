---
title: Alpaca
url: https://zorro-project.com/manual/en/rithmic.htm
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

# Rithmic Plugin 

[Rithmic](https://www.rithmic.com/apis), LLC is a leading provider of direct market access (DMA) trade execution services. The Rithmic R API allows a trading algo to access real-time price, place orders and manage portofolios.

The Rithmic plugin was developed by Kun Zhao and licensed under the MIT License. It can be downloaded from the author's [GithHub page](https://github.com/kzhdev/rithmic_zorro_plugin/releases), Place the 32-bit or 64-bit **rithmic.dll** file into the **Plugin** or **Plugin64** folder under Zorro's root path, and place the **rithmic_ssl_cert_auth_params** file into Zorro's root path. Enter the Rithmic specific settings to **Zorro.ini** or **ZorroFix.ini** as described on Github. Generate an API key for your account on the Rithmic website.

Zorro login fields for Rithmic:

**User:** | Rithmic User ID  
---|---  
**Password:** | Rithmic Password  
  
### Remarks

  * The Symbol field in the asset list must have a Symbol column with a dot followed by the exchange symbol at the end.   
Example: **ESH5, 5993.75, 0.25, 0.0, 0.0, 0.25000, 0.01, -98.7, 0, 1.00, 0.0, *.CME**
  * Market and limit orders are supported.
  * The source code of the plugin can be downloaded from [https://github.com/kzhdev/Rithmic_zorro_plugin](https://github.com/kzhdev/rithmic_zorro_plugin)
  * This plugin was not developed by oP group. For support or reporting problems, please contact the author on his GitHub account or on the [Zorro user forum](http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=465410#Post465410).

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **GET_COMPLIANCE**
  * **GET_BROKERZONE**
  * **GET_MAXTICKS**
  * **GET_POSITION**(Example: **brokerCommand(GET_POSITION, "ESH5.CME");**)
  * **SET_ORDERTEXT**
  * **SET_SYMBOL**
  * **SET_ORDERTYPE**(0 - IOC, 1 - FOK, 2 - GTC)
  * **SET_PRICETYPE** (1- quotes, 2 - trades)
  * **SET_DIAGNOSTICS**
  * **DO_CANCEL**

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm),**[brokerCommand](brokercommand.htm) **

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
