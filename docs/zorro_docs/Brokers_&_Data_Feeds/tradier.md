---
title: Tradier Plugin
url: https://zorro-project.com/manual/en/tradier.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [set, is](mode.htm)
- [Virtual Hedging](hedge.htm)
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [IB / TWS Bridge](ib.htm)
- [FXCM](fxcm.htm)
---

# Tradier Plugin

# Tradier Plugin 

Tradier is a US based brokerage for stocks, ETFs, and options with a REST based API for trading and market data. The Tradier plugin allows trading on real accounts and receiving data. For using it you need a Tradier Developer account. There are two types of Developer accounts: Sandbox (demo mode) and Brokerage (real mode). For real mode a Tradier Brokerage account is also required. 

A free sandbox account can be opened at [hhttps://developer.tradier.com](https://developer.tradier.com) for receiving delayed market data. Paper trading and account data for sandbox accounts is not yet available, but shall come soon.

The steps to get a brokerage developer account:

  * Open a Tradier Brokerage account at <https://brokerage.tradier.com> (login with username).
  * Open a Tradier Developer account at <https://developer.tradier.com> (login with email).
  * Email Tradier customer support for a Brokerage Developer access token. Refer clearly to both accounts.
  * Support will email a link to the access token. The link will disappear after the first click. Copy the access token and keep it safe.
  * Log in to the *brokerage* dashboard. Your account ID is on the top left of the screen.

Zorro login for Tradier:

**UsUser** | Brokerage account ID, or empty in Sandbox mode.  
---|---  
**Password** | Brokerage or Sandbox access token.  
  
### Remarks

  * Tradier does not support trades, only orders and positions. It uses UUIDs as order identifiers. The [NFA](mode.htm) flag is required. [Hedge](hedge.htm) must be either at 0, 4, or 5, and several Zorros cannot trade the same assets on the same account because long and short positions can not be opened simultaneously. Partial closing is supported by the API.
  * For resuming old trades when starting a new session, read the positions with the [GET_POSITON](brokercommand.htm) command and convert them to trades by script.
  * Tradier supports pending orders. Pending orders to open a position can be traced with BrokerTrade, pending orders to close a position are cancelled after the number of milliseconds given by [ SET_WAIT](brokercommand.htm) if not filled.
  * Tradier is limited in the number of simultaneous connections and in the price retrieval rate. For reducing the number of requests per second, increase **TickTime** and/or reduce **MaxRequests**.
  * Tradier only provides a limited amount of intraday historical data (10 days for M1). Systems with a long lookback period need a different data source and the [PRELOAD](mode.htm) flag.

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **SET_PATCH**
  * **SET_WAIT**
  * **GET_COMPLIANCE**
  * **GET_MAXTICKS**
  * **GET_MAXREQUESTS**
  * **GET_POSITION**
  * **GET_OPTIONS**
  * **GET_UNDERLYING**
  * **SET_SYMBOL**
  * **SET_MULTIPLIER**
  * **SET_COMBO_LEGS**
  * **SET_ORDERTYPE** (1 = GTC, 4 = DAY)
  * **GET_UUID**
  * **SET_UUID**

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm), [MTR4 bridge](mt4plugin.htm), [IB bridge](ib.htm), [FXCM plugin](fxcm.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
