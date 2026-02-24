---
title: Saxo Bank
url: https://zorro-project.com/manual/en/saxo.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [brokerCommand](brokercommand.htm)
- [set, is](mode.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
---

# Saxo Bank

# Saxo Bank Plugin

Saxo Bank is a Danish broker that provides currencies, stocks, and ETFs. The Saxo Bank plugin was designed specifically to work with Saxo Bank's V2 Web API. You need [Zorro S](restrictions.htm) for using the plugin.

For using the Saxo plugin you need either a valid API token, or a Client ID and Secret. The steps:

  * On the Saxo Bank OpenAPI website, create a live or demo "application" for your trading sessions. 
  * In the form on the application website, enter **<http://127.0.0.1:31022/>** for the Redirect URL.
  * If you have a permanent **token** , enter it in the **User** field, and keep the **Password** field empty.
  * If you have no token, enter the **client ID** in the User field, and the **secret** in the password field.
  * Start the session. With client ID and secret, you will be redirected to a website where you enter your **user ID** and **password**. 
  * Zorro will then automatically request a token and start trading. 

**User** | Token or Client ID, normally a large hexadecimal string  
---|---  
**Password** | Empty or Secret, normally a large hexadecimal string  
  
Example [account list](account.htm#account) entries:

```c
Name,Server,AccountId,User,Pass,Assets,CCY,Real,NFA,Plugin
SaxoToken,Saxo,EUR,tokenabcdefg123456xyz,0,AssetsSaxo,USD,1,14,Saxo
SaxoDemo,Saxo,EUR,abcdef0123456789,00123456abcdef,AssetsSaxo,USD,0,14,Saxo
```

### Supported broker commands

The Saxo plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

  * **GET_DELAY**
  * **SET_DELAY**
  * **GET_WAIT**
  * **SET_WAIT**
  * **SET_PATCH**

### Remarks

  * The SAXO API is NFA compliant. The [NFA flag](mode.htm) must be set, no long and short positions must be open simultaneously, and positions must be closed in FIFO compliant way. [Virtual hedging](Hedge.htm) mode is recommended. 

### See also:

[Brokers](brokers.htm), [ broker plugin](brokerplugin.htm), [MT4 bridge](mt4plugin.htm), [Sierra bridge](sierra.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
