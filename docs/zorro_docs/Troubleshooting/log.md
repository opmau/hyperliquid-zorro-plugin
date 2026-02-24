---
title: Log mesages
url: https://zorro-project.com/manual/en/log.htm
category: Troubleshooting
subcategory: None
related_pages:
- [Verbose](verbose.htm)
- [printf, print, msg](printf.htm)
- [set, is](mode.htm)
- [Multiple cycles](numtotalcycles.htm)
- [TrainMode](opt.htm)
- [algo](algo.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Error Messages](errors.htm)
- [loadStatus, saveStatus](loadstatus.htm)
- [Time zones](assetzone.htm)
- [bar](bar.htm)
- [Asset and Account Lists](account.htm)
- [Equity Curve Trading](trademode.htm)
- [Virtual Hedging](hedge.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Licenses](restrictions.htm)
- [contract, ...](contract.htm)
- [CBI](ddscale.htm)
- [BarMode](barmode.htm)
- [Dates, holidays, market](date.htm)
- [Status flags](is.htm)
- [Bars and Candles](bars.htm)
- [Testing](testing.htm)
- [Live Trading](trading.htm)
- [Export](export.htm)
- [Performance Report](performance.htm)
---

# Log mesages

# Log messages

All events, such as opening or closing a trade, are recorded in the **.log** file in the **Log** folder. This file can be opened with any text editor; it normally opens in Notepad++ when clicking [Result] after a backtest. The 'verbosity' of log entries can be set up with the [Verbose](verbose.htm) variable. Additional messages can be inserted in the log by the strategy script using the [print](printf.htm) functions. 

The [LOGFILE](mode.htm) flag must be set for recording a log. For comparing or combining logs from different sessions, append a number to the file name by setting the [LogNumber](numtotalcycles.htm) variable right at begin of the **main** or **run** function. Then compare both logs with a tool like BeyondCompare™. For generating a different log file name on any day, set **LogNumber = wdate(NOW)**. For logging a particular training cycle, use the [LogTrainRun](opt.htm) variable.

Any bar is recorded in the log with its number, UTC time stamp, current balance or equity, and the current price of the selected asset.

Any trade is identified by asset, algo, type, and a unique number, like this:

**[AUD/USD:CY:S1234]** \- Short trade (**S**) with **AUD/USD** , **CY** [algorithm identifier](algo.htm), and a trade ID number ending with **1234**. The ID can be used to identify the trade in the broker's platform. If a trade is partially closed, the ID can change. Trade types are short **S** , long **L** , put **P** , or call **C**. 

**[SPY::LP0103]** \- Long put (**LP**) with **SPY** , no algorithm identifier, and ID ending with **0103**.

**{AUD/USD:CY:s1234}** \- Short phantom trade (lowercase **s** and winged brackets). [Phantom](lots.htm) trades are simulated, not executed.

**(AUD/USD::L)** \- Long pending trade (round parentheses). Pending trades are not yet opened and thus have no trade number.

The decimals of prices or other numbers in the log vary. The higher the value, the less decimals are normally printed.

### Startup / price download messages 

The following messages are generated at start:

**Load AUD/USD.. 1000 h**

Zorro downloaded 1000 hours recent price history from the broker's server and used it for the [LookBack](lookback.htm) period. If an asset is not available for trading - which can happen with assets that US residents are not allowed to trade, f.i. some CFDs - an error message is issued and the asset must be excluded from the strategy.

**Load AUD/USD.. 910+111 h**

The [PRELOAD](mode.htm) flag was set and historical price data was available. Zorro read 910 hours price data from its history files and downloaded additional 111 hours price history from the broker's server. Dependent on broker history, the number of downloaded hours can differ from asset to asset. 

**Load AUD/USD.. 1000 h, gap 36 h**

There is a 36 hours gap between the end of the downlaoded price data and the current time. This happens when a the price download was restricted by the broker, or when the price server was not up to date due to a recent weekend or holiday, or when the market was closed at session start. Small gaps are usually not harmful; large gaps can, dependent on strategy and gap size. 

**Load AUD/USD.. 835 h, 65 bars added**

The price server could not cover the full **LookBack** period in time; 65 bars had to be substituted by Zorro. This can happen with [MT4 brokers](mt4plugin.htm) or other brokers iwht klimited history. MT4 servers sometimes get the missing data after a waiting time; in that case the strategy just needs to be restarted after a minute. Added bars are normally not harmful, as missing prices are interpolated. But when a large number of bars has been added, indicators with long time periods can be less accurate at strategy start and cause reduced performance during the first time. Use the [PRELOAD](mode.htm) flag for overcoming price server limitations. 

