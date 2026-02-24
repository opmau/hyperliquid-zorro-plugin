---
title: FXCM Plugin
url: https://zorro-project.com/manual/en/fxcm.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [price, ...](price.htm)
- [brokerCommand](brokercommand.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Error Messages](errors.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [IB / TWS Bridge](ib.htm)
---

# FXCM Plugin

# FXCM Plugin 

The included FXCM plugin allows direct trading with FXCM Markets Ltd or FXCM Australia Pty. Ltd on demo and real accounts, without the need to use the MT4 platform. Aside from forex, FXCM offers index, commodity, and cryptocurrency CFDs, free tick-based, good-quality historical price data, a free API, and no minimum monthly investments. 

For opening a demo or real account, visit [ FXCM.](https://www.fxcm.com) The demo account will normally expire after a month of no trading, but can be renewed indefinitely by opening a new demo account. 

The FXCM plugin is available in a 32 bit and a 64 bit version. For using it, you must unzip a bunch of DLLs. The 32 bit DLLs are contained in the **FXCM.zip** archive in the Zorro installation, the 64 bit DLLs in the Zorro64 subfolder. Unpack them directly into the Zorro main folder resp in the Zorro64 folder. The DLLs beginning with **"api-ms-win-core"** are only needed for very old Windows versions such as Vista or Server 2012. Windows XP is not supported by the FXCM API. When you successfully unpacked the DLLs, the FXCM plugin will appear in the Broker scrollbox.

Zorro login fields for FXCM:

**User** | FXCM Account ID  
---|---  
**Password** | FXCM Password  
  
[Asset list](account.htm) example: **AssetsFXCM.csv**

[Accounts.csv](account.htm) example entries:

**Name** | **Broker** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
FXCM Demo | FXCM | 0 |  U4567890 | 1234 | AssetsFXCM | EUR | 0 | 0 | FXCM  
FXCM Real | FXCM | 0 |  0 | 0 | AssetsFXCM | EUR | 1 | 0 | FXCM  
  
### FXCM symbols

The default **AssetsFix.csv** uses different CFD symbols. Use **AssetsFXCM.csv** for trading CFDs with FXCM.

### Additional data 

The FXCM plugin supports the following data streams: 

  * [marketVal](price.htm): Spread in historical data.
  * [marketVol](price.htm): Tick frequency in historical data.

### Supported broker commands

The FXCM plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **SET_WAIT**
  * **SET_DELAY**  

### Known FXCM API issues

You can trade with FXCM either through the [MT4 bridge](mt4plugin.htm), or with a direct API connection through the FXCM plugin. Direct API connection is preferable due to higher speed and lower slippage. The FXCM plugin uses the latest API version 1.62. 

  * **No XP** \- the FXCM API requires Windows 7 or above. 
  * **Single account** \- for different FXCM accounts also use different user names for logging in.
  * **Single session** \- some FXCM accounts are pre-set to connect only to a single session. For trading with several Zorros on the same account, contact FXCM and let them switch your account to multiple sessions.
  * **Orphaned trades** \- if the FXCM server does not respond to a trade command in time, but opens the trade nevertheless, the trade gets orphaned. This is a very rare event, but it can happen. Compare the open trades with the trade status page from time to time, and check the log for [Warning 070](errors.htm). Orphaned trades are not under Zorro control and must be closed manually.

### Error messages

Here's a list of explanations from FXCM for some of their error messages:

**ORA-20103 - Session expired:** Your connection has been lost. This error message could be displayed due to a number of reasons, including network instability, a system issue or a program crash. If the problem is a system issue, please try to reboot.  
  
**ORA-20143 - Order price too far from market price:** This error message is generated when the Buy Limit price is above the Bid price. 

**ORA-20112 - Limit price did not pass validation:** This error message is generated when the Limit price does not correspond to the ask price for the order type required. Ff the Time in Force is IOC or FOK then the Buy limit price should be >= Ask price. 

**ORA-20113 - Insufficient margin in session:** This error message is generated when you don’t have enough margin.

**ORA-20102 - Access Violation:** This error message is generated when a trade account is missing from the dealer account.

**ORA-20105 - Order price did not pass validation:** The rejected orders error message is generated when the stop price is too close the the ask price. For example, if the Ask price was 9911 and your Stop price 99=9917, you would receive this error message.

**ORA-20008 - Failed to create order, primary validation:** This error message is generated when Range prices are below the Ask price. For example if orders were placed on news events, and the spreads got wider. 

**How can I tell what account type I have?** Checking on Trading station: To check the type of account you have, you can login to the Trading Station and look in the tab “Accounts”. Scroll to the end and find column Type. Y = Hedging is allowed; N = Hedging is not allowed, O = Netting only, D = Day netting, F = FIFO.

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MT4 bridge](mt4plugin.htm), [IB plugin](ib.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
