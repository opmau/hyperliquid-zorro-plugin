---
title: Asset list, account list
url: https://zorro-project.com/manual/en/account.htm
category: Functions
subcategory: None
related_pages:
- [asset, assetList, ...](asset.htm)
- [brokerCommand](brokercommand.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [Spread, Commission, ...](spread.htm)
- [AssetMode](assetmode.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Performance Report](performance.htm)
- [Time zones](assetzone.htm)
- [Dates, holidays, market](date.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [Asset Symbols](symbol.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [Included Scripts](scripts.htm)
- [System strings](script.htm)
- [Dataset handling](data.htm)
- [contract, ...](contract.htm)
- [Licenses](restrictions.htm)
- [Virtual Hedging](hedge.htm)
- [Broker Arbitrage](brokerarb.htm)
- [Equity Curve Trading](trademode.htm)
- [Testing](testing.htm)
---

# Asset list, account list

# Asset Lists and Account Lists

Any asset used in a trading script takes its parameters and symbols from an entry in an **asset list** in the **History** folder (unless the asset was added by script). This entry determines all properties of the asset. The default asset list, **AssetsFix.csv** , also determines the assets that initially appear in the [Asset] scrollbox. Since any broker has his individual asset parameters and symbols, different asset lists are used for different brokers and accounts. The asset list can be selected either by script through the [assetList](asset.htm) command, or automatically with an **account list** (see below). If no asset list is selected, the default list is used. 

The parameters in the asset list are used for training and testing. In live trading, asset parameters are not read from the list, but taken directly from the broker API. However if parameters are not provided by the broker API, they are also taken from the asset list. Since broker API parameters are not always reliable, the [SET_PATCH](brokercommand.htm) command can be used to override the API and enforce the asset list parameters. 

The asset parameters need not be 100% precise. A backtest with an asset list for a different broker or account will normally also produce a useful result. But the parameters should not be totally off, especially not leverage and lot amount. If asset list parameters appear suspicious or deviate strongly from live parameters, a warning is issued at start of a live trading session.

Asset lists are simple comma separated spreadsheet files (CSV files) that can be edited with Excel or with a text editor for adding new assets, or for modifying parameters. Every asset is represented by a line in the CSV file. New assets can be added either manually - by editing the file and entering a new line - or semit-automatically from **Log\Assets.csv** as described below. For adding a new asset to the scroll box, edit **AssetsFix.csv** with the script editor and add a new line with the asset name and parameters.

Asset lists look like this:

**Name** | **Price** | **Spread** | **Roll  
Long** | **Roll  
Short** | **PIP** | **PIP  
Cost** | **Mgn  
Cost** | **Market** | **Mult** | **Comm  
** | **Symbol** | **_AssetVar..._**  
---|---|---|---|---|---|---|---|---|---|---|---|---  
AUD/USD | 0.77311 | 0.00005 | 0.05 | -0.07 | 0.0001 | 2.2 | -3.0 | 0 | 25000 | 0.5 | * | 1  
EUR/CHF | 1.20104 | 0.00005 | -0.023 | 0.01 | 0.0001 | 2 | -3.0 | 0 | 20000 | 0.5 | EUR.CHF | 2  
EURCHF2 | 1.20012 | 0.00007 | -0.031 | 0.0 | 0.0001 | 0.1 | -1.0 | 0 | 1000 | 0.6 | FXCM_Real:EUR/CHF | 3  
EUR/USD | 1.13795 | 0.00005 | -0.025 | -0.01 | 0.0001 | 1.8 | -1.0 | 0 | 20000 | 0.5 | * | 4  
USD/JPY | 117.489 | 0.004 | -0.04 | 0.0 | 0.01 | 1.7 | -1.0 | 0 | 25000 | 0.5 | * | 5  
SPX500 | 2081.43 | 0.25 | 0 | 0 | 1 | 0.09 | -5.0 | EST:0930-1545 | 0.1 | 0.02 | IBUS500-CFD-SMART |   
US30 | 17703.2 | 3 | 0 | 0 | 1 | 0.09 | -5.0 | EST:0930-1545 | 0.1 | 0.02 | IBUS30-CFD-SMART |   
NAS100 | 4654.34 | 0.5 | 0 | 0 | 1 | 0.9 | -5.0 | EST:0930-1545 | 1 | 0.02 | IBUST100-CFD-SMART |   
UK100 | 6869 | 1 | 0 | 0 | 1 | 0.9 | -5.0 | WET:0930-1600 | 1 | 0.02 | IBUK100-CFD-SMART-GBP |   
GER30 | 10767.8 | 1 | 0 | 0 | 1 | 0.9 | -5.0 | CET:0930-1600 | 1 | 0.02 | IBDE30-CFD-SMART-EUR |   
XAG/USD | 15.863 | 0.013 | 0 | 0 | 0.01 | 0.009 | -5.0 | 0 | 1 | 0.02 | XAGUSD-CFD-SMART |   
XAU/USD | 1156.25 | 0.07 | 0 | 0 | 0.01 | 0.009 | -5.0 | 0 | 1 | 1.6 | XAUUSD-CFD-SMART |   
  
The first line is the header line. A line is ignored when its **Name** begins with **"#"** ; this way assets can be temporarily 'commented out'. Names should contain no special characters except for '_' and '/', but their assigned symbols can contain special characters except for blanks, commas, and semicolons. Zorro also accepts semicolon separated **csv** files with commas instead of decimal points, but not a mix of both in the same file. Excel uses your local separation character, so it is recommended to set up your PC regional settings that it's a comma and the decimal is a point, otherwise Excel cannot read standard csv files. 

The asset parameters are to be set up as described below. In the description, a contract is 10,000 units for forex pairs, and a single unit for all other assets (brokers can have different individual contract sizes, see examples!). A lot is defined as the minimum tradeable number of units for forex pairs, and the minimum tradeable number of units or fraction of a unit for all other assets. Some parameters have a slightly different meaning when  negative, f.i. a percentage instead of an absolute value. For most parameters exist an equivalent script variable, which is set in live trading from the broker API if available, and from the asset list otherwise. 

**Name** | Name of the asset, f.i. **"EUR/USD".** Up to 15 characters, case sensitive, with no blanks and no special characters except for slash '/' and underline '_'. This name appears in the Asset scrollbox and is used in the script, f.i. by the [asset](asset.htm) function. It also determines the names of the historical data files (f.i. **"EURUSD_2020.t6"**). The name is the same for all brokers, while the asset symbol can be different for any broker. The asset is ignored when the name begins with a hash character "**#** ".   
---|---  
**Price** | Ask price per unit, in counter currency units. Accessible with the [InitialPrice](pip.htm) variable. For non-forex assets the counter currency is usually the currency of the exchange place, such as USD for US stocks, or EUR for the DAX (GER30). For digital coins it's usually another coin, such as Bitcoin The **Price** parameter is normally for information only and not used in the backtest.  
**Spread** | The current difference of ask and bid price, in counter currency units. Accessible with the **[Spread](pip.htm)** variable. Used for backtests with constant spread. If at 0, ask/bid spread must be determined by script.  
**RollLong  
RollShort** | Rollover fee (Swap) in account currency units for holding overnight a long or short position per calendar day and per 10000 units for currencies, or per unit for all other assets. Accessible with the **[RollLong/Short](spread.htm)** variables. This is normally the interest that is daily added to or subtracted from the account for holding positions and borrowing margin. Some brokers have a three times higher rollover cost on Wednesdays or Fridays for compensating the weekend. When manually entering rollover fees, make sure to convert them to per-10000 for forex pairs. Zorro multiplies the fees with the trade duration in days for determining the total rollover cost of a trade. Use the [SOFTSWAP](assetmode.htm) flag for continuous swap costs.  
If rollover is unknown, it can be estimated from the current interest rate of the counter currency divided by 365 and multiplied with **asset price*(1-1/leverage)**. If the broker charges a flat interest fee on margin loan, set **RollLong/RollShort** to 0 and use the [Interest](spread.htm) variable.  
**PIP** | Size of 1 pip in counter currency units; accessible with the **[PIP](pip.htm)** variable. Equivalent to the traditional smallest increment of the price. The usual pip size is **0.0001** for forex pairs with a single digit price, **0.01** for stocks and for forex pairs with a price between 10 and 200 (such as USD/JPY), and **1** for CFDs with a 4- or 5-digit price. Prices in the log are normally displayed with as many decimals as given by the pip size.  
**PipCost** | Value of 1 pip profit or loss per lot, in units of the account currency. Accessible with the **[PipCost](pip.htm)** variable and internally used for calculating trade profits or losses. When the asset price rises or falls by **x** , the equivalent profit or loss in account currency is **x * Lots * PIPCost / PIP**. For assets with pip size 1 and one contract per lot, the pip cost is the conversion factor from counter currency to account currency. For calculating it manually,**** multiply **LotAmount** with **PIP** ; if the counter currency is different to the account currency, multiply the result with the counter currency exchange rate. Example 1: AUD/USD on a micro lot EUR account has **PipCost** of **1000 * 0.0001 * 0.9** (current USD price in EUR) = **0.09 EUR**. Example 2: AAPL stock on a USD account has **PipCost** of **1 * 0.01 = 0.01 USD** **= 1 cent**. Example 3: S&P500 E-Mini futures (ES) on a USD account have **PipCost** of **12.5 USD** (1 point = 25 cent price change of the underlying is equivalent to $12.50 profit/loss of an ES contract). This paramter is affected by **LotAmount**.  
**MarginCost** | Margin required for purchasing and holding 1 lot of the asset, either in units of the account currency or in percent of the position value. Depends on account leverage, account currency, counter currency, and **LotAmount**. If the broker has different values for initial, maintenance, and end-of-day margin, use the highest of them. Accessible with the **[MarginCost](pip.htm)** variables. Internally used for the conversion from trade [Margin](lots.htm) to [Lot](lots.htm) amount: The number of lots that can be purchased with a given trade margin is **Margin / MarginCost**. Also affects the **Required Capital** and the **Annual Return** in the [performance report](performance.htm).   
Negative **MarginCost** is interpreted as a percentage of the position value, and internally converted to a leverage accessible with the **[Leverage](pip.htm)** variable. For instance, a 25% end-of-day margin is entered as **-** 25 and equivalent to leverage 4. For no leverage, enter -100.  
**MarginCost** and **Leverage** can be converted to each other with the formula **MarginCost = Asset price / Leverage * PipCost / PIP**. When the broker uses a leverage or margin percentage, the margin per purchased lot depends on the current price of the asset. When the broker uses positive **MarginCost** , the margin is independent of the current asset price, therefore the broker will adapt **MarginCost** from time to time when the price has moved far enough. This is typical for Forex/CFD brokers.   
**Market  
(Leverage)** | Time zone and market hours in the format **ZZZ:HHMM-HHMM** , for instance **EST:0930-1545**. Available to the script in the variables [AssetMarketZone](assetzone.htm), [ AssetMarketStart](date.htm), [AssetMarketEnd](date.htm), for skipping data series or preventing orders outside market hours. Old Zorro versions prior to 2.29 had the leverage in this field.   
**Multiplier /  
LotAmount** | Number of units for 1 lot of the asset; accessible with the **[LotAmount](pip.htm)** variable. Smallest amount that you can buy or sell with your broker without getting the order rejected or an "odd lot size" warning. For forex the LotAmount is normally 1000 on a micro lot account, 10000 on a mini lot account, and 100000 on standard lot accounts. Index CFDs or crypto currencies can have a **LotAmount** less than 1, such as 0.1 CFD contracts. For stocks and most other assets the lot amount is normally 1. The lot amount affects **PipCost** and **MarginCost**.  
Cryptocurrency broker APIs often support the [SET_AMOUNT](brokercommand.htm) command. You can then set any desired lot amount in this field, such as 0.0001 Bitcoins. 10000 Lots are then equivalent to 1 Bitcoin.   
A negative value in this field is interpreted as a multiplier for option or future contracts, and accessible with the [Multiplier](contracts.htm) variable. The **LotAmount** is then set to 1.   
**Commission** | Roundturn commission amount in account currency units for opening and closing one contract, per 10000 units for currencies, per unit for other assets, per underlying unit for options, or as percent of the trade value when negative. Accessible with the [Commission](spread.htm) variable. When manually entering the commission, double it if was single turn. For forex pairs make sure to convert it to per-10000. If an asset is a forex pair can be determined with the [assetType](asset.htm) function. A forex commission in pips is converted by **Commission = CommissionInPips * 10000 * PIP**.  
If the commission is a percentage of the trade value, enter the negative percent value, f.i. -0.25 for 0.25%. A percentage can be converted to commission amount with the formula **Commission = -Percent/100 * Price / LotAmount * PIPCost / PIP**.  
**Symbol** | Broker symbol(s) of the asset. Up to 3 [sources and symbols](symbol.htm) can be entered for trading, receiving live prices, and receiving historical prices, separated by '**!** ' exclamation marks in the form trade!live!history. Leave this field empty or enter '***** ' when the symbol is identical to the asset name or is automatically converted by the broker plugin. Up to 128 characters, case sensitive.  
**AssetVar** | Up to 8 optional [asset specific](algovar.htm) variables or strings, for evaluation in the script through **AssetVar[0]..[7]** or **AssetStr[0]..[7]**. They can contain multipliers, asset types, trading hours, portfolio weights, or similar parameters. Text strings must not exceed 7 characters.  
  
### Remarks:

  * Any live trading session generates a special asset list named **Assets.csv** in the **Log** folder. This list is updated with the actual parameters of all assets used in the trading script. If parameters are not available through the broker API, they are taken from the asset list or replaced by a default value. This is a convenient way to simulate your current broker account: just connect to the broker with a script that selects all needed assets (f.i. the [Download](scripts.htm) script), then copy the new asset parameters from **Assets.csv** to the asset list used by your script.   
!!  Check the content of **Assets.csv** carefully before copying it over. Dependent on broker API and market hours, some live parameters can be way off. For instance, on weekends most assets have an unrealistic spread and no rollover fees, and on Wednesdays or Fridays often the rollover is three times as high for compensating the weekend. If a parameter is temporarily or permanently unavailable from the broker API, it is just a copy from the current asset list. Those parameters are wrong when the current list is from a different broker or an account with different leverage. Some MT4 servers are also known to return wrong parameters since they are wrongly set up.. 
  * If you get bad backtest results after creating an individual asset list, check first the transaction cost - spread. rollover, commission - in the [performance report](performance.htm). The most frequent error in asset list creation is entering wrong rollover or commission parameters that result in extreme transaction costs.
  * Some broker APIs, such as TWS or FIX, provide no asset parameters at all, except for price and spread. The other parameters must be taken from the broker's website and entered manually in the asset list, as in the examples above. If unavailable or hidden, open a position of the asset in a demo account. Commission and margin is then often displayed in the broker's trade platform.
  * If the broker uses a complex formula of fees, margins, and commissions, rather than a flat fee or percentage, you can calculate it in the script. Set the Commission or MarginCost variables accordingly before entering and exiting a trade. 
  * Since prices and interest rates change, **RollLong/Short** , **PipCost** , and fixed **MarginCost** are only valid for the time at which the asset list was created or downloaded. In live trading they are automatically updated from the broker API when available. In the backtest they could be updated by script when required; normally this is not recommended since the backtest is supposed to estimate live performance, rather than replicating historical performance.
  * For backtesting or trading a certain asset, make sure that historical price data files for the tested period are available, and the asset it contained in the selected asset list. For simulating many different accounts, place several **Asset...csv** files in the History folder and call [assetList](script.htm) with the desired asset file name in the script for simulating the corresponding account. 
  * For simulating direct market access (**DMA**) with no broker interference, set the parameters in the following way: **Spread** to the BBO bid/ask spread; **Commission** , **RollLong** , and **RollShort** to**0; MarginCost** to **-1** 00 for no leverage; **LotAmount** to **1** ; and **PipCost** to **PIP** multiplied with the price of the assets or the counter currency in account currency units.
  * If a broker API returns asset prices in cent instead of dollars, **Price** , **Spread** , and **PIP** are 100 times higher than for a dollar-priced asset. Adapt the asset list accordingly and make sure that historical data also contains prices in cent. You can generate cent history with a simple script that imports dollar-based price history with [dataLoad](data.htm), multiplies all prices with **100** , then stores it with [dataSave](data.htm). For options or futures with cent prices, use the [ Centage](contracts.htm) variable.

### Parameter calculation examples (EUR account, EUR/USD at 1.15):

Broker website: Ticker **EURUSD** , Spread 0.3 pips, Margin 3.33%, LotSize 0.01 contracts, ContractSize 100000 EUR, PipValue 10 USD, SwapLong -5.70 USD, SwapShort -2.50 USD, OrderCommission 2.5 EUR. All parameters per 100000 EUR contract.  
  
=> Asset list: Name **EUR/USD** , Price 1.15, Spread 0.00003 (= 0.3 pips * PIP), RollLong -0.4956 (= -5.70 * 10000/100000 / 1.15), RollShort -0.2174 (= -2.50 * 10000/100000 / 1.15), PIP 0.0001 (= 10 / 100000), PIPCost 0.087 (PIP * LotAmount / 1.15), MarginCost -3.33, LotAmount 1000 (= 0.01 * 100000), Commission 0.5 (= 2 * 2.5 * 10000/100000), Symbol EURUSD.

Broker website: Ticker **USDJPY** , Spread 0.4 pips, Leverage 30, PIP 0.01, PipValue/Lot 1000 JPY, MinLot 0.01. Variable swap, no commission.

=> Asset list: Name **USD/JPY** , Price 105, Spread 0.004 (= 0.4 pips * PIP), RollLong/Short -0.01 (some assumed value), PIP 0.01, PIPCost 0.083 (PipValue * MinLot / Price / 1.15), MarginCost -3.33 (-100/Leverage), LotAmount 1000 (PipValue / PIP), Commission 0, Symbol USDJPY.

Broker website: Ticker **XAGUSD** , Price 17.5, Spread 0.02 points, Margin 10%, LotSize 0.01 contracts, ContractSize 5000 XAG, TickValue 5 USD, SwapLong -3.50 USD, SwapShort -3.00 USD, OrderCommission 0.0025%. All parameters per 5000 XAG contract.**  
**  
=> Asset list: Name **XAG/USD** , Price 17.5, Spread 0.02, RollLong -0.00061 (= -3.50 / 5000 / 1.15), RollShort -0.00052 (= -3.00 / 5000 / 1.15), PIP 0.01 (by tradition), PIPCost 0.435 (PIP * LotAmount / 1.15), MarginCost -10, LotAmount 50 (= 0.01 * 5000), Commission -0.005 (= -2 * 0.0025%), Symbol XAGUSD.

Broker website: Ticker **DE30** , Price 12567, Pip size 1, Avg Spread 0.5 pips, Leverage 1:100, LotSize 0.01 contracts, TickValue 1 EUR, SwapLong -3.0 %, SwapShort -6.0 %, Commission 5 / mio. **  
**  
=> Asset list: Name **GER30** , Price 12567, Spread 0.5, RollLong -1.03 (= -3,0% / 100% * Price / 365), RollShort -2.06 (= -6.0% / 100% * Price / 365), PIP 1, PIPCost 1, MarginCost -1.0 (-100 / Leverage), LotAmount 0.01, Commission -0.00005 (= 5 / 1000000 * 100%), Symbol .DE30.

### Included asset lists

No guarantee of completeness or correctness! The lists below can be used as examples or templates for your own account specific asset list.

**AssetsFix.csv** \- The default asset list when no different list is specified in the script; determines the 15 assets initially available in the scrollbox. Based on an Oanda™ 100:1 USD account with lot size 1000.

**AssetsForex.csv, AssetsForexEUR.csv** \- 35 currency pairs, including all pairs of the 7 main currencies EUR, USD, AUD, GBP, CHF, NZD, CAD. For trading with a multitude of currencies. Based on a typical Forex account with 100:1 leverage (**AssetForex**) or 30:1 leverage (**AssetsForexEUR**). Commission is, as usual, included in the spread. 

**AssetsFUT.csv** \- 20 frequently used futures contracts of currencies, commodities, and indexes. Use the [ contractSetSymbol](contract.htm) function to adapt the symbols to the desired expiration date.

**AssetsArb.csv** \- 3 Forex pairs from different brokers as an example of broker arbitrage.

**AssetsOanda.csv*, AssetsOandaEUR.csv*** \- 15 Forex/CFD assets on an Oanda 100:1 USD or 20:1 EUR account with lot size 1.

**AssetsFXCM.csv*** \- 15 Forex/CFD assets on a FXCM 30:1 EUR account.

**AssetsGP.csv*** \- 15 Forex/CFD assets on a Global Prime 100:1 USD account.

**AssetsDarwinex.csv*** \- 15 Forex/CFD assets on a Darwinex 100:1 EUR account.

**AssetsIB.csv*** \- Forex pairs, CFDs, and 40 major stocks on an Interactive Brokers™ USD account.

**AssetsBittrex.csv*** \- Example cryptocurrencies on a Bittrex Bitcoin account. 

**AssetsSP30.csv*** \- Top 30 stocks from the S&P500 index, for downloading historical prices from the broker.

**AssetsSP50.csv*** \- Top 50 stocks from the S&P500 index, with symbols for downloading historical prices from Stooq or Yahoo.

**AssetsSP250.csv*** \- Top 250 stocks from the S&P500 index. Plain list, no symbols or parameters.

**AssetsRussell2000.csv*** \- 2000 small cap stocks from the Russell 2000 index, prices from Yahoo.

**AssetsCFD200.csv** \- 200 stock CFDs with 10:1 leverage for trading with IB.

**AssetsZ8.csv** \- 20 stocks and ETFs for a Markowitz portfolio rotation strategy, prices from Stooq, trading with IB.

**AssetsZ9.csv** \- 10 US sector ETFs and bonds for a momentum portfolio rotation strategy, prices from Stooq, trading with IB.

**AssetsZ9E.csv** \- 10 European sector ETFs and bonds for a momentum portfolio rotation strategy, prices from Yahoo, trading with IB.

**AssetsZ10.csv** \- 10 major cryptocurrencies, prices from Cryptocompare, trading with Binance.

*  !!  Broker-specific asset lists do not necessarily match your account, even with the same broker. Asset parameters and trading costs are often different in different countries and change all the time. We do not permanently observe brokers or stock indices lists for updating the above asset lists. For making sure that your asset list is correct, get the recent parameters from the broker API or the broker's website, compare with your asset list, and adapt it if necessary.  

# The account list  (Accounts.csv)

Account lists are supported with [Zorro S](restrictions.htm). By default, only a Demo and a Real account is available with the [Account] scrollbox. Individual accounts, with user-defined parameters and separate brokers and price sources, can be added with an account list. It is stored in a spreadsheet file named **Accounts.csv** in the **History** folder. This file is a convenient way to manage many broker accounts with different logins, passwords, asset parameters, and special modes. The **Accounts.csv** file can be edited with Excel or with the script editor. Zorro must be restarted when the file was modified. It contains the account parameters in comma separated spreadsheet format.

Account list example:

**Name** | **Server** | **AccountId** | **User** | **Pass** | **Assets** | **CCY** | **Real** | **NFA** | **Plugin**  
---|---|---|---|---|---|---|---|---|---  
MT4_Generic | MT4 | 0 | 0 | 0 | AssetsFix | USD | 0 | 0 | ZorroMT4.dll  
FXCM_Demo | FXCM | 0 | D23456789 | 1234 | AssetsFix | EUR.€ | 0 | 0 | FXCM  
FXCM_Real | FXCM | 0 | 276345231 | 5678 | AssetsFXCM | EUR.€ | 1 | 0 | FXCM  
FXCM_RO | FXCM | 0 | 276345231 | 5678 | AssetsFXCM | EUR.€ | 3 | 0 | FXCM  
IB_TWS_1 | IB | D456378 | 101 | 0 | AssetsIB | EUR | 1 | 14 | IB  
IB_Gateway | IB | U2319821 | 1 | 0 | AssetsIB | EUR | 1 | 14 | IB  
Oanda_V20 | Oanda | 0 | 304-007... | 5c3dc12d... | AssetsOanda | EUR | 1 | 0 | Oanda.dll  
NxCore | NxCore | 0 | 0 | 0 | AssetsSP500 | USD | 0 | 0 | NxCore.dll  
Crypto | BitShark | BTC | 0 | 0 | AssetsCrypt | BTC.B8 | 1 | 0 | Btrx.dll  
ABC_Demo | ABC | 0 | 4009876 | n4qw4ert7 | AssetsABC | CHF | 0 | 0 | ZorroMT41.dll  
ABC_Real | ABC | 0 | 3456789 | ab234qrz2 | AssetsABC | CHF | 1 | 0 | ZorroMT42.dll  
  
An example file **AccountsExample.csv** is contained in the **History** folder as a template for creating your own **Accounts.csv** file. Every account in the scrollbox corresponds to a line in the file with the following parameters, separated with commas:

**Name** | Account name (no blanks) that appears in the account scrollbox. A line beginning with '#' is outcommented.  
---|---  
**Server** | Broker name, server URL, or some other info identifying the connection (no blanks). Sent to the API plugin with the [ SET_SERVER](brokercommand.htm) command before login.   
**AccountId** | Account/subaccount identifier sent to the broker, or **0** when no subaccounts are used.  
**User** | User ID for the login, or **0** for manually entering the user name.  
**Pass** | Password for the login, or **0** for manually entering the password.  
**Assets** | Name of the default asset list (see above) for this account, with no blanks and without the **.csv** extension. This is the list used for live trading and backtesting when the account is selected and no [assetList](asset.htm) function is called in the script.  
**CCY** | Name of the account currency, f.i. EUR or USD. Sent to the API plugin with the [SET_CCY](brokercommand.htm) command before any account request. Optionally an ANSI currency symbol and the number of digits to display for account values can be added after a decimal point. By default, the currency symbol is '$' and account values are displayed with 2 decimals when less than 100, and without decimals from 100 on.  
**Real** | **0** for a demo account, **1** for a real account, or **3** for a real account with no trading. In 'no trading' mode all trades are opened as [phantom trades](lots.htm) and not sent to the broker.   
**NFA** | Compliance of the account. Affects default state of [NFA](mode.htm#nfa) flag and [Hedge](hedge.htm) mode: **0** for no restrictions, **2** for **Hedge = 0** (no hedging),**14** or **15** for **NFA = on** (full NFA compliance).  
**Plugin** | File name of the broker plugin in the **Plugin** folder, case sensitive (**.dll** extension can be omitted). The same plugin can used for several price sources by copying the plugin to a different name (see below). In the example, **ZorroMT41.dll** and **ZorroMT42.dll** are both copies of the **ZorroMT4.dll**. If the field is empty, the broker plugin is selected with the scrollbox.  
  
The first line in the CSV file must be the header line. Names and strings must contain no blanks (check for blanks at the end that may be accidentally added!) and no special characters except for **'-'** and **'_'**. Zorro also accepts semicolon separated **csv** files with commas instead of decimal point, but not a mix of both in the same file. User names and passwords are stored in unencrypted text in the spreadsheet, so leave those parameters at 0 when other people have access to your PC.

In the above example, the account **ABC_Real** is prepared for trading simultaneously with **ABC_Demo** , one used as price source, the other for entering trades by setting **Asset List / Symbol** accordingly (see above). Since two broker connections cannot use simultaneously the same plugin, the Zorro user with the above account list has made two copies of the **ZorroMT4.dll** , and renamed them to **ZorroMT41.dll** and **ZorroMT42.dll**. The connected MTR4 client must also be duplicated this way, since MTR4 can also not handle two different accounts simultaneously.

When the **AssetsFix.csv** or **Accounts.csv** lists were changed, Zorro must be restarted for the changes to have an effect.

### Remarks

  * Be careful when manually opening and saving CSV files with Excel. Dates, times, delimiters and decimal points are then possibly saved in your local format, which can cause the file to become unreadable by Zorro. 

### See also:

[ assetAdd](asset.htm), [assetList](asset.htm), [symbols](symbol.htm), [broker arbitrage](brokerarb.htm), [TradeMode](trademode.htm), [ Testing](testing.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
