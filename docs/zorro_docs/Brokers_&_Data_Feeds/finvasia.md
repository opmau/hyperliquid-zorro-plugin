---
title: Finvasia
url: https://zorro-project.com/manual/en/finvasia.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [set, is](mode.htm)
- [brokerCommand](brokercommand.htm)
- [Links & Books](links.htm)
- [order](order.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# Finvasia

# Finvasia Plugin 

Finvasia is an India-based broker. It supports many Indian Exchange products, including stocks, options, futures, and options on futures. It offers a REST API. Real mode and Demo mode is available, the latter for simulated trading via the Sandbox endpoint.

Login procedure: For the sandbox account, log in using PAN for username. For a real account, first log in using PAN, and when the message complains about a missing OTP, request an OTP via the Shoonya web portal. You will be emailed your OTP password. With Zorro still open, enter it into the User field, replacing the contents. Once you log out, you will need to request yet another OTP and manually enter it into the User field. Once a trading session has ended and you enter yet another, you will yet again need to request and enter another OTP into the User field.

Zorro login fields for Finvasia:

**User:** | [OTP or PAN]  
---|---  
**Password:** | [uid] [pwd] [vendor_code] [apikey] [imei] _(with a space in between each item)_  
  
[Accounts.csv](account.htm) example entry:  
**Name** | **Server** | **Account** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
Finvasia-Demo | 0 | 0 | 15678 | Test1234 Pwd VndCode key1234 cde4321 | AssetsFV | INR | 0 | 14 | Finvasia  
Finvasia-Real | 0 | 0 | 15678 | Real1234 Pwd VndCode key1234 cde4321 | AssetsFV | INR | 1 | 14 | Finvasia  
  
### Remarks

  * For asset symbols, either the Finvasia TradingSymbol itself can be used, or IB-style symbols for options("-OPT-"), futures("-FUT-"), and futures options("-FOP-"). Examples:  
"ZOMATO-EQ" is Zomato stock on NSE exchange.  
"ZOMATO" is Zomato stock on BSE exchange.  
"TCS29SEP22P4540" is the same as "TCS-OPT-20220929-4540-P", on the NFO exchange.  
"GOLDM26OCT22P49100" is the same as "GOLD-FOP-20221026-49100-P", on the MCX exchange.  
"COTTON31JAN23" is the same as "COTTON-FUT-20230131", on the MCX exchange.
  * Finvasia requires the NFA flag.
  * Finvasia does not supply historical data via API. Either use [PRELOAD](mode.htm) or wait for enough lookback to accumulate.
  * If combo orders fail, call **brokerCommand(5002,0)** (see below) to place leg orders individually.

### Supported broker commands

The plugin supports the [brokerCommand](brokercommand.htm) function with the following commands:

**SET_FUNCTIONS  
SET_DIAGNOSTICS** (0: off, 1: on)  
**SET_PRICETYPE**(NOTE: bid-ask data sometimes MISSING, prefer 2=last trade price)  
**GET_DELAY  
SET_DELAY  
GET_WAIT  
SET_WAIT  
SET_SYMBOL  
GET_COMPLIANCE  
SET_PATCH  
SET_ORDERTYPE **(0: day, 2: GTC, 16:EOS)  
**SET_COMBO_LEGS**(only works if account supports prctyp="2L" or "3L", see custom command 5002)  
**GET_POSITION  
GET_FUTURES  
GET_OPTIONS  
GET_FOP  
DO_CANCEL  
GET_UNDERLYING**

Supplemental commands:

**5001** Set Finvasia product for next order, such as "C", "M", "I", "H", or "B".  
**5002** **0** (default): allow combo orders. **1** : BANS combo orders in lieu of independent leg orders.

All data besides spread and price must be manually entered in the asset list. Example asset list entries:

```c
Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,MarginCost,Market,Multiplier,Commission,Symbol
ZOMATO_BSE,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,ZOMATO
ZOMATO_NSE,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,ZOMATO-EQ
RENUKA_BSE,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,RENUKA
RENUKA_NSE,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,RENUKA-EQ
TATASTEEL_BSE,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,TATASTEEL
TATASTEEL_NSE,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,TATASTEEL-EQ
RECLTD,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,RECLTD
TCS,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,TCS
MRF,100,0.05,0,0,0.05,0.05,0,UTC:0315-1000,1,0.01,MRF
GOLD,25000,1,0,0,1,1,0,UTC:0330-1825,1,0.01,GOLD
CRUDEOIL,2500,0.1,0,0,0.1,0.1,0,UTC:0330-1825,1,0.01,CRUDEOIL
```

### See also:

[Links](links.htm), [order](order.htm), [brokers](brokers.htm), [broker plugin](brokerplugin.htm),**[brokerCommand](brokercommand.htm) **

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