**Load AUD/USD.. failed**

The asset is available, but price data can not be downloaded. This can happen with MTR4 servers that need a long time for sending assets that have not been requested before (f.i. because no chart with that asset was opened in the MTR4 client). In such a case, start the strategy again after a minute - chances are that the MTR4 server meanwhile got its act together.

**AUD/USD 2019 complete**

The current historical data is already up to date. Its file date is more recent than the most recent price quote minus 1 day. No data is downloaded.

**[AUD/USD:CY:L4401] - resumed**

Zorro is resuming a previously closed or interrupted trading session and continues the trades that were opened in the previous session. Previous trades are not resumed when the new session opened new trades already in the first run. 

**[AUD/USD:CY:L4401] - (closed)**

The trade cannot be resumed. Either an asset named "AUD/USD" was not available at session start (see [Error 053](errors.htm)), or the trade was externally closed through a 'safety net' stop loss or manually in the broker platform. 

**Session statistics resumed - 27 bars passed**

The previous session was closed 27 bars ago. Session statistics have been saved with the [SAV_STATS](loadstatus.htm) flag and is resumed with the current session. The end of the previous session was still within the lookback period of the current session.

**Read Trend.par Trend.c Trend.fac**

The parameters, rules, and capital allocation factors of the **Trend** strategy were loaded. This message is generated at start and again when the strategy was re-trained (see below).

**LookBack 123 bars, 20-11-19 00:00..21-05-19 00:00**

Start of the lookback period

**Bar 123 continued until 20:00**

Bar 123 started in the lookback period, but continues in the life period.

**End of lookback period at 19:00**

All data series are filled and trading is now enabled.

### Bar messages

**[21: Wed 11-01-19 00:00] (95.18)**

Bar number and bar close date and time, either in UTC or in the time zone given by [LogZone](assetzone.htm), rounded to one second. Optional ask price at bar end of the asset selected with the scrollbox. The number of decimals of the price depends on the **PIP** value of the asset. 

**[217: Wed 11-01-19 01:15:00] 91.41/96.02\91.35/95.90**

Bar number, close time, and optional open/high\low/close ask prices of the asset selected with the scrollbox, on [Verbose](verbose.htm)**= 2**. If none of the assets used in the script is selected with the scrollbox, no prices are printed.

**[217: Wed 11-01-19 17:15:00c] 91.41/96.02\91.35/95.90 -0.15**

As before, but with current ask/bid spread with [Verbose](verbose.htm)**= 3**. A **"c"** or **"w"** next to the time indicates a bar outside market hours or at weekend, **"f"** indicates a flat bar where the price did not change, and **"z"** indicates local time by [LogZone](assetzone.htm).The spread is displayed as a negative value to be subtracted from the ask prices for getting the bid prices.

**[217: Wed 11-01-19 17:15:00c] 90.00/95.00\85.00/95.00 (94.90) -0.15**

As before, but OHLC prices from a [special bar](bar.htm), the current real price in parentheses, and the current ask/bid spread, with [Verbose](verbose.htm)**= 3**. 

**[1385: Fri 12-06-29 00:00] 9038 +1438 9/7 (97.69)**

Bar number, rounded bar end time, current balance or equity (dependent on [BALANCE](mode.htm) flag), current open value, number of open winning/losing positions, and current price (when trades have been opened). The decimals of the equity and open value numbers depend on the decimals setting in the [account list](account.htm).

### Trade messages

The following messages are generated when trades are entered or exited. Trade symbols are in curly brackets **{ }** for [ phantom](trademode.htm) and [virtual](hedge.htm) trades, in square brackets **[ ]** for trades sent to the broker, and in parentheses **( )** for trades that were not opened or are still pending. Long or short trades are indicated by a capital **S** or **L** for virtual or pool trades, and a small **s** or **l** for phantom trades. Prices and profits displayed are based on the last price quote. The number of decimals depends on the amount and can be enforced by an [account list](account.htm) entry. Prices in the log are generally displayed as ask prices, even though long positions open at the ask and close at the bid, and short positions open at the bid and close at the ask. Profits include estimated trading costs. In live trading, real fill prices and profits can slightly deviate due to slippage and differences in trading costs. 

**(AUD/USD:CY:S) Skipped - Lots/Margin 0**

The trade was not entered because [Lots](lots.htm) or [Margin](lots.htm) was set to **0**.

**(AUD/USD:CY:S) Skipped - Margin 10, Min 45**

