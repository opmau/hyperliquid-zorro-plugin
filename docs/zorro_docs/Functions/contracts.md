---
title: Contract Variables
url: https://zorro-project.com/manual/en/contracts.htm
category: Functions
subcategory: None
related_pages:
- [contract, ...](contract.htm)
- [Dataset handling](data.htm)
- [asset, assetList, ...](asset.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [trade management](trade.htm)
- [enterLong, enterShort](buylong.htm)
- [combo, ...](combo.htm)
---

# Contract Variables

# Variables for futures and options

The following variables are valid after loading a [ contract chain](contract.htm) and selecting a contract. 

## ContractType

## ContractExpiry

Type and expiration date (**YYYYMMDD** format) of the selected contract, or **0** when no valid contract was selected.   

### Type:

**int** , read/only 

## ContractAsk

## ContractBid

## ContractUnl

## ContractStrike

## ContractVol

## ContractVal

Current ask, bid, underlying, and strike price, and trade volume or open interest of the selected contract. **0** when no contract chain was loaded, or no valid contract was selected, or when the parameter is not or not yet available in the selected contract. In historical data, **ContractAsk** or **ContractBid** can be 0 with illiquid or worthless contracts; such contracts should not be traded. **ContractVal** includes the multiplier retrieved from the broker API in [Trade] mode. For retrieving the ask and bid prices, the underlying, and the volume in [Trade] mode, [contractPrice](contract.htm) must be called before.   

### Type:

**var** , read/only 

## ThisContract

The currently selected contract by the last [contract](contract.htm) call. In [Trade] mode, **(string)ThisContract** \- the first element of the **CONTRACT** struct - contains the trading class retrieved from the broker API. 

## Contracts

The current option or future chain loaded by the last [contractUpdate](contract.htm) call. A list of **CONTRACT** structs of the size **NumContracts** , sorted first by ascending expiration, second by ascending strike. Can be used for enumerating all contracts. 

### Type:

**CONTRACT***

## NumContracts

The number of contracts in the chain, up to 20000, or **0** when no chain was loaded.. 

## ContractRow

The row number of the selected contract in the historical **.t8** [dataset](data.htm) that holds the contract chain history of the current asset in Test and Train mode; the row number of the selected contract in the options chain (**Contracts** pointer) in Trade mode. Can be used to retrieve further data belonging to the contract from additional datasets. 

## ContractHandle

The handle of the historical **.t8** [dataset](data.htm) that holds the contract chain history of the current asset in Test and Train mode. Can be set to a specific dataset before calling [contractUpdate](contract.htm); otherwise it is automatically set. 

### Type:

**int** , asset specific. 

The following variables are used to set up asset specific contract parameters: 

## Multiplier

Number of underlying units per option / future contract, for calculating the trade volume, selecting a contract, and filtering the options chain (default **0** = download all options). Asset specific; set it after selecting the [asset](asset.htm) and before buying contracts. If at **0** , [contractUpdate](contract.htm) will in [Trade] mode automatically set this variable to the multiplier of the first contract if available via broker API. 

## Centage

Combination of flags that determine which option or futures prices in historical data or live data are given in cents instead of dollars (default **0** = all prices in dollars). Prices are then automatically converted to dollars when loading history or retrieving data from the broker API. Centage flags can be combined by adding them up. Asset specific; to be set after selecting the asset and before buying contracts.  
  
**1** \- strike in cents, in live contract data  
**2** \- strike in cents, in contract history  
**4** \- underlying in cents, in live contract data  
**8** \- underlying in cents, in contract history  
**16** \- ask/bid in cents, in live contract data  
**32** \- ask/bid in cents, in contract history  
**64** \- underlying prices and spreads in cents, in live and historical price data (similar to [HedgeRatio](pip.htm) = 0.01).

Example: **Centage = 59** (= 32+16+8+2+1) for ask/bid always in cents, underlying history in cents but live in dollars, and strike always in cents.

### Type:

**int** , asset specific

## ExpiryTime

The hour in HHMM format at which contracts expire; default **1200** (UTC). Used for [contractDays](contract.htm). Added to the [ TradeExitDate](trade.htm) variable when a trade is opened. At that time on its expiration day, the trade will be removed from the open trade list, and its profit or loss will be booked to the account. Set this to a higher value for checking contract expiration with the [contractCheck](contract.htm) function in live trading. Note that some exchanges have restrictions to trading or exercising contracts at their expiration day. 

### Type:

**int** , global for all assets 

## Exchange

The exchange for the contracts in live trading. Set the exchange symbol manually through this variable when the exchange is required by the broker. The list of valid exchange symbols can be obtained from the broker website. 

### Type:

**string** , asset specific.   

### Remarks:

  * Contract variables are only valid when a contract chain was loaded (see [contractUpdate()](contract.htm)).
  * **Multiplier** and **Centage** (if required) must be set up for the current asset before loading contract chains.
  * Some assets require **Exchange** to be set for retrieving prices and trading with options.  

### Examples: see [Options and Futures](contract.htm)

### See also:

[enterLong/](buylong.htm)[Short](buylong.htm), [Contract](contract.htm), [Combo](combo.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
