---
title: Stop, Profit, Trail, Entry
url: https://zorro-project.com/manual/en/stop.htm
category: Functions
subcategory: None
related_pages:
- [LifeTime, EntryTime, ...](timewait.htm)
- [Equity Curve Trading](trademode.htm)
- [combo, ...](combo.htm)
- [round, roundto](round.htm)
- [tick, tock](tick.htm)
- [trade management](trade.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Broker API](brokerplugin.htm)
- [Virtual Hedging](hedge.htm)
- [brokerCommand](brokercommand.htm)
- [enterLong, enterShort](buylong.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [contract, ...](contract.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [set, is](mode.htm)
- [orderCVD](ordercvd.htm)
- [Spread, Commission, ...](spread.htm)
- [Indicators](ta.htm)
- [Fill modes](fill.htm)
- [Links & Books](links.htm)
- [Bars and Candles](bars.htm)
---

# Stop, Profit, Trail, Entry

# Trade parameters 1 - entry and exit limits 

## Entry

Entry limit, entry stop: Enter the subsequent trade when the price reaches the given limit or moves by the given distance (default = **0** = enter immediately at market). The value can be positive or negative, and can be either directly an ask price level, or a distance to the current ask price. A positive price level or positive distance constitutes an **entry stop** , a negative price level or negative distance constitutes an **entry limit**.   
An entry limit buys when the price moves against the trade direction and touches or crosses the limit. It increases the profit because it buys at a price that went in opposite direction to the trade. An entry stop buys when the price moves in trade direction and reaches or crosses the limit; it reduces the profit, but enters only when the price is moving in favorable direction, and thus acts as an additional trade filter. For a long trade, an entry limit should be below and an entry stop should be above the current price. If the entry price is not reached within the allowed time period (set through [EntryTime](timewait.htm)), the trade is cancelled and a "Missed Entry" message is printed to the log file. 

## OrderLimit

Use a limit order at the given limit price, instead of a market order, for subsequent trades in live trading. Has only an effect if supported by the broker API; has no effect in training or backtests. **OrderLimit** is different to an **Entry** limit because it is handled on the broker's side (see remarks) and is - if supported by the API - also valid for exits. If not sent in [GTC](trademode.htm) mode, the order is automatically cancelled if not filled within the **wait time** set for the broker API.   
The price limit is directly used for the order and thus must be an ask price for **enterLong** and a bid price (ask minus **Spread**) for **enterShort**. For a [combo](combo.htm) order it must be the sum of the ask and bid limits of all legs. The script must [round](round.htm) the limit to an integer number of pips if required by the broker API. Unlike normal orders, not filling a limit order does not produce a "Can't open trade" error message in the log. For avoiding that it affects subsequent entries and exits, set **OrderLimit** back to **0** after entering. A [tick](tick.htm) or [tmf](trade.htm) function can be used to resend orders with different **OrderLimit** values for catching the best price within a certain time period. 

## Stop

Stop loss: Close the subsequent trade.when the price moved in adverse direction to the given limit or by the given difference to the fill price (default = **0** = no stop loss). When a new trade cannot be entered due to the [MaxLong/MaxShort](lots.htm) limit, open trades of the same component are automatically updated to the stop that the new trade would have. A good value for **Stop** is derived from the [ATR](ta.htm#atr), f.i. **3*ATR(20)**. Setting a stop loss is recommended for [risk control](lots.htm). 

## StopFactor

Stop loss distance sent to the broker API to act as a 'safety net' in units of the real stop loss distance (default = **0** = don't send stop). At the value **1.5** the stop sent to the broker is 50% more distant than the stop on the Zorro side. This allows to stop out the trade even when the broker connection breaks down, but hides the real stop value from the broker and thus prevents 'stop hunting' or similar practices. At **0** the stop is only controlled on the Zorro side, at **1** it is only controlled by the broker. If **StopFactor** is set to a negative value or if a **BrokerStop** function is not available in the [broker plugin](brokerplugin.htm), the broker stop is only placed at opening the trade, but not updated afterwards. **StopFactor** has no effect on pool trades in [Virtual Hedging Mode](hedge.htm). Broker stops on [NFA](trading.htm#nfa) compliant accounts require separate orders and must be explicitely enabled with the [SET_ORDERTYPE](brokercommand.htm) command. 

## StopPool

Broker stop loss distance for pool trades in [Virtual Hedging Mode](hedge.htm) (non-[NFA](trading.htm#nfa) mode only; default **0** = no stop loss), in price units. For instance, at **0.25** any pool trade sends a stop to the broker at 25% distance from the initial price. 

## TakeProfit

Profit target: Close the subsequent trade.when the price moved in favorable direction to the given limit or by the given difference to the fill price (default = 0 = no profit target). When a new trade cannot be entered due to the [MaxLong/MaxShort](lots.htm) limit, open trades of the same component are automatically updated to the profit target that the new trade would have.A profit target takes profits early, which increases the number of winning trades, but normally reduces the overall profit of a strategy. It is preferable to use **TrailLock** instead of setting a profit target. 

## Trail

Trailing: Raise the stop loss value as soon as the price reaches the given limit, or goes in favorable direction by the given distance in price units (default = **0** = no trailing). Has only an effect when a **Stop** is set. The stop loss is increased in a long position, and decreased in a short position so that it normally follows the price at the distance given by the sum of the **Stop** and **Trail** distance. For instance, **Stop = 10*PIP** and **Trail = 5*PIP** lets the stop follow the price at 15 pips distance. A slower or faster 'movement speed' of the stop loss can be set through **TrailSlope**. 

## TrailSlope

Trailing 'speed' in percent of the asset price change (default = **100** %); has only an effect when **Stop** and **T****rail** are set and the profit is above the trail distance. Example: The asset price of a long position was above the **Trail** limit and goes up by more 10 pips. **TrailSlope = 50** would then raise the stop loss by 5 pips. **TrailSlope = 200** would raise the stop loss by 20 pips. 

## TrailLock

'Locks' a percentage of the profit (default = **0** = no profit locking); has only an effect when **Stop** and **T****rail** are set and the price has exceeded the trail distance. A stop loss is then automatically placed at the given percentage of the current price excursion. Example: A long position is currently in profit by 20 pips above the entry price. **TrailLock = 80** would then place the stop loss at 16 pips above entry, thus locking 80% of the profit (without trading costs). **TrailLock = 1** (or any small number) would set the stop loss at the entry price when the current price reaches the **Trail** value. Using **TrailLock** is in most cases preferable to setting a profit target. 

## TrailStep

Automatically raise the stop loss every bar by a percentage of the difference between current asset price and current stop loss (default = **0** = no automatic trailing); has only an effect when **Stop** and **T****rail** are set and the profit is above the trail distance.  Example: A long position has a stop at USD 0.98 and the price is currently at USD 1.01.**TrailStep = 10** will increase the stop loss by 0.003 (30 pips) at the next bar. **TrailStep** reduces the trade time dependent on the profit situation, and is often preferable to a fixed exit time with [ExitTime](timewait.htm). Stopping and restarting a strategy counts as a step and will raise the stop loss as if a bar had passed. 

## TrailSpeed

Speed factor for faster raising the stop loss before break even, in percent (default = **100%**). Has only an effect when **Stop** and **Trail** are set and the profit is above the trail distance. Example: **TrailSpeed = 300** will trail the stop loss with triple speed until the entry price plus spread is reached, then continue trailing with normal speed given by **TrailSlope** and **TrailStep**. Has no effect on **TrailLock**. This parameter can prevent that a winning trade with a slow rising stop turns into a loser. 

### Type:

**var**

### Remarks:

  * All parameters above are asset specific. They must be set after selecting the [asset](asset,htm) and before entering a trade with [enterLong](buylong.htm) / [enterShort](buylong.htm). They have no effect on different assets or on already entered trades, except when an [enter](buylong.htm) call updated open trades due to a [MaxLong/Short](lots.htm) limit. For changing a stop or profit target of a trade that was already entered, use either a [TMF](trade.htm), or call [exitTrade](selllong.htm) with the new price limit (see [example](selllong.htm)).
  * Either an (absolute) price level, or a (relative) distance to the trade fill price ([TradePriceOpen](trade.htm)) can be used for **Stop** , **TakeProfit** , **Trail** , and **Entry**. If the value is less than half the asset price, Zorro assumes that it's a distance, otherwise it's a price level. For [options](contract.htm) the prices are always distances, f.i. **Stop = 0.7 * contractPrice(ThisContract)** sets a stop limit at 70% loss of the subsequent contract or combo leg. For an entry limit the given price must be negative; "buy at 10 pips below the current Low" is in code: **Entry = -(priceLow()-10*PIP);**. If **Stop** or **TakeProfit** are on the wrong side of the price, no trade is entered, but a "Skipped" message is printed in the log file. 
  * Stop, profit, trail, and entry limits are handled by the Zorro software and controlled at each [tick](bars.htm#tick). They are normally not sent to the broker, except for **OrderLimit** and the 'safety net' **StopFactor.** The exit limits are therefore unknown to the broker and not visible on their trading platform. 'Stop hunting' or similar broker practices are prevented in this way. Handling limits on the Zorro side also steps around [NFA Compliance Rule 2-43(b)](trading.htm#nfa) that does not allow direct stop loss or profit targets with an order.
  * Stops and order limits are rounded to the next [PIP](pip.htm) step when sent to the broker, since some broker APIs reject them otherwise. The rounding can be disabled by setting the [TR_FRC](trademode.htm) flag.
  * **Entry** and **OrderLimit** are not equivalent. **Entry** generates a  pending trade that is either filled at market when the price touches the limit, or cancelled after [EntryTime](timewait.htm). **OrderLimit** generates no pending trade, but places a limit order at the exchange that is either filled at the given limit, or cancelled after a time determined by [SET_WAIT](brokercommand.htm). A pending trade can be filled at a worse price than the limit; a limit order cannot. A pending trade won't require margin and commission, a limit order might, depending on broker and market. Entry limits are used to enter a position when the price drops by a certain amount or reaches a certain limit. Order limits are normally used to ensure entry at the current price, or at a slightly better price such as the center of ask and bid. **Entry** and **OrderLimit** can be combined for ensuring a fill at the limit price. Some brokers might reject limit orders when **OrderLimit** is too close to the current price. Forex/CFD brokers normally don't support limit orders, but convert them internally to pending trades. It is not recommended to use **OrderLimit** when limit orders are not properly supported by the broker.
  * **OrderLimit** is handled on the exchange and has no effect in a backtest. For a rough order limit simulation by script, use **Entry** as a proxy. Run the test in **[TICKS](mode.htm)** mode with BBO tick data in **.t1** format, convert the limit to an entry ask price, deduct the margin, and set [EntryTime](timewait.htm) to the same time as the broker API wait time (60 seconds by default). For a more precise simulation that considers effects by the API order chain, the execution latency, and the connection speed, use order book data in **.t2** format and fill the trade at the matching quote by [orderUpdate](ordercvd.htm) under consideration of the latency to the exchange. Do not use an entry limit and an order limit at the same time in live trading. 
  * Except for **OrderLimit** , all prices are given as ask prices, regardless of whether they are intended for entry or exit, or for a long or a short trade. A trade is opened or closed at market when the ask price reaches the given entry or exit limit. Zorro automatically handles the conversion from ask to bid: long trades are filled at the ask price and closed at the bid price, short trades are filled at the bid price and closed at the ask price. The bid price is the ask price minus [Spread](spread.htm). For setting prices or distances in pip units, multiply the pip amount with the **PIP** variable (f.i. **Stop = 5*PIP;**). For adapting distances to the price volatility, set them not to fixed pip values, but use a multiple of [ATR(..)](ta.htm) \- this often improves the performance.
  * Price distances and limits need normally not be rounded, unless they are directly sent to the broker, such as **OrderLimit.** Directly sent prices are automatically rounded to the next 1/10 pip for currencies, and to the next pip for all other assets. If a different rounding step is needed, use the [roundto](round.htm) function.
  * In bar-based backtests, intrabar entries and exits are based on a simulated price curve inside the bar. If a limit is hit, trades are filled at the most recent price, which is normally different to the entry, stop, or profit limit**.** If desired, **'naive'** filling at the limit can be enforced with the [Fill](fill.htm) variable. **Stop** is always triggered earlier inside the bar than **TakeProfit** , causing a slightly pessimistic test result when multiple limits are triggered within the same bar. Entry and exit limits will not be triggered at the same at the same time. Use [TICKS](mode.htm) mode when intrabar precision is required, especially when the trade profit depends on intrabar entry and exit limits. 
  * When absolute price limits are given for **Trail** , **TakeProfit** , **Entry** , or **Stop** , they should be at the correct side of the current market price with sufficient distance. Limits at the wrong side of the price cause error messages or unfavorable trades. When the **Entry** condition is already met at entering (f.i. a long trade is entered with the market price already below the entry limit), the trade will be opened immediately at either the market price or the entry price, whatever is worse. When the **Stop** limit is already met at entering, the trade will be opened and immediately closed, losing slippage and spread. If a trade lasts only one or a few [ticks](bars.htm#tick), the result by stop or trailing can become inaccurate by a large factor due to the limited resolution of the price history. 
  * **TrailSlope** , **TrailStep** , and **TrailLock** can be set simultaneously. The stop loss limit is then trailed by their largest movement inside a tick or bar. On a long trade, the stop limit is trailed by the High and limited by the Low; on a short trade it's the other way around. When the **Stop** is moved inside the price range of the current bar or tick, the trade will be immediately closed. If a more complex stop loss / take profit / trailing behavior is required, use a [TMF](trade.htm). The TMF runs at every price quote - or every [tick](bars.htm#tick) \- and can trail and check stop and profit limits. 
  * A stop loss - regardless if handled by software or sent to the broker - is no guarantee to limit losses. In case of low market liquidity or price shocks (such as the EUR/CHF price cap removal in January 2015) brokers can often not close trades at the stop loss level, and close them at the next opportunity instead. The real loss of the trade will then be remarkably higher. 
  * Stop limits updated to the broker can appear to temporarily move in 'wrong' direction by a small distance. This can happen when the spread changes at the same time, but in opposite direction as the price. 
  * Stop limits affect the trade volume when the [Risk](lots.htm) variable is used. The risk of a trade per lot, in account currency and without transaction costs, is PIPCost /PIP * (Stop distance). The stop distance at a given risk value per lot is therefore Risk * PIP/PIPCost.
  * When Zorro runs unobserved, placing stop loss limits is recommended, even when they appear to reduce the strategy performance. They protect against price shocks. The trade engine also uses them for calculating the risk per trade; without a stop, the capital exposure is not available and the performance statistics are less precise.
  * The win rate ('accuracy') of a system mostly depends on the **Stop** /**Takeprofit** ratio, also called risk/reward ratio. This way you can set up any win rate for your system up to almost 100%. Chapter 9 of the [Black Book](links.htm) deals with the consequences of setting extreme win rates.

### Examples:

```c
// set stop at 1% distance 
Stop = 0.01*priceClose(); 
enterLong();

// place limit order at ask/bid center
OrderLimit = priceC() - 0.5*Spread;
enterLong();
```

### See also:

[bar](bars.htm), [enterLong/Short](buylong.htm), [exitLong](selllong.htm)/[Short](selllong.htm), [](lots.htm)[EntryTime](timewait.htm), [ExitTime](timewait.htm), [Lots](lots.htm), [TMF](trade.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