The trade was not entered because [MARGINLIMIT](mode.htm) is set and the allocated [Margin](lots.htm) of 10 account currency units was too low for buying 1 lot of the asset, which would require margin **45**. If the [ACCUMULATE](mode.htm) flag is set, the margin of those skipped trades will be accumulated until a trade is eventually entered.

**(AUD/USD:CY:S) Skipped - Risk 40, Min 90**

The trade was not entered because the allowed [Risk](lots.htm) of **40** (in account currency units) was too low to cover the trade risk of **90** (see [RISKLIMIT](mode.htm)). 

**(AUD/USD:CY:S) Skipped - Total Margin 450**

The trade was not entered because [MARGINLIMIT](mode.htm) is set and the account has not enough free margin to cover the [Risk](lots.htm) of the trade in a safe distance from a margin call. Either reduce the trade volume, or close open trades, or add some money to the account. You can see the current free margin in the broker platform, f.i. in the Account status of the FXCM Trading Station. 

**(AUD/USD:CY:S) Skipped - Total Risk 900**

The trade was not entered because there's not enough balance in the account to safely cover the total risk of all open trades (see [RISKLIMIT](mode.htm)). 

**(AUD/USD:CY:S) Skipped (not enough capital)**

Only in a [verbose](verbose.htm) log. The trade was not entered due to insufficient funds. 

**(AUD/USD:CY:S) Skipped (frame)**

Only in a [verbose](verbose.htm) log. The trade was not entered because trading is disabled inside a [TimeFrame](barperiod.htm).

**(AUD/USD:CY:S) Skipped (market hours)**

Only in a [verbose](verbose.htm) log. The trade was not entered because trading was disabled during weekends or holidays

**(AUD/USD:CY:S) Skipped (Max / Amount)**

Only in a [verbose](verbose.htm) log. The trade was not entered because it would exceed the allowed number of open or pending trades of this type or algo.

**(AUD/USD:CY:S) Skipped (in-sample)**

Only in a [verbose](verbose.htm) log. The trade was not entered because trading was disabled due to [DataSkip](dataslope.htm), [DataSplit](dataslope.htm), or [DataHorizon](dataslope.htm).

**(AUD/USD:CY:S) Skipped (no trading)**

Only in a [verbose](verbose.htm) log. The trade was not entered because there is no trading in the [LookBack](lookback.htm) period. The period can be extended beyond the normal lookback end when a particular asset had otherwise not enough price history.  

**(AUD/USD:CY:S) Short 3@0.87400 Entry stop**

A pending trade was opened, with an entry stop at **0.87400**. If the price drops to the entry stop within [EntryTime](timewait.htm), **3** [lots](lots.htm) will be bought at **0.87400** plus slippage. Note that pending trades are not sent to the broker. 

**(AUD/USD:CY:S) Short 3@0.87600 Entry limit**

A pending trade was opened, with an entry limit at **0.87600**. If the ask price rises to the entry limit within [EntryTime](timewait.htm), **3** lots will be bought at **0.87600** plus slippage. Note that pending trades are not sent to the broker.

**(AUD/USD:CY:S)** **Missed entry 0.87400 after 2 bars**

The pending trade was not executed because the [entry](stop.htm) price was not met within the allowed time period ([EntryTime](timewait.htm)). This message is not displayed in the window, but in the log file.

**(AUD/USD:CY:S)** **Missed entry 0.87400 at cancellation**

The pending trade was not executed because the [entry](stop.htm) price was not met until the next [exit](selllong.htm) command. This message is not displayed in the window, but in the log file.

**(AUD/USD:CY:S)** **Order expired after 48 hours**

The trade was not executed because the order was entered just before the market was closed at a weekend or holiday, and [EntryTime](timewait.htm) was not long enough to cover that period. This message is not displayed in the window, but in the log file.

**(AUD/USD:CY:S)** **Balance limit exceeded**

The trade was not executed because you're already too rich. The [balance limit](restrictions.htm) for trading with the free Zorro version on a real money account was exceeded. You can either get [Zorro S](restrictions.htm), or withdraw your profits until your account balance is again below the limit.

**[AUD/USD:CY:S4400] Short 1@0.87310 Bid 0.87305 Risk 116 ptlsx**

A short trade was entered. The number **4400** are the last 4 digits of the trade ID that was assigned by the broker API. The trade volume was **1** lot, the current ask quote was **0.87310** , the current bid quote was **0.87305**.(displayed at [Verbose](verbose.htm)**= 2** or above) The loss of this trade at the stop level would be approximately **116** account currency units. This can deviate from a set-up [Risk](lots.htm) value when only a few lots are opened or when the entry price is different from the current price. Special exit conditions or trade modes are displayed with lowercase letters: "**p** " for a profit target, "**t** " for trailing, "**l** " for a profit lock, "**s** " for a fixed trailing step, "**x** " for a fixed exit time, "**o** " for a limit order, and "**g** " for a GTC order. In the log file the entry time is also listed.

