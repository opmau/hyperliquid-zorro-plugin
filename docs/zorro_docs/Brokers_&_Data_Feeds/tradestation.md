---
title: TradeStation
url: https://zorro-project.com/manual/en/tradestation.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [Asset and Account Lists](account.htm)
- [brokerCommand](brokercommand.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
---

# TradeStation

# TradeStation Plugin

TradeStation is a Florida-based broker that supports many instruments, including stocks, options, futures, and futures options. The TradeStation plugin was designed specifically to work with TradeStation's v3 Web API. You need [Zorro S](restrictions.htm) for using the plugin.

The TradeStation bridge was developed by Andrew Dolder. For using the bridge you need an API Key and API Secret. The steps:

  * Email TradeStation support and specifically request v3 keys.
  * Enter key and secret in the User and Password fields. 
  * Log in with Zorro. You will be required to follow some high-security multi-factor authentication protocols.
  * Once that's completed, Zorro will print out the API Refresh Token in the log and then quit. **Save the API Refresh Token**.
  * Copy and paste the API Refresh Token into the password field with a space after the secret, as shown below. Save this permanently into your [Accounts.csv](account.htm) file, close the CSV file, and restart Zorro.
  * Henceforth, you will no longer need to follow authentication protocols, unless you change your API keys.

**User** | API Key  
---|---  
**Password** | API_Secret(space)Refresh_Token  
  
For the first time authentication described above, the refresh token is not needed.

Example account list entries:

```c
Name,Server,AccountId,User,Pass,Assets,CCY,Real,NFA,Plugin
TS-firsttime,TradeStation,0,abcd,efgh,AssetsTS,USD,1,14,TradeStation.dll
TS-real-margin,TradeStation,1234M,abcd,efgh ijkl,AssetsTS,USD,1,14,TradeStation.dll
TS-real-futures,TradeStation,1234F,abcd,efgh ijkl,AssetsTS,USD,1,14,TradeStation.dll
TS-demo-margin,TradeStation,2345M,abcd,efgh ijkl,AssetsTS,USD,0,14,TradeStation.dll
TS-demo-futures,TradeStation,2345F,abcd,efgh ijkl,AssetsTS,USD,0,14,TradeStation.dll
```

Example asset list entries:

```c
Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,MarginCost,Market,Multiplier_LotAmount,Commission,Symbol
ESZ21,2576,0.25,0,0,0.25,0.25,160,0.04,-50,1,*
ES-CONT,2576,0.25,0,0,0.25,0.25,160,0.04,-50,1,@ES
AAPL,118.44,0.01,0,0,0.01,0.01,0,2,-100,0.01,*
AMZN,118.44,0.01,0,0,0.01,0.01,0,2,-100,0.01,*
AXP,74.35,0.03,0,0,0.01,0.01,0,2,-100,0.01,*
BA,146.78,0.05,0,0,0.01,0.01,0,2,-100,0.01,*
SPX,4947.96,0.02,0,0,0.01,0.01,0,2,-100,0.01,$SPX.X
SPY,215,0.02,0,0,0.01,0.01,0,2,-100,0.01,*
```

### Remarks

  * The returned volume is the session volume.

### Supported broker commands

The bridge supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_ORDERTYPE (0:DAY, 1: FOK, 2: GTC, 16:IOC)**
  * **SET_PRICETYPE (1: Bid/Ask, 2: Last)**
  * **GET_UUID**
  * **SET_UUID**
  * **DO_CANCEL**
  * **GET_DELAY**
  * **SET_DELAY**
  * **GET_WAIT**
  * **SET_WAIT**
  * **SET_SYMBOL**
  * **GET_COMPLIANCE**
  * **SET_PATCH**
  * **GET_MAXTICKS**
  * **GET_POSITION**
  * **GET_FUTURES**
  * **GET_OPTIONS**
  * **GET_FOP**
  * **GET_UNDERLYING**
  * **SET_COMBO_LEGS**

Custom broker commands:

**2000**. Input: var, changes **SET_UUID** order's limit price.  
**2001**. Changes **SET_UUID** order to a market order..  
**2002**.Cancels all orders associated with symbol by **SET_SYMBOL**.  
**2003**. Indiscriminately cancels all open orders.  
**2004**. For **SET_UUID** order, returns 1 if order is still open, 0 if closed.  
**4000**. Output: char array of length NAMESIZE2. For **SET_UUID** order, copies TradeStation SymbolID to output.  

### See also:

[Brokers](brokers.htm), [ broker plugin](brokerplugin.htm), [MT4 bridge](mt4plugin.htm), [Sierra bridge](sierra.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
