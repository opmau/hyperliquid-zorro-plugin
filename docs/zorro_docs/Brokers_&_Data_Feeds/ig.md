---
title: IG Plugin
url: https://zorro-project.com/manual/en/ig.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [TickTime, MaxRequests, ...](ticktime.htm)
- [set, is](mode.htm)
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [IB / TWS Bridge](ib.htm)
- [FXCM](fxcm.htm)
---

# IG Plugin

# IG Plugin 

IG is a Forex and CFD broker with a Java-based API. The IG plugin was developed by Daniel Lindberg and supports trading on demo and real accounts. However, you need to open a real account even for trading on an IG demo account through their API. 

For using the IG plugin you need to download it from the author's GitHub page, and also install the Java Development Kit:

  * Download the IG plugin from <https://github.com/dan-lind/igzplugin>. Unzip the folder **"ig"** and the **"IG.dll"** in the Zorro **Plugin** directory. 
  * Download and install the 32-bit Java JDK from [Oracle](http://www.oracle.com/technetwork/java/javase/downloads). Make sure it is the 32-bit version (**x86** suffix) since the plugin DLL is a 32-bit library, and that the folder path contains no spaces. In case you already have a recent 32-bit JDK installation you might skip this step. According to user reports, JDK version 8 is required for the IG plugin.  

  * Add the **\jre\bin** and the **\jre\bin\client** folders to the front of your **Path** environment variable ([here](http://www.computerhope.com/issues/ch000549.htm) is a howto). Your **Path** environment variable should now begin like this:  
**C:\Java\jdk1.8.0_121\jre\bin;  
C:\Java\jdk1.8.0_121\jre\bin\client;  
**... further paths ...  

  * You need an API key for using the IG API. Refer to [IG Labs Getting Started guide](https://labs.ig.com/gettingstarted) for step by step instructions on how to get the key (you need to have a real account with IG). Then navigate to the **Plugin\ig** folder and open **application.properties** with a text editor. Here you should adapt the **plugin.realApiKey** and/or **plugin.demoApiKey** to the keys that you generated in the previous step. You can leave the other entries to their default values.
  * Start Zorro. You should now be able to select **IG** from the [Account] scrollbox. 

Zorro login fields for IG:

**User:** | IG Account ID  
---|---  
**Password:** | IG Password  
  
### Remarks

  * You need a dedicated asset list for IG, since the asset symbols are very different to the asset names. For instance, the EUR/USD symbol is CS.D.EURUSD.CFD.IP You can find the correct symbol names with the [IG API Companion](https://labs.ig.com/sample-apps/api-companion/index.html). Log in with you user name, password and api key, then scroll down to Market Search, enter your search String, e.g. DAX. Now look in the response for the "epic" key, e.g. "epic": "IX.D.DAX.IFD.IP",
  * The price downloading from IG servers is limited to 30 quotes per minute, and 10 000 quotes per week. If your allowance goes to zero, you won't be able to download more quotes for the lookback period. IG cannot be used as a source for downloading long periods of historic data, and also has problems with portfolio strategies with long lookback periods. You also might have to increase the [TickTime](ticktime.htm) for reducing the quote frequency, and use the [PRELOAD](mode.htm) flag dependent on your strategy. 
  * The default quotas for IG Streaming API connections is 40 concurrent connections. The plugin currently uses 2 streams per asset, which means so can at a maximum trade 20 assets at the same time.
  * The source code of the IG plugin can be downloaded from <https://github.com/dan-lind/igzplugin>. 
  * This plugin was not developed by oP group. For support or reporting problems, please contact the author on his GitHub account or on the [Zorro user forum](http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=465410#Post465410).

### Supported broker commands

The IG plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **brokerCommand(SET_PATCH, patch)**
  * **brokerCommand(SET_ORDERTEXT, text)**  

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MTR4 bridge](mt4plugin.htm), [IB bridge](ib.htm), [FXCM plugin](fxcm.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