**[AUD/USD:CY:S4400] Short %2@0.87310 of 5 Risk 116 g**

A short trade was entered as above, with no exit conditions, but as a GTC order ("**g** "). Only 2 ("**%2** ") of 5 lots were initially filled. The remaining 3 lots could be filled later.

**{AUD/USD:CY:l4500} Long 3@0.87310 Risk 200**

A long [phantom trade](lots.htm) \- mind the winged brackets and the lowercase **l** \- was entered at market. **3** imaginary lots were bought at the price **0.87310**. 

**[AUD/USD:CY:l4500] Long 3@0.87310 Pool trade**

A pool trade in [virtual hedging mode](hedge.htm) was entered at market in reaction on the opening or closing of phantom trades. **3** lots were bought at price **0.87310**. 

**[AUD/USD:CY:S4400] Cover 1@0.85934: +105 at 17:01**

A short trade was closed by an [exitShort](selllong.htm) call. **1** lot was covered at **17:01** UTC at current price **0.85934** and profit of **105** account currency units. Since displayed profit and exit price are based on the last price quote and estimated trading costs, the real profit or loss of the trade can slightly deviate from the displayed values, and is visible in the broker platform. 

**[AUD/USD:CY:L4501] Sell %1@0.85945: +33 at 17:01**

A long trade was partially closed by an [exitLong](selllong.htm) call. **1** lot was sold at **17:01** UTC at exit price **0.85945** and profit **33**. The rest of the position remains open. The trade ID usually changes by partially closing, and the displayed ID and all further messages of that trade are for the remaining open position. 

**[AUD/USD:CY:L4401] Reverse 1@0.85945: +33 at 17:01**

A long trade was closed by opening a short trade in opposite direction. 

**[AUD/USD:CY:L4451] Expired 1@0.85945: +33 at 17:01**

A long trade was automatically closed because its [ LifeTime](timewait.htm) expired. 

**[AUD/USD:CY:L4444] Liquidated 50@0.8567: -1044 at 17:01**

A long trade was liquidated due to a margin call by spending all [Capital](lots.htm).

**[AUD/USD:CY:S4400] Exit 1@0.85934: +105 at 17:01**

A trade was closed by the trade management function. **1** lot was sold at the price **0.85934**. The profit was **105**.

**[AUD/USD:CY:S2210] Closed 1@0.80201: +20.5 at 17:01**

The trade was manually closed in the broker platform. **1** lot was sold at the price **0.80201**. The profit was **20.5**.

**[AUD/USD:CY:L1200] Stop 1@0.78713: -49.30 at 17:25**

The trade was stopped out at **17:25** UTC. **1** lot was sold at the price **0.78713**. The loss was **49.30**.

**[AUD/USD:CY:L1200] TP 1@0.7971: +55.70 at 17:25**

The trade hit the profit target at **17:25** UTC. **1** lot was sold at the price **0.7971**. The profit was **55.70**.

**[AUD/USD:CY:S4400] Trail 1@0.79414 Stop 0.80012**

