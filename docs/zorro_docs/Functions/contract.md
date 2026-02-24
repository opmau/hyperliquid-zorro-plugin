---
title: Futures and options
url: https://zorro-project.com/manual/en/contract.htm
category: Functions
subcategory: None
related_pages:
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [Dataset handling](data.htm)
- [Dates, holidays, market](date.htm)
- [Asset Symbols](symbol.htm)
- [brokerCommand](brokercommand.htm)
- [Retraining](retraining.htm)
- [enterLong, enterShort](buylong.htm)
- [asset, assetList, ...](asset.htm)
- [trade management](trade.htm)
- [IB / TWS Bridge](ib.htm)
- [Broker API](brokerplugin.htm)
- [Indicators](ta.htm)
- [R Bridge](rbridge.htm)
- [System strings](script.htm)
- [Licenses](restrictions.htm)
- [for(open_trades), ...](fortrades.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [assetHistory](loadhistory.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Asset and Account Lists](account.htm)
- [wait](sleep.htm)
- [Spread, Commission, ...](spread.htm)
- [combo, ...](combo.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [optimize](optimize.htm)
- [Virtual Hedging](hedge.htm)
- [Time / Calendar](month.htm)
- [Included Scripts](scripts.htm)
- [contractCPD](contractcpd.htm)
- [W8 - Options Trading](tutorial_options.htm)
---

# Futures and options

# Options, Futures, FOPs

The following functions can be used for selecting, analyzing, and trading options, futures, and options on futures (FOPs). Due to their additional parameters, such as type, strike, and expiration, options trading is a bit more complex than trading the underlying. An introduction to options trading and options strategy development with Zorro can be found in the **[Black Book](https://www.amazon.de/Black-Book-Financial-Hacking-Algorithmic/dp/1546515216)** chapter 8. A very brief introduction is also found on Financial Hacker. 

## contractUpdate (string Name, int Handle, int Mode): int 

Loads a new chain of contracts of type **Mode** (see below) for the current underlying and the current day. The contract chain contains all options, futures, or FOP contracts available at a certain date. It is required for all the subsequent contract functions and for entering or exiting contract positions.  
Contract chains are only loaded when the [LookBack](lookback.htm) period is over and real trading begins. They are loaded individually per asset and can be directly accessed with the [Contracts](contracts.htm) pointer. Chains are automatically sorted at loading: first by expiration date, then by strike. This allows fast finding a certain option or combo. [NumContracts](contracts.htm) is set to the number of contracts in the chain, which is also returned. If no contract chain for the current or given time is found, **0** is returned.  
In [Test] or [Train] mode the chain is either read out of a contract chain file named **"History\Name.t8"** at or before the current time, or from a previously loaded [dataset](data.htm) of contracts when its **Handle** is nonzero. **Name** can contain month or day numbers for monthly or daily historical contract files. If **Name** is **0** , the current asset name is used. If the [Now](date.htm) variable is nonzero in [Test] or [Train] mode, it is used for filtering contracts instead of the current time. [ContractRow](contracts.htm) is set to the row of the first found contract in the dataset. This allows to load extra data, such as greeks, from a second parallel dataset.  
In [Trade] mode the chain for the symbol **Name** is either downloaded from the broker API, or - when **Handle** is nonzero - filled from a previoulsy loaded [dataset](data.htm) with no date/time filtering. If **Name** is **0** , the current asset name is used. For contracts that are no US stocks or traded on multiple exchanges, **Name** should be a [symbol](symbol.htm) with exchange, currency, and for futures the expiration date (e.g. **"N225-FUT-20220609-225-OSE.JPN-JPY"**). For limiting the chain to a particular trading class, use the [ SET_CLASS](brokercommand.htm) command. 

## contractChain (string Name, int* Parameters): int 

A fast alternative to **contractUpdate** ; generates an artificial options chain from a parameter list of expirations and strikes (see below). In [Test] or [Train] mode the chain is generated for the current asset, regardless of **Name**. When the **Parameters** pointer is **0** in [Trade] mode, the lists of options expirations and strikes for the underlying symbol **Name** are downloaded from the broker API with the [GET_CHAIN](brokercommand.htm) command. If **Name** is **0** , the current asset name is used. For limiting the chain to a particular trading class, use the [ SET_CLASS](brokercommand.htm) command.   
Note that a real options chain is a subset of this artificial chain, since it does not contain all combinations of expirations and strikes. For trading an option from the artifical chain, fill in the class if it is different to the symbol, and call **contractPrice** to find out if the option is available for trading. 

## contractRecord (string Name, var MinStrike, var MaxStrike, int MinDays, int MaxDays)

Appends the current contract chain to the begin of the **.t8** contract history file in [Trade] mode. This allows recording live contract chains for [re-training](retraining.htm) or other purposes. The prices of all recorded contracts are loaded from the broker API. Because loading contract prices can take a long time (up to 10 seconds per option contract with the IB API), the strike range and life time can be limited with the **MinStrike** , **MaxStrike** , **MinDays** , **MaxDays** parameters. Only contracts that fall into the limits and have a nonzero price are recorded. If the limits are 0, prices for the whole chain are loaded. Example: **contractRecord(0,priceClose()-50,priceClose()+50,1,70);** appends all contracts within a +/-50 strike range and up to 70 days expiration at the begin of the **.t8** file of the current asset. 

## contract (int Type, int Expiry, var Strike): CONTRACT*

Selects the option or future contract from the current option chain that matches exactly the given **Type** (including **FUTURE** or **EUROPEAN**), **Expiry** date, and **Strike** value. Returns a **CONTRACT** pointer to the found contract, or **0** when no such contract was found. If a contract is selected, [enterLong](buylong.htm) and [enterShort](buylong.htm) will buy or write (sell short) the contract instead of the underlying asset. The function sets up [ContractStrike](contracts.htm), [ContractExpiry](contracts.htm) and other contract variables, and sets [ContractRow](contracts.htm) to the row of the selected contract in the historical dataset or in the options chain. Calling [asset()](asset.htm), **contractUpdate()** , or **contract(0,0,0)** deselects the contract and allows again trading with the underlying. 

## contract (int Type, int Days, var Strike): CONTRACT*

As above, but selects the option or future contract that has an expiration of at least the given minimum life time in **Days** and strike closest to the given **Strike** value. Returns the **CONTRACT** pointer, or **0** when no contract with matching **Type** , at least **Days** life time, and a strike closer than 10% to **Strike** was found. 

## contract (int N): CONTRACT*

As above, but selects the **N** th contract from the current contract chain, where **N** = **1..**[NumContracts](contracts.htm). If **N = 0** , the currently selected contract is deselected in order to allow trading with the underlying. 

## contract (CONTRACT*): CONTRACT*

As above, but selects directly the given option or future contract, and does not alter [ContractRow](contracts.htm). 

## contract (TRADE*): CONTRACT*

As above, but selects the contract from a contract trade. Returns **0** if no contract for this trade was found, which can happen at the expiration day or when the chain was not updated. Otherwise [trade variables](trade.htm) and [contract variables](contracts.htm) are available for the trade and contract. 

## contractNext (int Type, int Expiry, var Strike): CONTRACT*

As above, but selects and returns the option or future contract with the **Type** and **Expiry** , but the next-higher strike than the given **Strike** value. If **Strike** is negative, selects and returns the contract with the next-lower strike value. 

## contractFind (int Type, int Days, var Find, int N): CONTRACT*

## contractFind (int Type, int Days, var Find, int N, var MinStrike, var MaxStrike): CONTRACT*

## contractFind (int Type, int Expiry, var Find, int N): CONTRACT*

## contractFind (int Type, int Expiry, var Find, int N, var MinStrike, var MaxStrike): CONTRACT*

As above, but selects the option or future contract with the given **Type** , closest to the given minimum life time in **Days** , and a parameter closest to the given **Find** value. The parameter can be selected with **N** : 1 = ask, 2 = bid, 3 = fVal, 4 = fVol, 5 = fUnl, 6 = strike, 7 = strike distance from the underlying. The strike range can be optionally limited to **MinStrike** and **MaxStrike** for speeding up the function. Returns a **CONTRACT** pointer, or **0** when no contract was found. If a selected parameter is not yet available, such as ask, bid, or underlying price in live trading, it is automatically retrieved from the broker API. Depending on the number of contracts that match the expiration and strike range, this function can then take a long time (up to 10 seconds per contract) on the first call in live trading after loading a new contract chain. Subsequent calls are faster. 

## contractDays (CONTRACT*): var 

## contractDays (TRADE*): var 

Returns the fractional number of calendar days until contract or trade expiration (see [ExpiryTime](contracts.htm)). Returns a negative number when the expiration time is in the past. Note that the exact time to expiration depends on the time zone and market close time of the exchange where the contract is traded, so adapt [ ExpiryTime](contracts.htm) to your exchange when the exact life time in hours and minutes is needed. Note that many exchanges have restrictions to trading contracts at their expiration day. 

## contractPrice (CONTRACT*): var 

## contractPrice (TRADE*): var

Selects the contract and updates the current ask, bid, volume, and underlying price of a contract, as well as the fill amount and trade profit of a trade, either from the broker API, or from historical data by the last **contractUpdate** call. Only valid as long as the contract is not expired. Returns the bid/ask mid price per underlying unit. If volume is available from the broker, it is updated to the **fVol** element.   
Updating contract prices can be slow, depending on broker. Some broker plugins, such as [IB](ib.htm), support a 'fast mode' by **brokerCommand(SET_PRICETYPE,8);** this can remarkably speed up the price update. For retrieving the underlying price in live trading, the [GET_UNDERLYING](brokercommand.htm) command is used; for updating the fill amount, the [BrokerTrade](brokerplugin.htm) function is used and must be supported by the API. If the contract is not traded or the market data not subscribed, the function returns **0**. Contract prices of open positions are automatically updated at any bar. If the underlying price is not available, it is updated from the current asset price. The underlying price is automatically copied to all contracts of the current chain with the same expiration date. 

## contractUnderlying (): var 

Returns the unadjusted underlying price, in [Trade] mode from the current contract, otherwise from the **fUnl** element of the first contract of the chain. If the underlying price is not available, returns the price of the currently selected asset.Make sure with **contractUpdate** and **contractPrice** that the contract is up to date. Note that FOPs have no common underlying price, but **fUnl** depends on the expiration and can be different for any contract. Source code in **contract.c**. 

## contractPosition (TRADE*): int 

Returns the current number of open lots or open contracts of the given trade (negative values for a short position). Can be used for determining if a certain contract was expired or exercised by the broker. In live trading the [GET_POSITION](brokercommand.htm) command must be supported by the broker API, and no other trades with the same contract type, strike, and expiry must be opened. 

## contractCheck (TRADE*, int Mode): int

Checks the option position on the broker account in live [Trade] mode. If it is still open, returns the total number of open contracts (negative for a short position). Otherwise it checks for an open position of the underlying, and returns the position size. If **Mode** is at **1** and no open contract was found, the trade is automatically closed with a "**Lapsed** " message. If **Mode** is at **3** and an underlying position was found, it is sold at market. **Mode = 3** thus replicates the backtest behavior of expired options. The function can be called regularly in a [for(open_trades)](for_trades.htm) loop for keeping the positions between broker and strategy in sync. The [GET_POSITION](brokercommand.htm) command must be supported by the broker API, and no other trades with the same contract type, strike, expiry, and underlying must be opened. 

## contractRoll (TRADE*, int Days, var Strike, function TMF): TRADE* 

Rolls a closed option or future contract by opening a duplicate with the same type, volume, stop loss and profit distances, the given number of **Days** until expiration, the given **Strike** (**0** for using the previous strike) and an optional [trade management function](trade.htm). Returns a pointer to the new trade, or 0 if the contract could not be rolled. Source code in **contract.c**. 

## contractSellUnderlying (): int 

Checks in [Trade] mode if any underlying position of the current asset is open, and if so, sells it at market. Returns the number of lots, or 0 when no underlying position was found. Source code in **contract.c**. 

## contractExercise (TRADE*)

Exercises an option contract. In the backtest, the option is closed at its intrinsic price (see remarks). In live trading, an order to exercise the option is sent to the broker. It is normally not immediately executed, but pending. Use **contractCheck()** for closing the trade and selling the underlying as soon as the order was executed. Don't call this function for European options before expiration, or for options that are not in the money.  

##  contractVal (CONTRACT*, var Price, var HistVol, var Dividend, var RiskFree, _var* Delta, var* Gamma, var* Vega, var* Theta, var* Rho_): var

Returns the theoretical value and optionally the greeks of the given option contract at the given unadjusted **Price** of the underlying. **Type** , **fStrike** , and **Expiry** of the contract must be set; the other parameters don't matter. **Expiry** can be either an expiration date in the YYYYMMDD format, or alternatively the number of calendar days until expiration. **HistVol** is the underlying annual volatility of the log returns, f.i. by [Volatility](ta.htm) or [VolatilityOV](ta.htm) (usually over 20 days). **Dividend** is the continuous annual dividend yield per year of the underlying, as a fraction in the **0..1** range. **RiskFree** is the annual risk-free yield as a fraction in the **0..1** range, f.i. by **yield()/100**.   
**Delta** is the impact of an underlying price change on the option value, in the **0..1** range for calls and **0..-1** for puts. **Gamma** is the impact on **Delta**. **Vega** is the impact of a change of the underlying volatility, **Theta** is the impact of a change of the expiration time, and **Rho** is the impact of a change of the risk-free yield.   
The function uses [R](rbridge.htm) and the **RQuantLib** package; both must be installed (see remarks below). Source code in **contract.c**.

## contractVol (CONTRACT*, var Price, var HistVol, var Value, var Dividend, var RiskFree): var

Returns the implied volatility (IV) of the given option contract with the given **Value**. For the parameters, see **contractVal**. The function uses [R](rbridge.htm) and the **RQuantLib** package; both must be installed on the trading PC (see remarks below). Source code in **contract.c**. The implied volatility is an estimate of the future volatility of the underlying, based on the current option parameters. If the contract has no value or if the strike price is too far or on the wrong side of the asset price, the function returns **0**. 

## contractDelta (int Type, int Days, var Price, var HistVol, var RiskFree, var Strike): var

Returns the theoretical delta of a contract of given **Type** with the given **Days** until expiration, unadjusted underlying **Price** , volatility **HistVol** , **RiskFree** yield rate, and **Strike**. Uses the formula **_Delta = cdf((ln(Price/Strike) \+ (r + (sigma^2)/2) * T) /(sigma * sqrt(T)))_**. Source code in **contract.c**. Delta is the first derivative of the option value with respect to the underlying price, and can be used as a proxy of the probability to expire in the money. Note that some platforms and brokers use individual methods for calculating **HistVol** and thus produce different Delta values for the same contracts.

## contractStrike (int Type, int Days, var Price, var HistVol, var RiskFree, var Delta): var

Returns the theoretical strike value of a contract of given **Type** with the given **Days** until expiration, unadjusted underlying **Price** , volatility **HistVol** , **RiskFree** yield rate, and **Delta**. Uses the formula **_Strike = Price * exp(-qnorm(Delta) * sigma * sqrt(T) + (r + (sigma^2)/2) * T)_**. If a positive **Delta** is entered for a put, it is converted to the equivalent negative delta, so the same **Delta** value can be used for selecting put and call contracts. Source code in **contract.c**. 

## contractIntrinsic (CONTRACT*, var Price): var

## contractIntrinsic (TRADE*, var Price): var

Returns the intrinsic value of the option contract per underlying unit at the given **Price** of the underlying. The intrinsic value of a call option is the difference between price and strike; for put options it's the difference between strike and price. A positive difference means that the option is in the money, a negative difference means it's out of the money. **Type** and **fStrike** of the contract or trade must be set; the other parameters don't matter. Source code in **contract.c**.

##  contractProfit (CONTRACT*, var Price, var Premium): var

Returns the profit per underlying unit of a contract sold or purchased at the given **Premium** when expiring at the given underlying **Price**. **Premium** is positive for sold, negative for bought contracts. **Type** and **fStrike** of the contract must be set; the other parameters don't matter. Trading costs are not included. The calculation of profit or loss is described in the remarks below. Source code in **contract.c**. 

## contractMargin (CONTRACT*, int AssetType): var

Calculates the margin cost of the given non-covered short put/call contract for the given **AssetType**(the margin of long contracts is simply their ask price). Uses the margin formula on the Interactive Broker's [US Options Margin Requirements](https://www.interactivebrokers.com/en/index.php?f=26660&hm=us&ex=us&rgt=1&rsk=0&pm=1) page. Source code in **contract.c**. 

## contractClass (CONTRACT*): string

Returns a pointer of the class name of the given contract; only available in live trading mode since historical contracts have no class. Source code in **contract.c**. 

## contractLetter (int Month): string

Returns the letter assigned to the given expiration month (F Jan, G Feb, H Mar, J Apr, K May, M Jun, N Jul, Q Aug, U Sep, V Oct, X Nov, Z Dec). This function can be used to generate a contract [symbol](symbol.htm). Source code in **contract.c**. 

## contractSetSymbol (int Days, int N): int

Sets [SymbolLive](script.htm) and [SymbolTrade](script.htm) of the current selected underlying future to the next expiration date after the given number of days. The expiration date is the third Friday of either any month (**N = 1**) or any third month (**N = 3**). Returns the expiration date in YYYYMMDD format. Source code in **contract.c**.   

## contractPrintChain ()

Exports the current contract chain to a **Data\\*.csv** file, for diagnostics purposes, 

## contractPrint (CONTRACT*)

Prints the given contract parameters to the log file for diagnostics purposes, in the order Date, Type, Expiry, Strike, Underlying, Ask, Bid, fVal, fVol. 

## plotContract (int N, CONTRACT*, var HistVol, var Min, var Max, int Days, int Modus)

Plots the payoff diagram of **N** contracts (negative for selling) with volatility **HistVol** from price **Min** to price **Max**. **Modus 0** initializes the plot and plots a zero line, **Modus 1** plots the diagram at expiration, **Modus 2** at the given **Days** into the lifetime (**RQuantLib** required), and **Modus 3** plots the Delta. Type, strike, ask, bid, and expiry in days must be set in the **CONTRACT** struct. Source code in **contract.c** ; usage example in **PayOff.c**.   
  

## yield(): var

Helper function, returns the current yield rate of 3-months US treasury bills in percent. Can be used as a proxy of the risk-free interest for calculating the values of option contracts (divide it by **100** for using it in **contractVal** or **contractVol**). Works in backtest as well as live trading. Uses the [dataFromQuandl](data.htm) function. [Zorro S](restrictions.htm) required for Quandl data access; enter your Quandl key in the **QuandlKey** field in **Zorro.ini**. Source code in **contract.c**. 

## yieldCSV(): var

As above, but gets the current yield rate from an already-downloaded treasure history in **History\FRED-DTB3.csv**. 

## initRQL(): int

Helper function that initalizes **RQuantLib**. Call this in the **INITRUN** when **contractVal** or **contractVol** are used. 

### Parameters:

**Name** | The name of the underlying, f.i. **"ES"** or **"SPY",** or an underlying symbol - if different to the name - in live trading, f.i. **"DAX-IND-EUREX-EUR"**. Give **0** for using the name or symbol of the current asset. For contract chains in backtest mode, use the name of the current **.t8** file (f.i. **"SPY_202412.t8"**) for loading monthly or daily files.  
---|---  
**Handle** | A number from **1...800** that identifies a previously loaded [dataset](data.htm) containing a **CONTRACT** list with historical options data; or **0** for automatically loading content from a historical file or from the broker API.   
**Mode** | **PUT+CALL** for options, **FUTURE** for futures, **PUT+CALL+FUTURE** for options on futures.   
**Parameters** | Pointer to an **int** array. The first element contains the number of expirations, second the number of strikes, followed by the list of expirations in the YYYYMMDD format, followed by the list of strikes cast to **float**. Give **0** for loading the array from the broker.   
**Type** | The exact contract type to match, a combination of **PUT** , **CALL** , **FUTURE** ,**EUROPEAN** , **BINARY** (f.i. **PUT+EUROPEAN**), plus the following optional flags:  
**+ONLYW3** \- select only contracts that expire at the 3rd Friday, i.e. between the 16th and the 21th of the month.  
**+ONLYMATCH** \- select only contracts that exactly match the expiration date.  
  
**AssetType** | **1** for a currency, **2** for an index, **3** for stocks and ETFs. The margin for futures must be individually calculated.  
**Days** | The minimum number of calendar days until expiration. The closest expiration date will be selected.   
**Expiry** | The expiration date in the **YYYYMMDD** format.   
**Strike** | Option strike price, or **0** for futures.   
**Delta** | First derivative of the option value with respect to the underlying price. Determines the impact of an underlying price change on the option value, and can serve as a proxy of the probability to expire in the money. **0..1** for calls and **0..-1** for puts.  
**HistVol** | The historical annualized volatility of the underlying asset, as a fraction, f.i. 0.2 for 20%. Normally calculated with the [Volatility](ta.htm) or [VolatilityOV](ta.htm) indicators from the last 20 days. For the calculation of greeks, implied volatility is often used instead of historical volatility.  
**Dividend** | The annual dividend yield of the underlying, as a fraction, f.i. 0.02 for 2% dividend yield.   
**RiskFree** | The risk-free interest rate, as a fraction, f.i. **yield()/100**.   
**Value** | The value of the option for determining the implied volatility.   
**TRADE*** | The pointer of an option or future trade. Use **ThisTrade** for the trade pointer in a [TMF](trade.htm) or [trade enumeration loop](fortrades.htm).   
  
### Remarks:

  * **contract.c** must be included for all **contract** functions. The underlying asset must be selected and a contract chain must be loaded before calling a **contract** function.
  * The **contractVal** and **contractVol** functions require [R](rbridge.htm) with the **Rcpp** and **RQuantLib** packages that you can install from the Cran repository, either with the **install.packages** command or from the menu when using RStudio. For checking if everything is working, call a library function from the R console, like this:  
  

```c
require('RQuantLib')
AmericanOption('put', underlying=100, strike=100, dividendYield=0.02, riskFreeRate=0.01, maturity=0.5, volatility=0.4, engine='CrankNicolson')

Concise summary of valuation for AmericanOption
  value  delta  gamma  vega theta rho divRho
11.4122 -0.4463 0.0139  NA   NA   NA   NA
```

  
**RQuantLib** must be initialized in the **INITRUN** by calling **initRQL** The current **RQuantLib** version tends to crash when a function is called with bad parameters, like negative volatility, or with a value that contradicts the strike and spot price, or with too long strike distances. After a crash, all **RQuantLib** functions return 0.
  * Contract chains loaded from historical data normally contain prices, underlying, and optional data such as delta. Chains downloaded from the broker normally contain only strikes and expirations, but no prices. Since loading option chains can take a long time (~20 minutes for long chains with some brokers), and expirations and strikes don't change intraday, it is recommended to call **contractUpdate** in live trading only once per day, preferably outside market hours. If the broker API supports the **GET_CHAIN** command, use **contractChain** instead, which takes only a few seconds. Prices can be updated with **contractPrice** ; prices of open trades are automatically updated**.** In test and training, **contractUpdate** should be called at any bar for updating the prices.
  * Some brokers are reported to return  already-expired contracts in their contract chain. Make sure to check the expiration date when selectiung a contract.
  * High resolution options data can be split in monthly or daily **.t8** files and loaded per script in a dataset that is then fed to the **contractUpdate** function (see example).
  * [Entry](stop.htm), [Stop](stop.htm), and [TakeProfit](stop.htm) limits also work for trading options and futures. They must be given as an absolute price limit to the ask price of the contract, not as a distance. When an exit limit is hit, the contract is sold at market. Of course the usefulness of stop/profit limits for option values is arguable. 
  * For entering a trade, make sure that a contract is selected. Otherwise you would unwillingly open a position of the underlying. Check with **contracePrice()** that a nonzero ask or bid price, depending trade direction, is available for the contract or for all contracts of a combo. Set the **[Multiplier](contracts.htm)** correctly, otherwise the backtest result will be wrong. Some brokers also require the [Exchange](contracts.htm) to be set. The correct multiplier of an option or future contract can be found on the asset description on the website of the broker or the exchange. For stocks, the multiplier is normally 100. 
  * Underlying prices downloaded with the [assetHistory](loadhistory.htm) function are normally split and dividend adjusted and therefore won't match strike prices. You can use the **[UNADJUSTED](loadhistory.htm)** flag for downloading unadjusted prices instead. But often you'll need both sorts of prices in an options trading system, unadjusted prices for calculating greeks or determining strikes, and adjusted prices for historical volatility and for indicators. The usual convention is having adjusted prices in **.t6** history and using the **contractUnderlying()** function for the unadjusted prices. 
  * When an option is closed, either by an [exit](selllong.htm) command or by reaching a stop or profit target, it is automatically sold back or covered at the current ask or bid price. If no market price is available, the option stays open until the next exit command or until it expires or is exercised. Options and futures can only be traded, and their prices can only be downloaded when the market is open. Do not send [enter](buylong.htm), [exit](selllong.htm), or **exercise** option positions outside market hours.
  * If an option expires or is exercised, a position of the underlying can appear. For live trading, the **contractCheck** command can check if an underlying position is open, and sell it automatically. In the backtest, underlying positions opened by expired or exercised options are immediately sold at market, and the profit or loss is booked on the account. 
  * For calculating profit/loss of an option, the following algorithm is used:  
\- A call is assumed in the money when the strike is below the bid price (i.e. ask - spread); a put is assumed in the money when the strike is above the ask price.   
\- A long call or short put in the money buys the underlying at strike price, and sells it at bid price. The difference is added to or subtracted from the account.  
\- A long put or short call in the money sells the underlying at strike price, and buys it back at ask price. The difference is added to or subtracted from the account.  
\- For a long position the premium is subtracted from the account, for a short position the premium is added to the account.
  * An example for finding the parameters of commodity FOPs. Go to <https://www.cmegroup.com/trading> and enter the name, for instance "Natural Gas", in the search field. A list of symbols will appear (otherwise you have to look it up at a different exhange).  Select the symbol with the high trade volume, in the example "NG". Click on the link under Product Name. Then select  Contract Specs. The multiplier (10,000 for NG) is the Contract Unit, the pip size of the underlying is the Minimum Price Fluctuation ($0.001). Pip cost are the $0.001 in your account currency. Under Quotes you can see the current price per underlying unit ($3.146). Check if the price is in cents or dollars (see below). Under Margins you can see that the most recent November 2018 contract has $1950 maintenance margin. That's $0.195 margin cost per underlying unit. Now look up the symbol in your broker platform and check its ticker name. Check also if your broker uses the same margin. That's all you need to add the underlying to your [asset list](account.htm). The line will look like this (on an USD account): **NG,3.146,0.001,0,0,0.001,0.001,0.195,0,1,0.002,NG**.
  * Futures and options on futures (FOPs) sometimes have prices sometimes in cents, sometimes in dollars, sometimes mixed in any possible combination of historical and live data. For trading with FOPs, check historical data and broker data and determine which prices - ask/bid, strike, underlying - are in cents and which in dollars. This can be different with any future and any broker. Set the [Centage](contracts.htm) variable accordingly for using consistent dollar prices in the strategy, charts, and reports.
  * The class or root symbol of a contract can be retrieved in live trading from the first element of a **CONTRACT** struct: **string RootSymbol = (string)MyContract;**. Note that this only works in Live mode, as in the backtest the first CONTRACT element is the time.
  * Many broker APIs, for instance [IB](ib.htm), do not manage trades, but only positions. Those APIs can not distinguish between an underlying position opened by a normal trade, and a position opened by an exercised option. This must be considered in the script. Buying long and short the same contract with the same strike and expiry is treated as two different trades by Zorro, but as two positions that cancel each other by most brokers.
  * Some broker APIs, for instance [IB](ib.htm), need a certain "recovery time" after downloading an options chain before downloading the next chain. When downloading chains of multiple assets, [wait](sleep.htm) some seconds between **contractUpdate** calls.
  * Use the [exit](selllong.htm) command if you want to close a contract by selling it or buying it back. On NFA accounts, functions like **contractPosition** or **contractCheck** only check the net position of open option trades, and will return wrong results when several long and short positions of the same contract are open. For the same reason, multiple scripts can trade options on the same NFA account only when they don't open the same contracts. 
  * [Slippage](spread.htm) is not simulated for options. Commission and margin are set by default to the commission and margin of the underlying in the [asset list](account.htm), multiplied with the [Multiplier](contracts.htm). Commission is reduced by 50% when an option expires worthless. Since most brokers have different and complex calculation methods for option margins, the default margin is just an approximation. For better accuracy, use the **contractMargin** or [comboMargin](combo.htm) functions or calculate [Commission](spread.htm) and [MarginCost](pip.htm) parameters explicitely in the script prior to entering a trade. For instance when trading vertical spreads, margin cost is often determined by the strike price difference, therefore set **MarginCost = 0.5 * (Strike1-Strike2)** before opening the two positions. The margin cost of covered contracts is usually 0. The margin requirements by IB can be found on <https://www.interactivebrokers.com/en/index.php?f=24176>.
  * When [optimizing](optimize.htm) option strategies, the default **objective** function is not the ideal performance measure for options. Better is dividing the total profit by the required margin. Example: 

```c
var objective()
{
  if(!NumWinTotal && !NumLossTotal) return 0;
  return (WinTotal-LossTotal)/MarginMax;
}
```

  * Phantom trades and virtual hedging are not used for contracts. Options can always be hedged regardless of the [Hedge](hedge.htm) setting. 
  * Some historical options before 2014 expired on Saturday; afterwards they expired on Friday. Options with Saturday expiration are treated as if they expired on Friday. The expiration hour can be set up with the [ExpiryTime](contracts.htm) variable.
  * The **CONTRACT** struct is defined in **trading.h**. For backtests the following struct elements must be filled in the data set; for futures: **time, fAsk, fBid, Expiry, Type** ; for options: **time, fAsk, fBid, fStrike, Expiry, Type**. The other **CONTRACT** elements are optional and for use in the script only. 
  * **contract(Contracts)** selects the first contract in the chain.
  * For calculating the option value, American options are approximated with the Crank-Nicolson method, while European options are calculated with the (much faster) Black-Scholes formula. For short-term options the results of both methods are not very different, so the **EUROPEAN** flag can be used in such cases even for Americon options in order to speed up the calculation.
  * The **contractVol** function is known to terminate the R session when called with unrealistic parameters, such as an unlikely small option value for a high strike-spot difference and short expiration date. This can be tested with the [Rrun](rbridge.htm) function ( **if(!Rrun()) quit("R terminated!");**). When the R session is terminated by an exception in the backtest, Zorro must be restarted. 
  * For finding the contract with the lowest or highest strike, start the with strike price at the current price, then call **contract(...)** in a loop and modify the strike in steps until the condition is fulfilled.. 
  * For converting dates from or to the **YYYYMMDD** format or for finding the 3rd Friday of a month, use the [dmy](month.htm), [ymd](month.htm), and [nthDay](month.htm) functions. 
  * Inverse contracts have a nonlinear payoff, and bear higher risk and higher volatility than standard contracts. The profit/loss of such positions must be calculated by script (see [TradeCost](trade.htm)).
  * Several option scripts are included. The **[TradeOptions](scripts.htm)** script can be used for testing the opening and closing of **SPY** options via broker API. It only runs at NY market hours and takes about 30 seconds for downloading the option chain at start. The **[Payoff](scripts.htm)** script can interactively display payoff and delta diagrams of option combinations. The [OptionsCalculator](scripts.htm) script calculates the price and delta of calls and puts, where underlying, strike, and expiration of options can be set up with sliders. Feel free to combine the scripts to your own specialized options tool.  

### Examples:

```c
// Sell call and put options at 1 stddev of the underlying// Buy them back 2 days before expiration
#include <contract.c>

void run() 
{
  BarPeriod = 1440;
  BarZone = EST;
  BarOffset = 15*60; // trade at 3 pm Eastern time
  LookBack = 20;
  if(Init) {
    assetList("AssetsIB");
    assetHistory("SPY",FROM_AV|UNADJUSTED);
    brokerCommand(SET_CLASS,"SPYW");
  }
  asset("SPY");
  Multiplier = 100;
 
  if(!NumOpenTotal && !is(LOOKBACK)) {
    contractUpdate(Asset,0,PUT|CALL);
    vars Close = seriesC();  
    int DTE = 6*7; // look for 6-week contracts
    var Strangle = StdDev(Close,20);
    CONTRACT* Call = contract(CALL,DTE,Close[0] + Strangle);
    CONTRACT* Put = contract(PUT,DTE,Close[0] - Strangle);
    if(Call && Put) { // open single positions, no combo
      contract(Call); enterShort();
      contract(Put); enterShort();
    }
  }

// Check expiration and buy them back when in the money  
  for(open_trades) 
  {
    if(contractDays(ThisTrade) <= 2
      && contractIntrinsic(ThisTrade,Close[0]) > 0)
         exitTrade(ThisTrade);
  }
}
```

```c
// Load high res contract chains from daily files like "SPY_20210505.t8"
if(!is(LOOKBACK) && day(0) != day(1)) { // new day?
  if(Live) { // load a new contract chain
    if(!contractChain(0,0)) // first try the fast way
	contractUpdate(0,0,CALL|PUT); // then the slow way
  } else // backtest: load every day a new t8 dataset with prices 
    dataLoad(1,strf("History\\%s_%04d%02d%02d.t8",Asset,year(0),month(0),day(0)),9);
}
if(!Live) // backtest: update prices at any bar
  contractUpdate(0,1,CALL|PUT);
```

```c
// Example script for converting EOD options data to .t8:
// Format: underlying symbol, exchange, date MMDDYYYY, adj close, option symbol, expiry MMDDYYYY, strike, Call/Put, American/European, ask, bid, volume, open interest, close
// Sample: "TLT,NYSEArca,04/10/2015,129.62,TLT   150410C00112500,04/10/2015,112.5,C,A,17.3,16.2,0,0,129.62"
string Format = ",,%m/%d/%Y,,,i,f,s,s,f,f,f,f,f";

void main() 
{
// first step: parse the CSV file into a dataset
  int Records = dataParse(1,Format,FILENAME);
  printf("\n%d Records parsed",Records);
// second step: convert the raw data to the final CONTRACT format
  for(i=0; i<Records; i++) 
  {
    CONTRACT* C = dataAppendRow(2,9);
    C->time = dataVar(1,i,0);
    string PC = dataStr(1,i,3);
    string EA = dataStr(1,i,4);
    C->Type = ifelse(*PC == 'P',PUT,CALL) + ifelse(*EA == 'E',EUROPEAN,0);
    int Expiry = dataInt(1,i,1); 
    C->Expiry = 10000*(Expiry%10000) + Expiry/10000; // MMDDYYYY -> YYYYMMDD
    C->fStrike = dataVar(1,i,2);
    C->fAsk = dataVar(1,i,5);
    C->fBid = dataVar(1,i,6);
    C->fVol = dataVar(1,i,7);
    C->fVal = dataVar(1,i,8); // open interest
    C->fUnl = dataVar(1,i,9);
    if(!progress(100*i/Records,0)) break; // show a progress bar
  }
  dataSort(2);
  dataSave(2,"History\\MyOptions.t8");
}
```

### See also:

[enterLong/](buylong.htm)[Short](buylong.htm), [exitLong/](selllong.htm)[Short](selllong.htm), [Quandl bridge](data.htm), [date functions](month.htm), [contract variables](contracts.htm), [contractCPD](contractcpd.htm), [combo](combo.htm), [workshop 8](tutorial_options.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
