---
title: Dukascopy Plugin
url: https://zorro-project.com/manual/en/dukascopy.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [IB / TWS Bridge](ib.htm)
- [FXCM](fxcm.htm)
---

# Dukascopy Plugin

# Dukascopy Plugin 

Dukascopy is a Swiss bank and ECN broker with a Java-based API. The Dukascopy plugin was developed by Jürgen Reiss and supports Forex trading on demo and real accounts.

The Dukascopy plugin is not included in the Zorro distribution. Get the plugin from Github and also install the Java Development Kit:

  * Download and install the plugin - **dzjforex-0.9.6.zip** or newer - from <https://github.com/juxeii/dztools>. The DLL and the **dukascopy** folder both go into your **Plugin** directory.  

  * Download and install the latest 32-bit Java JDK from [Oracle](http://www.oracle.com/technetwork/java/javase/downloads). Make sure that it is the 32-bit version (**x86** suffix) since the plugin DLL is a 32-bit library, and that the folder path contains no spaces. In case you already have a recent 32-bit JDK installation you might skip this step.  

  * Add the **\jre\bin** and the **\jre\bin\client** folders to the front of your **Path** environment variable ([here](http://www.computerhope.com/issues/ch000549.htm) is a howto). Your **Path** environment variable should now begin like this:  
**C:\Java\jdk1.8.0_121\jre\bin;  
C:\Java\jdk1.8.0_121\jre\bin\client;  
**... further paths ...  

  * Start Zorro. Make sure that you now can select **Dukascopy** from the [Account] scrollbox. 

**User** | Dukascopy user ID  
---|---  
**Password** | Dukascopy Password  
  
### Remarks

  * Currently only Forex and CFDs are supported (no stocks, binary options, etc.).
  * The history downloading from Dukascopy servers is sometimes not reliable; just try again in case of errors.
  * The source code of the Dukascopy plugin can be downloaded from <https://github.com/juxeii/dztools>. 
  * This plugin was not developed by oP group. For support or reporting problems, please contact the author on his GitHub account or on the [Zorro user forum](http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=447697&#Post447697). 
  * Another Dukascopy plugin was recently developed by Metakod on the [ Zorro user forum](https://opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=473439&page=1). You can get it here: [ http://metakod.com/mk/tools/12](http://metakod.com/mk/tools/12).

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **GET_MAXREQUESTS**
  * **GET_PRICETYPE**
  * **GET_LOCK**
  * **SET_PATCH**
  * **SET_ORDERTEXT**
  * **GET_DIGITS**
  * **GET_MAXLOT**
  * **GET_MINLOT**
  * **GET_MARGININIT**
  * **GET_TRADEALLOWED**
  * **GET_TIME**
  * **GET_MAXTICKS**
  * **GET_SERVERSTATE**
  * **GET_ACCOUNT**
  * **SET_HWND**
  * **SET_SLIPPAGE**
  * **SET_LIMIT**

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MTR4 bridge](mt4plugin.htm), [IB bridge](ib.htm), [FXCM plugin](fxcm.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