The stop loss moved to a different position, either diretly by script, or by trailing, or by setting an [exit price limit](selllong.htm), or by updating open trades when entering at a [MaxLong/Short](lots.htm) position limit. The new stop is **0.80012**. In [TICKS](mode.htm#ticks) mode and in life trading, a trade can trail several times within a bar. A trail message is only printed when the stop loss has moved by more than 2 pips, therefore the real stop limit at the end of the bar be different than the last printed value. Smaller **PIP** values produce more trail messages.

### Trade messages - futures, options, and combos

The following messages are generated when option or future contracts are entered, closed, or exercised. Note that displayed underlying prices are taken from the options history when available, and thus can sometimes slightly deviate from the unadjusted asset prices taken from the price history. Displayed premiums, unlike underlying prices that are always the ask price, do include the ask-bid spread.

**[SPY:COT:LP0103] Buy 1 Put 20170118 210.0 100@1.16 Val 10 at 20:00**

Algo **COT** buys a **SPY** put option with expiration at **January 18, 2017** and strike price **210** at **$116** premium (**100 * 1.16**). The **Val** element of the contract was **10**.

**[SPY:COT:SC0104] Write 1 Call 20170118 215.0 100@1.09 at 20:00**

Algo **COT** sells short a **SPY** call option with expiration at **January 18, 2017** and strike price **215** for **$109** premium. 

**[SPY::SC0104] Place 1 Call 20170118 215.0 100@1.09 at 20:00**

A **SPY** combo order places a short call (**SC**) with expiration at **January 18, 2017** and strike price **215** for **$109** premium. Only the last leg of the combo order really opens the position.

**[SPY::LC0104] Buy 1 Call 20170118 215.0 %0@1.09 at 20:00**

A **SPY** long call (**LC**) with expiration at **January 18, 2017** and strike price **215** for **$109** premium was ordered. The position was initially not filled (**%0**), but can be filled later when it was a [GTC order](trademode.htm).

**[SPY::LP6633] Exercise 1 Put 212.0 100@209.84: +124$ at 19:00**

Exercise a **SPY** put option with a strike price **212** at an underlying ask price of **209.84** , and sell the underlying at market for a total win of **$124**. 

**[SPY::LC6634] Sell 1 Call 210.0 100@1.46: -22.00 at 19:00**

Sell back a long **SPY** call option at **$1.46** , for a total loss of **$22**. 

**[SPY::SC6634] Cover 1 Call 210.0 100@1.46: +22.00 at 19:00**

Buy back a short **SPY** call option at **$1.46** , for a total win of **$22**.

**[SPY::LP5727] Expired OTM 1 Put 207.0 100@0.0: -190$ at 19:00**

A long **SPY** put option expired out of the money. The **$190** premium was lost. 

**[SPY::LC5131] Lapsed 1 Call 207.0 100@150.00: +50$ at 19:00**

A long **SPY** call option position was found by [contractCheck()](contract.htm) to be externally closed on the account, by expiration, manual closing, being exercised, or similar events. A profit of **$50** was assumed, based on the premium and the current underlying price **150**.

**[SPY::LC5728] Expired ITM 1 Call 207.0 100@209.50: +112$ at 19:00**

A long **SPY** call option expired in the money at current underlying ask price **209.50**. The underlying was sold at market. The total profit, minus the paid premium, was **$112**. 

### Other messages 

**Thursday 24.1.2012 --- Profit $880 ---**

The daily profit or loss in relation to the previous day (if [Verbose](verbose.htm) >= **2**).

**[AUD/USD:CY:S4400] +116$ s0.8733** **c0.8729 e0.8731**

Daily list of all currently open trades (if [Verbose](verbose.htm) >= **3**); also displayed on the status page. The trade made 116$ profit, based on a stop price of 0.8733, current price of 0.8729, and entry price of 0.8731. 

**Current DD: 2100$ (65%) in 11 days, CBI 23%**

If [Verbose](verbose.htm) >= **2:** Depth and duration of the last or current equity drawdown in percent of the preceding equity peak; also displayed on the status page. **[CBI](ddscale.htm)** is the probability of the current drawdown based on the [PnL curve](export.htm#pnl) from the last backtest. Note that the CBI is only valid when the backtest used the same trade volume as the live trading session, and no trades from a previous session have been resumed.

**Market closed on 02.01.2017 23:00**

It's weekend or holiday, either set up by the script or detected by the broker plugin. Zorro takes a break and suspends trading until the market opens again, which triggers a **Market open...** message at bar start. Weekends usually end on Sunday night GMT when the Sydney stock market opens. See [BarMode](barmode.htm), [StartWeek](date.htm), [EndWeek](date.htm). 

**Liquidation / Margin call in 2016 (Equity 12800$ Margin 13500$)**

The current margin requirement exceeds the available account capital in the backtest (when the [Capital](lots.htm) variable is set). The trade is liquidated at market and the [MARGINCALL](is.htm) status flag is set. 

**!ZDAS Exception - there is no tradeable price  
[AUD/USD:CY:L] Can't open 1@0.82345 at 21:05:30 **

Messages that begin with an exclamation mark **("!")** are information, errors, warnings, diagnostics, or other messages from the broker API (see [Error Messages](errors.htm)). A frequent message is about not being able to open or close a trade because no price quote is currently available or trading is temporarily suspended (**"Instrument trading halted",** **"There is no tradeable price** " etc.). This happens especially when assets are traded outside business hours. If a trade can not be closed, Zorro will attempt in regular intervals to close it again. 

**Error XXX ....  
Warning XXX ...**

Error messages normally indicate a bug in the script or a problem with the historical data. Warning messages indicate a potential problem. For details see [Error Messages](errors.htm).

### See also:

[Bar](bars.htm), [testing](testing.htm), [trading](trading.htm), [file formats](export.htm), [error messages](errors.htm), [performance report](performance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
