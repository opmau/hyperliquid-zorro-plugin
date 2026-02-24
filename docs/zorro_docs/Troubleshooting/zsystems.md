---
title: The Z algo trading strategies
url: https://zorro-project.com/manual/en/zsystems.htm
category: Troubleshooting
subcategory: None
related_pages:
- [Links & Books](links.htm)
- [Licenses](restrictions.htm)
- [Performance Report](performance.htm)
- [CBI](ddscale.htm)
- [Virtual Hedging](hedge.htm)
- [FXCM](fxcm.htm)
- [IB / TWS Bridge](ib.htm)
- [Asset and Account Lists](account.htm)
- [Oversampling](numsamplecycles.htm)
- [set, is](mode.htm)
- [Money Management](optimalf.htm)
- [Evaluation](shell.htm)
- [Visual Studio](dlls.htm)
- [markowitz](markowitz.htm)
- [brokerCommand](brokercommand.htm)
- [Equity Curve Trading](trademode.htm)
- [BarMode](barmode.htm)
- [Verbose](verbose.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [Oanda](oanda.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Log Messages](log.htm)
- [Live Trading](trading.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [W6 - Portfolios, Money Management](tutorial_kelly.htm)
- [Troubleshooting](trouble.htm)
- [Trading Strategies](strategy.htm)
- [Testing](testing.htm)
---

# The Z algo trading strategies

# The 'Z' Strategies: Trading for Regular Income 

Before starting automated trading with one of the included Z systems on a real money account, please read this page from top to bottom. Make sure that you understood how the system principially works, how it is set up, and how it is adjusted to your capital, broker, and account. At first, some general rules for achieving a regular trading income. 

  * Trade only what you understand. Know your algo trading method, the market inefficiency on which it is based, and what risk and performance you can expect. Test it under different spread and margin settings to avoid nasty surprises. Stay away from systems with 'secret algorithms' - it's easy to set up a 'secret system' and fake an impressive 'live equity curve' that's even verified on MyFxBook™. You can read up about that in the **[Black Book](links.htm)** .  

  * Have enough capital. The minimum capital for a modest regular income is in the $30k range. The free Zorro version limits your account size to $15k (see [restrictions](restrictions.htm)), but you need not all capital in the broker account - you can keep it in a bank account and quickly remargin when necessary. Have at least twice the [required capital](performance.htm) from the performance report at your disposal. Most brokers allow depositing funds by credit card in a few minutes.   

  * Expect long periods under water. Many successful strategies have strong equity fluctuations with short profit bursts and long sidewards and downwards periods. Check out the **Z5** live profit curve below to learn what you can expect. The [down time](performance.htm#totaldown) of strategies - the time when the account is lower than it was before - is usually in the **80%-90% area**. Which means that when you start with $1000, there's a high chance that you are at $900 some days later. If you panic and pull out, the money is gone. This is a well known phenomenon in financial investment, and causes people to lose money even with highly profitable funds. Better grit your teeth and sit it out.  

  * Do not get greedy. When your profits accumulate, you'll be tempted to likewise increase the investment. Don't. Follow the square root rule: For every doubling of your account, increase the invested margin by factor **1.4** , not by factor **2**. Increasing the investment too fast can wipe out any account. In the Black Book you can find the formulas for the optimal amount to withdraw for a regular passive income.   

  * Pull out in time.**** Examine the trading results regularly - at strategy start, daily - and compare them with the [test performance](performance.htm). If your strategy strongly deviates from the expected performance, don't hesitate and stop it. It might be expired due to a permanent change of the market. This happened in fact already with several Z systems, like Z3 and Z5.  
What is a "strong deviation"? The simplest way is comparing the current drawdown with the **Required Capital** from the backtest - when you lost noticeably more, you should stop. A better way is checking the [Cold Blood Index](ddscale.htm) of your system that's daily updated by the Z systems and printed on the status page and the log.  

  * Do not tamper with the strategy. It's hard to see profits dwindle in a drawdown. But don't forget that the strategy is already optimized, tested, and verified (or at least, it should). Manual intervention cannot improve it, but will make it worse. Only exception is when you know of some upcoming event - such as bizarre statements from a president - with great impact on open positions. In that case it makes sense to get out of the market or use the Z system's **Panic** slider for locking current profits.  

# The Z Systems: Overview

The Z systems are prefabricated, ready-to-run algo trading strategies for a wide range of markets. They are not based on 'technical analysis', but on solid math. Their algorithms can be found in the [Black Book](links.htm#black), some on the [Financial Hacker](https://financial-hacker.com/) website. To avoid misunderstandings: these systems should not be your only reason of using Zorro! We encourage you to learn developing your own systems. Strategy development is not trivial: If a market inefficiency were very easy to spot and exploit, many trading systems would do that, and the inefficiency would be leveled quickly. This effect keeps the number of successful strategies in check and gives programmers a large advantage over traders. 

The Z systems are relatively simple. None of the basic algorithms exceeds 25 lines of code. Still, all systems underwent a solid development and test process with the methods described in the Black Book. Early versions of some systems had been in live trading for almost a decade; all results so far have been consistent with the simulation. We're constantly observing the systems and modify or replace expired algorithms with any Zorro update.

When using a Z system, do not do it blindly, but learn system development, understand how it works, and eventually replace it with your own system. There are **five good reasons** why you should prefer a self-programmed system. Firstly, you have the opportunity to program better strategies than the relatively simple Z systems. Secondly, systems traded by a lot of people don't work very well - price curves, especially of stocks, can be noticeably affected even by a small number of traders. Third, you can face the inevitable drawdowns in a more relaxed manner when you know precisely what the systems do and why. Fourth, you'll gain experience with programming and statistics - valuable skills not only for trading, but in general life. And finally, you cannot blame us for losses. 

The Z systems cover a large range of trade methods, win rates, and budgets. Performance metrics as of **January 2026** :

System |  Z1+ |  Z2+ |  Z12+ |  Z6+ |  Z7 |  Z8 |  Z9 |  Z13  
---|---|---|---|---|---|---|---|---  
**License** | **free** | **free** | [Zorro S](restrictions.htm) | [Zorro S](restrictions.htm) | [Zorro S](restrictions.htm) | **free** | [Zorro S](restrictions.htm) | [Zorro S](restrictions.htm)  
**Traded Assets** |  Forex, CFDs  |  Forex, CFDs  |  Forex, CFDs  |  Forex |  Forex |  Stocks, ETFs |  ETFs, Treasure  |  Options   
**Method** |  Trend   
following  |  Mean   
reversion |  Anti-  
correlation |  Excess  
volatility  |  Price   
patterns  |  Portfolio  
rotation |  Portfolio  
rotation |  Combo   
selling  
**Algorithm** |  Spectral  
analysis  |  Spectral  
analysis  |  Spectral  
analysis  |  Adaptive   
grid |  Machine   
learning |  Markowitz   
MVO  |  Dual   
momentum  |  Volatility  
filter   
**Risk** |  High |  High |  High |  Moderate |  High |  Moderate |  Moderate |  High  
**Bar period** |  1 hour |  1 hour  |  1 hour  |  by tick |  1 hour  |  1 day  |  1 day  |  1 day   
**Avg Trade** |  1 day |  10 days  |  2 days  |  40 hours |  6 hours |  6 months |  6 months |  6 weeks  
**Rebalance** |  6 months |  6 months |  6 months |  6 months |  6 months |  7 weeks  |  7 weeks  |  6 weeks   
**Max DD** |  8 months |  8 months |  8 months |  4 months |  22 months |  11 months |  8 months |  12 months  
**Return** |  CR 160% |  CR 100% |  CR 210% |  CR 60% |  CR 190% |  CAGR 16% |  CAGR 25% |  CAGR 45%  
**W/L** |  1.4 |  1.2 |  1.3 |  1.7 |  1.2 |  3.2 |  5.4 |  1.3  
**R2** |  0.90 |  0.88 |  0.97 |  0.97 |  0.90 |  - |  - |  -  
**Win Rate** |  42% |  49% |  46% |  69% |  48% |  69% |  74% |  44%  
**Capital** |  2000 $ |  2000 $  |  4000 $  |  1000 $  |  1000 $  |  5000 $  |  5000 $  |  9000 $   
**Monthly Income** |  170 $ |  130 $ |  570 $ |  40 $ |  120 $ |  260 $ |  640 $ |  9000 $  
  
The metrics above have been generated from a walk-forward analysis with default capital, default asset lists, and [virtual hedging](hedge.htm) enabled. For stock or ETF portfolios, 3% broker interest on margin is assumed. Using smaller capital, no virtual hedging, or assets with lower leverage or higher spreads, swaps, and commissions reduces the perfomance. For instance, 25:1 leverage instead of 100:1 reduces the annual return of the Z12+ system from 200% to 80%. 

The Forex/CFD systems use a subset from the standard asset lists, selected for robust returns and a balanced portfolio. You can remove, but not add assets. The Z systems with dedicated asset lists allow adding and removing assets at your own risk. For early detecting deviations, we're running all Z systems permanently on real money accounts. The Forex/CFD systems are running on [FXCM](fxcm.htm) accounts, the systems for options, stocks and ETFs on [IBKR](ib.htm) accounts. This has historical reasons and is in no way a recommendation for a particular broker.

The Risk of a system is estimated from equity curve fluctuations and fundamental criteria. Rebalance is the period after which the system should be rotated or should get newly trained parameters from us. Return for not reinvesting systems is the annualized **Calmar Ratio** , the annual profit divided by normalized maximum drawdown. For portfolio rotation systems that reinvest profits, it's the **CAGR** , the annual account growth. W/L is the **profit factor** , the net win divided by net loss. The R2 coefficient is the similarity of the equity curve to a straight upwards line (ideal = 1.0); it is a metric about the stability under different market situations. Reinvesting systems have an exponential equity curve and thus no R2 coefficient. 

Capital is the initial capital for the backtest. You can set it up with the **Capital** slider, up to the limit of the free Zorro version. It is not recommended to trade with smaller capital, since then trades will be skipped and the performance will decrease. Any strategy has therefore a lower limit to the capital, depending on account leverage. On low-leverage accounts, determine the required capital by backtesting. There is also an upper limit to the capital: If you want to invest millions, you'll need 'iceberg' orders and special entry/exit mechanisms. Contact us in that case.

Montly income in the above table is the average monthly return from a backtest with the given capital. For strategies with rotation or reinvesting, such as Z8, Z9, Z13, the average monthly income has not much meaning since it will be higher at the end and lower at the begin of the test period. For **long-term investment** , the **Z8** or **Z9** systems are probably the best suited since they have the lowest risk. The other Z systems have higher annual returns, but accordingly higher risk, and can theoretically cause a total loss of the invested capital. Many systems have long drawdown periods that you can see in the equity curves below. You're more likely to start in a drawdown than not. Be prepared for that and do not invest what you can't afford to lose. 

**The Z systems will not start right away.** First you have to download historical data for testing them. So if you complain on the user forum about "Z1 displays 'history data missing' errors", we know that you have not read the manual! Stock trading systems load their data automatically, for the other systems the historical data files can be downloaded from the [Zorro download page](https://zorro-project.com/download.php) and unpacked into the **History** folder. After that, click [Test]. If the systems had optimized parameters or rules, they run a walk-forward analysis, otherwise a plain backtest for determining their historical performances with the set **Capital** and [asset list](account.htm). This can take several minutes especially with systems such as Z1, Z2, or Z12 that run through several [oversampling](numsamplecycles.htm) cycles. The walk-forward analysis ends at a fixed date, the plain backtests end at the end of the history. The meaning of the performance figures can be found under [performance report](performance.htm). 

For **live trading** the Z systems, you normally first have to create an individual [asset list](account.htm) with the parameters and symbol names used by your broker (see below). The Z systems do not automatically reinvest profits in live trading. The traded volume depends only on the **Capital** slider and is unrelated to the account balance. For reinvesting, increase the **Capital** slider once per month according to accumulated profit (if any), and follow the square root rule for reinvesting in high leverage forex and CFD systems. The **InvestCalculator** script can be used for determining the optimal investment size. The Z systems use the [ACCUMULATE](mode.htm) mechanism that allows trading with very small capital exposure.

**Training** the Z systems is not necessary, since we provide updates with new trained rules and parameters with any Zorro update. For this reason, use always the latest Zorro release when you trade a Z system. If you still want to train them between updates, you'll need Zorro S; see 'Tips & Tricks' below.

The Z systems can be adapted to your need as described below, but they are not available in source code. They are compiled executables (***.x** or ***.dll**). This is not because they would use some secret trading system. On the contrary, the algorithms are all published in the [Black Book](links.htm) or elsewhere. After reading the book, you can easily write similar strategies, or hire our programming service to write them for you. But if we provided the source code, one could determine precisely when the strategies buy and when they sell. It would then be easy to prey on Z system traders with fore-running methods. For the same reasons, the strategies use a small random time offset in live trading, so that they don't open positions all at the same time. 

The same Z system can produce **slightly different returns** on different accounts, even with the same assets and the same broker. This is partially caused by the above mentioned random offset, but mainly by different start dates and different capital. Because the number of open trades is limited, a different start date leads to a different trade history. Dependent on available capital, trades are skipped. In the long term the returns will equalize, but in short term they can be different.

Due to the [profit limit](restrictions.htm), the free Zorro version allows to trade only one system \- either one of the Z strategies or a system of your own - on a real money account. Z1, Z2, and Z8 are included in the free version. The other systems are included in [Zorro S](restrictions.htm).

### Z1+, Z2+, Z12+

Z12 is a portfolio of algorithm variants, based on **chapters 3 and 4** of the Black Book. It conists of two sub-systems: Z1 algos trade on trend reversals, Z2 algos detect market cycles for trading against the trend. The systems are intended for Forex/CFD brokers with high leverage accounts. The algorithms are self-adapting to the market and use an 'equtiy curve trading' mechanism for skipping unprofitable market periods. **[OptimalF](optimalf.htm)** factors distribute the capital among the components. Some of the algorithms control the trade duration by moving the stop loss; this can cause the stop to move in both directions. 

In 2026, Z12 has been modernized, converted to C++ and based on [evaluation shell](shell.htm) algos. The new algos run on different timeframes of 1, 2, and 4 hours. This has increased the annual return and produced a smoother equity curve with smaller drawdowns. 

![](../images/z1perf.png)  
Z1 equity curve

![](../images/z2perf.png)  
Z2 equity curve

![](../images/z12perf.png)  
Z12 equity curve

Z1 and Z2 share their parameter and factor files with Z12. Due to the relatively long lookback period, they should only be traded with brokers that provide sufficient price history. If price data gaps are indicated in the message window at start, it is recommended to use the **Preload** flag (see below) or trade the system with a different broker. 

### Z6+

The new Z6+ system is a special type of grid trader with an **adaptive grid**. Although we have listed grid trading among the [ 17 worst trading methods](https://financial-hacker.com/seventeen-popular-trade-strategies-that-i-dont-really-understand/), this one is different, since it is based on a real inefficiency of Forex markets. This effect can be statistically analyzed and is visible in the volatility chart below, from the AUD/USD pair:

![](../images/Z6_VSpectrum_s.png)

The red line is the maximum price movement in pips over the period given on the X axis in hours. The black line is the same maximum movement if the markets were perfectly efficient, simulated by randomizing the price curve. The curves don't touch zero because the smallest sample period was one hour. We can see that they approximate a square root function, as can be expected, since volatility follows that rule. But we can also see that the real short-term volatility greatly exceeds the theoretical volatilty from a random price curve. This 'excess volatilty' effect is a strong inefficiency, and remains consistent over the years. It is exploited with this system. 

Trades are triggered by price ticks, so the bar period is not very relevant. The algorithm will be described in detail in an upcoming book about Forex trading. The system can be traded with very low capital, has a high win rate and an almost ideal steadily rising equity curve with **0.96** R2 coefficient. The results are unrelated to trends and cycles, and uncorrelated to all other systems.

![](../images/z6perf.png)

Download the minor pairs Forex history for backtesting the system. The equity curve above is from a walk-forward analysis on a Forex account with 100:1 leverage. The blue lines in the small charts below are the separate equity curves of the Forex pairs. You can see that the overall equity curve is much smoother, since several currencies contribute to it. For eliminating the dangers of grid trading, all trades have a maximum life time of one week. Most close earlier. Since long and short positions can be open simultaneously, [virtual hedging](hedge.htm) is recommended. The stop loss is relatively distant and is used for risk limitation only. Z6+, as indicated by the '+', was written in [C++](dlls.htm).

### Z7

Z7 is a short-term, small-capital forex trading system based on the price pattern detection algorithm from **chapter 6** of the [Black Book](https://www.amazon.com/dp/1546515216). It uses time dependent filters to catch matching patterns on the US, European, and Pacific sessions. A 'master algorithm' detects price patterns, algorithm variants with several filters enter the trades. Other than the system described in the book, this one passed the reality check. The results are unrelated to trends and cycles, and uncorrelated to the other systems.

![](../images/z7perf.png)

The equity curve above is from a walk-forward analysis on a Forex account with 100:1 leverage. All Z7 trades end after at least one market day, most after a few hours. The stop loss is relatively distant and is used for risk limitation only. A trailing mechanism locks profits when the trade goes high into profit in volatile situations. The system produces many trades with a relatively small profit per trade, so its performance depends heavily on leverage and transaction cost, i.e. spread, commission, and slippage.

### Z8

Z8 is a long-term trading system based on modified [Markowitz mean/variance optimization](markowitz.htm) (MVO). It is a variant of the MVO system described on [Financial Hacker](http://www.financial-hacker.com/get-rich-slowly/), and should be traded with a stock/ETF broker. Z8 opens a portfolio of ETFs, stocks, or other assets determined by an external asset list, and re-allocates the invested capital among the portfolio components in regular intervals for keeping a positive balance curve with minimum variance. Since it trades with low leverage, the profit, but also the risk is smaller compared to the other systems. 

![Z8 equity curve](../images/z8perf.png)

The balance curve above is from a walk-forward test with the default assets, based on a margin account with 2:1 leverage and $5000 start margin. The backtest reinvests the capital growth to the power of 0.9, and keeps the remaining profit as cash reserve on the account for buffering drawdowns. The assets, account leverage, and price data source must be set up in an [asset list](account.htm) named **AssetsZ8.csv**. By default it contains a set of ETFs that were selected by industry diversity and by their supposed long-term prospects. Z8 can also trade stock portfolios or any other instruments with long-term positive returns. Forex or CFDs are not suited for Z8. For adding a new asset, just duplicate a line in **AssetsZ8.csv** and edit the asset name in the first column. For temporarily out-commenting a line, add a '#' in front of the asset name. The **Symbol** column contains also the download source. Up to 300 assets can be contained in the list, but you should have no less than 5 and no more than 30\. When you remove an asset from the list, make sure to also manually close any of its open positions (this won't happen automatically). 

Z8 needs not be trained, since this is automatically handled by the MVO process. The [Train] button can instead be used for finding assets with low correlation ([Zorro S](restrictions.htm) only). It displays a heatmap of the correlations between the assets in **AssetsZ8.csv** (red = high correlation, blue = low correlation). If you don't have Zorro S, you can use the script for generating heatmaps on Financial Hacker. 

![Z8 equity curve](../images/Z8heat.png)

For the backtest, historical data is automatically downloaded from the data providers set up in the 'Symbol' column in the asset list, so no further price history files are necessary. If you want to download data from a different source, either edit the asset list, or load it before starting the strategy. The system will automatically detect that up-to-date data is already present, and not attempt to download it again. 

For trading Z8, the broker API must support the [GET_POSITION](brokercommand.htm) command, which is the case for the [IB TWS API](ib.htm). Make sure that you've subscribed market data for all traded assets (with IB, normally the US Equities "Value Bundle" and "Streaming Bundle"), that you're permitted to trade them (check all relevant boxes on the "Trade Permissions" page), and that the leverage in **AssetsZ8.csv** matches your account leverage. If your portfolio contains leveraged ETFs, check their margin requirement: IB accepts **Leverage** **2** for unleveraged or 2x leveraged ETFs; **Leverage** **1.333** for 3x ETFs; and **Leverage 1** for 4x ETFs. 

Since Z8 trades only every 5th week, it needs not run permanently, and requires no VPS and no permanent broker connection. Just start it in [Trade] mode once every 5 weeks after the opening time of the New York market (9:30 ET). Z8 will first download historical prices of all assets, then calculate the optimal portfolio. Set the **Capital** slider to the total margin you want to invest. That's normally the cash value on your account. After about a minute a message box will pop up, like this: 

![Z8 equity curve](../images/z8msg.png)

"Old" is the current position in the asset, and "new" is the new position. Clicking [Yes] will automatically sell or buy the difference of any position, and this way optimize the portfolio. If the broker API does not support position requests, the 'old' positions will all be 0, even when positions are open. In that case open or close the position differences manually in the broker's trading platform, then click [No] on the message box. Afterwards the trading session is closed, and the suggested date for the next start is printed in the message window. 

Since you can only buy integer numbers of shares, the real invested margin will normally be smaller than **Capital** , dependent on portfolio component weights and prices. The maximum capital can be set up in **Z.ini** (see below). For avoiding margin calls, we recommend to set **Capital** not higher than about 80% of the account equity. For accounts with no leverage, edit the asset list and set all leverages to 1; you can then set **Capital** at 100%. If the system does not open all trades - for instance due to a temporary connection problem or to being outside market hours - just run it again. Since it is detects actual positions, running it several times does no harm. If you missed the suggested start date, maybe because you're on vacation with all the money you made so far, run it at the next occasion. Missing a single date does not do much harm, but don't miss many dates in a row.

### Z9

The portfolio rotation system from Black Book **chapter 8** , but with sector ETFs, as proposed by Gary Antonacci in his "Dual Momentum" strategy. It uses the assets in **AssetsZ9.csv** , which should contain a mix of US sectors, non-US indices, and bonds or treasury. The portfolio is rotated in regular intervals so that it always contains the top third of the assets, selected by their highest relative momentum and positive absolute momentum. If no sector or index has any positive momentum, the system invests in gold or treasury. If gold or treasuries tank too, the system stays out of the market. 

![Z9 equity curve](../images/z9perf.png)

The balance curve above is from a walk-forward test with the default assets, based on a margin account with 2:1 leverage and $5000 initial capital. Although the Z9 system uses a simpler algorithm than the Z8 system above, is achieves better CAGR. Editing the asset list, setting up the **z.ini** parameters, reinvesting profit to the power of 0.9, and rebalancing the portfolio works just as described for **Z8** above. In the **Type** column of the **AssetsZ9** list, enter **0** for sectors, **1** for bonds, **2** for non-US indexes, and **3** for a lead index that represents the market. The lead index is not traded, but used for detecting weak market periods. It can be omitted at slightly higher risk. As with Z8, [Train] generates a correlation heatmap. For trading Z8 and Z9 simultaneously on the same account, **AssetsZ8** and **AssetsZ9** must be modified for making sure that they share no assets. 

Since US ETFs are not available anymore to European traders due to PRIIP regulation, an alternative asset list **AssetsZ9E** has been included that contains only ETFs that can be traded in Europe. The assets are similar, but not identical to their US counterparts, so the profit is smaller than with US ETFs. The asset list can be set up in the **Z.ini** or **Z9.ini** file. 

### Z10

Z10 was a cryptocurrency rotation system based on mean-variance optimization like **Z8**. It traded with a portfolio of digital coins. Due to its high risk and volatility, we have decided to abandon it and remove it from the Z systems. 

### Z13

A variant of the options selling system from **chapter 7** of the [Black Book](links.htm). Z13 trades with a stocks and options broker, like [IB](ib.htm). It sells in-the-money SPY option combos and collects the differences of premiums and assignment losses. The combo type can be set up with the [Action] scrollbox. It determines the risk and CAGR of the system. 

**Naked Put** | Sell in-the-money puts (high risk).   
---|---  
**Put Spread** | Sell and buy puts, one in the money and one out of the money.  
**Condor** | Sell a put, buy a put, sell a call, buy a call.   
  
A naked put can lose a large amount, spread and condor limit the loss to the strike difference. Risky market situations are detected with a volatility filter; trading is then suspended. The backtest reinvests profits at an exponent given with the **Reinvest** slider (1.0 = reinvest all profits, 0.5 = reinvest square root of profits, 0 = no reinvestment). The figures in the table are from a backtest with naked puts and 0.9 reinvestment.

When options get exercised, close the assigned position manually, or let the broker liquidate it at market. If you want to leave the closing to the broker, take care to regularly withdraw profits for keeping the account in an insufficient funds state for enforcoing a margin call. If the broker closes the position only partially, close the rest manually. If the broker prematurely exercises options due to insufficient funds, as some brokers do, either have enough capital on the account for preventing this to happen. Or trade the system with an European-style option, such as **SPX** or **XSP**. This reduces the profit, but prevents the counter side from exercising the sold leg. You can set up the instrument in **AssetsZ13.csv**. The system trades the first asset from that list.

For backtesting Z13 you'll need historical **SPY** options data that you can get from the Zorro download page. The chart below displays the balance curve (blue) of selling naked puts at 0.9 reivestment, compared with a benchmark line (grey) from a SPY buy-and-hold strategy at leverage 2.

![Z10 equity curve](../images/z13perf.png)

For trading Z13, the broker API should support the [SET_COMBO_LEG](brokercommand.htm) and the [GET_POSITION](brokercommand.htm) commands, which is the case for [IBKR](ib.htm). Make sure that you're permitted to trade options, and check the relevant boxes on the "Trade Permissions" page. If you have no market subscription for SPY, set **Preload** in **z.ini** (see below) and download SPY from another source. Z13 trades 6-week options and thus opens new positions only every 6th week. Therefore it requires no VPS and no permanent broker connection. You only need to run it every 6 weeks. Start it in [Trade] mode after the opening time of the New York market (9:30 EST) when the options from the previous run are expired or have been exercised. If a single leg of a combo has been prematurely exercised, close the remaining legs and the assigned underlying manually and start Z13 afterwards for entering the next combo.

At start, set the **Capital** slider to the total margin you want to invest. For reinvesting profits in live trading, you can use the **InvestCalculator** script to determine the amount. Z13 will first download SPY historical prices and the SPY options chain, which can take up to 20-30 minutes.Then it will place the option orders. Confirm them when the message box pops up. If Z13 considers the market too volatile or the **Capital** too low, it will instead print a message (f.i. **"need XXX"**). If the market is unsuited, wait a few days. If it's too low capital, be more generous with the **Capital** slider and add funds if needed. The statistically best time for running Z13 is Tuesday 15:30 EST after option expiration and short before market close.

Z13 achieves higher returns with less effort than a mere portfolio rotation system, but at higher risk. Do not trade with money that you cannot afford to lose. 

# The Z Systems: Setup and Handling 

Every Z system can be configured at startup with parameters from an **.ini** file, and is controlled at runtime with the **Capital** and **Panic** sliders. First set up the strategy to your country and other requirements: open **Strategy\z.ini** with the script editor, and edit the following parameters:

**Line** | Default | Description  
---|---|---  
**NFA = 0** |  No NFA compliance | Set this to **1** for for [NFA mode](mode.htm#nfa) (only needed when NFA was not defined in an [account list](account.htm)). Often required for US accounts and NFA brokers, f.i. [IB](ib.htm); not for MT4/MT5 accounts. A wrong NFA flag causes trades not being closed, so test your account before setting it!   
**Phantom = 0** |  No equity curve trading. | Set this to **1** for enabling [equity curve trading](trademode.htm). When active, trades of Z1, Z2, or Z12 components are temporarily suspended when the algorithm deteriorates, and resumed when it becomes profitable again. Equity curve trading can reduce the profit, but can also prevent losses by expiration of strategy components.  
**NoLock = 0** | Locking enabled | Set this to **1** for setting the [NOLOCK](mode.htm) flag. This prevents Zorro instances to wait for each other when sending commands to the broker API. Use this flag at your own risk and only when all Zorro instances trade with different brokers.  
**Hedge = 4** |  Virtual hedging. | Set this to **2** for disabling [virtual hedging](hedge.htm), to **4** when your broker does not support partially closing of trades (such as MT5 and some MT4 brokers), or to **5** for full virtual hedging with partial closing. [Zorro S](restrictions.htm) required for virtual hedging. Close open trades and restart the strategy after changing this parameter.  
**BarMode = 0** |  By script | Set this to **32** for activating [BR_SLEEP](barmode.htm) or to **96** for activating [BR_LOGOFF](barmode.htm).  
**Export = 1** |  IBKR export | Set this to **1** for generating a CSV file that can be directly imported in the [IBKR](ib.htm) TWS portfolio reblance window.  
**MaxCapital = 10000** |  10000 | Set this to the desired maximum value of the**Capital** slider for trading with higher volume (see [Zorro S](restrictions.htm) for capital limits)..   
**ScholzBrake = 0** |  0 | Activate the [ScholzBrake](lots.htm#scholz) (German users only).   
**Verbose = 1** |  Verbosity. | Set this to **2** or **3** for more messages (see [Verbose](verbose.htm)). Set it to **15** for 'black box' diagnostics.  
**BrokerPatch = 0** | Broker API ok. | Set this to **3** for letting Zorro calculate the balance, equity, and open trade profits displayed on the Zorro panel (see [SET_PATCH](brokercommand.htm)), rather than reading the values from the broker API. Recommended when the broker API does not return correct trade profits or equity. This was reported for some versions of the FXCM API, and some MT4 servers.   
**Preload = 0** | Load all prices. | Set this to **1** for loading the lookback period prices not from the broker's price server, but from Zorro's historical data files (see [PRELOAD](mode.htm)). Recommended when the broker has only a very short price history, and/or when the llookback period is rather long. Recent history files are required.   
**StopFactor = 0** | No broker stop. | Set this to the [StopFactor](stop.htm), or to **0** for not sending the stop loss level to the broker (see [StopFactor](stop.htm) and [StopPool](stop.htm)). Check before if broker rejects trades with too distant stops, or accepts only certain stop values, or requires unique sizes for stop loss orders (f.i. [Oanda](oanda.htm)).  
**MaxRequests = 0** | Default request limit. | Use this to limit the [ requests per second](ticktime.htm) when several Zorros trade on the same broker account that has a request limit  
**BarOffset = 0** | Default bar offset. | Set this to the trading time in minutes since midnight when trading assets on daily bars in other time zones. Close open trades and re-train the strategy after changing this parameter.   
**AssetList = ""** | Default account.  | Enter a different asset list for simulating different accounts or for setting up different symbols. All traded assets must be contained in the list. Close open trades and restart the strategy after changing this parameter.  
**Cancel = 0** | Cancel no trade. | Set this to the trade ID when a position was closed externally and must not be managed anymore by Zorro. Zorro normally detects externally closed trades, but not when a trade is closed by opening a counter-trade on an NFA account. The cancelled trade will be removed from the trade list at the end of the current bar. In the case of virtual hedging, cancel the phantom trade only; this will also automatically cancel the pool trade.   
  
The **.ini** file is read at start of the strategy. If you want to trade different Z strategies with different parameters, save **Z.ini** under the name of the strategy, f.i. **Strategy\Z12.ini** for Z12. The strategy will then read the parameters from its corresponding **.ini** file.  Not all parameters have effect on all strategies; f.i. for Z8/Z9 only **MaxCapital** , **Verbose** , and **AssetList** are relevant.

While running, the Z strategies are controlled with up to 3 sliders, dependent on the system: 

**Slider** | Range | Default | Description  
---|---|---|---  
**Capital** | **0..10000** | **1000 .. 9000** | Determines the trade volume by setting the invested total amount. The amount on the broker account is irrelevant for the trade volume. In [Test] mode you can adjust this slider for adjusting the desired monthly income (**MI**) and the minimum required capital on the account (**Capital** in the message window). Set **MaxCapital** (see above) for modifying the slider range.  
**Percentage** | **0..200** | **150** | **Z8** , **Z9** only: capital percentage for generating an IBKR rebalance file that can be imported in the TWS rebalance window. **Export** must be set in the .ini file. On margin accounts, more than 100% of the capital can be invested, but do not exceed 200%.  
**Panic** | **0..100** | **0** | Tighten the [stop loss](stop.htm) of all open trades in percent of the current price difference to the original stop loss. At **0** , stop loss control is completely up to Zorro; at **100** the stop limits are placed just at the current asset prices. This causes trades to be sold as soon as their prices move slightly in adverse direction, and can be used for taking profits early. No new trades are opened as long as the position is above **90**. This slider reduces the profit, so use it only in drastic situations, for instance when an upcoming event - like Brexit - will have a large impact on the stock market. Put it back to **0** when the danger is over.   
  
The sliders can be set after starting the strategy in test or trade mode (if no strategy is running, the sliders have no meaning). The last slider positions are stored, and the sliders snap to their last positions when the strategy is started again. Move them afterwards to a new posiion if desired. 

The most important is the **Capital** or **Percentage** slider. The Z systems do not autonomously reinvest profits or determine otherwise how much to invest; that is solely your decision and set up with this slider. Before trading the system, find your minimum capital requirement by running a [Test] with your current account parameters (see [asset list](account.htm)) and the current slider position. Check the **Required Capital** (read [performance report](performance.htm) about how it's calculated) - that's the absolute minimum you need either in your broker account, or at least in your bank account. For reinvesting your profits, you can slowly increase the slider when the capital on the account grows. For not risking a margin call, take care to reinvest or withdraw capital only according to the **square root rule** explained in the Black Book. 

For a realistic backtest, make sure that the account settings in the [asset list](account.htm) are correct and up to date. For the Forex and CFD systems, an average Forex broker account with 100:1 leverage (**AssetsFix.csv**) is simulated in the backtest. For stock and ETF trading systems it's an average IB Margin account with 2:1 leverage. Capital requirement and System performance can be different on different accounts, especially with higher trading costs or lower leverage. If a certain asset is not provided by your broker, remove it from the strategy as described below. If it is available, but under a different symbol, edit the [asset list](account.htm) and enter its symbol name. US citizens are often not allowed to trade CFDs, and EU citzens are not allowed to trade US ETFs. Remove or replace unavailable assets as described below in the FAQ. 

Check the requirements of your broker and account. Typical issues are not being able to open or to close a trade (**"Can't close..."**) due to NFA compliance, FIFO compliance, a wrong symbol, an invalid trade size, or unsupported partial closing. Make sure that **NFA** and **Hedge** are set correctly. 

Before starting live, run a final [Test] with the **Capital** slider at the same position as in the trading session. This allows the system to permanently compare live performance with backtest performance, using the [Cold Blood Index](ddscale.htm). The Capital can be increased later, but the initial position must match the last backtest. The CBI is printed once per day in the message window and on the status page. Consider pulling out when it gets down to a low value or 0. 

After starting a strategy, it can take some days or even weeks before the first trades are entered. Don't grow impatient. The frequency of trades depends on the strategy and the market; in unprofitable periods with high market efficiency there can be no trades at all, especially when **Phantom** is set in the **Z.ini** file. Most trades are hold for 2-3 days, but some can stay open for weeks when they are profitable. 

Don't unnecessarily stop and restart strategies. This closes open trades, initializes intermediate parameters, and sets back the lookback period. All this will reduce the profit and performance. Don't tamper with the trades: manually closing an open trade at the wrong moment can cost several hundred pips and can convert a winning strategy to a losing one. For 'soft stopping' a strategy without loss, set the **Capital** slider to zero so that no new trades are opened, then wait until the strategy itself has closed all open trades.

Your account balance will often go down in the first weeks after starting a strategy. This has 3 reasons: Drawdown periods are longer and more likely than profitable periods; unprofitable trades are closed first and thus contribute earlier to the account balance; and equity fluctuations - the up and down 'ripples' in the equity curve - almost always exceed the slowly increasing initial profit. Thus, periods of negative balance and equity are common at start of almost all strategies, as in the live profit curve below (from the **Z5** system, now expired). When the profit accumulates, negative profit periods get shorter and eventually disappear. However if you're still not in the profit zone after 6-12 months and your trade returns still deviate largely from the predicted performance, as visible from a low [CBI](ddscale.htm), something is wrong. The strategy might be expired and you should stop trading it. 

![Z5 profit curve](../images/z5zulu0.jpg)  

Events - such as entering and exiting trades, or trailing the stop loss - are displayed in Zorro's message window (see [Log](log.htm)). Additionally, the current trading status, open trade list, and equity curve is displayed on the [status page](trading.htm). You can configure that page to be visible under a particular URL and in this way observe your system online from everwhere. If you have multiple accounts with different brokers, you can use the **[ZStatus](trading.htm#zstatus)** script for displaying the profits of all of them on a single panel. 

![](../images/zstatus.png)

# Z Systems and Algo Trading: Frequently Asked Questions

**What is the Z systems with the best profit-to-risk ratio?**

Z9.

**Can I stop and start a Z system to any time**?

Technically yes, when the broker server is online. But since most systems download recent price data at start for filling the **LookBack** period, avoid starting them at weekends when recent prices are not available and broker servers can be offline. Otherwise gaps in the price data can cause reduced performance during the following week. Once a system is trading, stop it only in case of an emergency. Most systems do not enter trades immediately after being started, so frequent stopping and starting will reduce the performance. While a system is stopped, it can not react on the market, which bears the danger of a potential loss when positions are still open. 

**Must I train the Z systems?**

No. The portfolio rotation and options trading systems have no parameters to be trained. The other Z systems must be re-trained every 6..12 months, but we do that with any Zorro update. So you need not train them yourself. If you want, train them on your own risk with Zorro S by clicking the [Train] button. The last WFO cycle with the parameters for live trading is then retrained. The factors file (**Z12.fac**) of Z1, Z2, Z12 and the rules files of Z7 cannot be retrained by the user.

**Why does the backtest not go up to the current date?**

In [walk-forward optimized](numwfocycles.htm) algo trading systems, such as Z1-Z7 and Z12, backtest and training cycles must be in sync and end at the same date. Otherwise the backtest would produce wrong results. For the other systems the backtest ends at the latest date of the available price and/or options data. 

**When can I withdraw profits?**

All strategies have long drawdowns that are listed in the table above (**max drawdown**). Don't withdraw profits when you're in a drawdown, since this would get you closer to a margin call. Remargin your account when your equity runs dangerously low. If you're winning, you can and even must remove profits regularly from your account, since the free Zorro version will stop trading when your balance exceeds its capital limit (use [Zorro S](restrictions.htm) for no limit). For the maximum amount to reinvest or to withdraw during a trading session, observe the [square root rule](tutorial_kelly.htm). 

**Can I trade with very little capital?**

For trading with low capital, such as a few hundred dollars, move the **Capital** slider mostly to the left and run repeated tests until the value displayed under **Capital** meets your needs. All margin based Z systems use the [ACCUMULATE](mode.htm) flag and thus allow trading with very low capital, although trades are then frequently skipped. Alternatively, reduce the capital requirement by removing assets from the portfolio, as described below. Note that low capital and few assets will normally reduce the live performance and increase risk and drawdowns due to the reduced number of trades. The setting of the **Capital** slider is unrelated to the displayed **"Risk"** of a trade, which does not reflect the strategy risk, but the stop loss distance.

**How can I add new assets or exclude assets from trading?**

Z8, Z9, Z10 can be used with any portfolio of stocks and ETFs. The other systems are optimized for a particular set of assets; you can not add, but you can exclude assets from trading.  For excluding a certain component from trading in Z1, Z2, Z12, or Z7, edit **Data\Z12.fac** or **Data\Z7.fac** and place minus signs '**-** ' in front of its **OptimalF** factors. The algorithms with the **ES** , **HU** , **LP** , **LS** , **VO** identifiers belong to Z1, **A2** , **BB** , **CT** , **CY** , **HP** to Z2, and **PA** , **PD** , **PDD** , **PG** to Z7. An example for excluding the CT algorithm with the USD/CAD asset: 

```c
...
USD/CAD:CT          -.041  1.32  263/420
USD/CAD:CT:L        -.029  1.25  177/227
USD/CAD:CT:S        -.058  1.38   86/93
...
```

Components with a negative or **.000** **OptimalF** factor are automatically excluded this way. Be aware that excluding the least profitable assets or algorithms will generate a too optimistic backtest result due to [selection bias](testing.htm#bias). 

**My broker uses different symbols - how can I rename assets?**

Use an [asset list](account.htm) that contains the symbols and and account parameters of your broker. Enter the asset name used by the broker in the **Symbol** column of the list, as described [here](account.htm). Do not modify the asset names in the first column, since they are used by the strategies. 

**Can I 'fire and forget' an automated trading system?**

Better not. Check the system's [status page](trading.htm) often, like once every day. Know your broker's trading platform for being able to observe and manually close trades anytime in case of an emergency, ideally from your mobile phone. If a trade was not correctly opened or closed due to a software glitch - this happens rarely, but it can happen - know how to deal with the problem. If it was a glitch of the broker's server or software, know how to contact the broker and request a refund. If it was a [bug in your script](trouble.htm), you'll get our compassion, but no refund. 

**Can I trade several Z systems together?**

You'll need[ Zorro S](restrictions.htm) for trading several automated trading systems at the same time. Make sure with your broker that your account is set up to allow multiple sessions. Some brokers set up accounts for single sessions only by default. Since Zorro stores its logs, trades, slider positions and other data in files named after the script, **do not** trade the same script with several Zorro instances in parallel! For doing that, either give the scripts different names, or run them from different Zorro installation folders. 

**How does algo trading affect my tax declaration?**

In most countries you have to pay tax for trading profits. Normally the broker keeps records of your trades that you can download from his website. Otherwise Zorro also records all trades in the **trades.csv** spreadsheet in the **Data** folder for your tax declaration. Interest (rollover) and profit/loss are recorded separately. Usually you have to submit this spreadsheet or the broker's records together with your tax declaration. Note that sometimes you cannot deduct trading losses from trading profits ([Scholz Tax](https://financial-hacker.com/the-scholz-brake-fixing-germanys-new-1000-trader-tax/)). 

**How do I update a live trading system?**

We'll publish updates to the Z strategies in regular intervals, so it's recommended to visit the user forum or the [Zorro website](restrictions.htm) from time to time. If you checked the box on the download form, you'll also be informed by email about updates. Please look [here](trading.htm#update) for how to move from an old to a new version with no interruption to trading. 

**Can an algo trading system become permanently unprofitable**? 

Yes. Any strategy can eventually expire. For instance, there was a **Z5** system that expired when the Swiss removed the CHF cap. If we suspect that a certain system will expire, we'll announce that on the forum and will also inform subscribers and Zorro S users by email. Normally we'll provide a successor after a system expired. As long as the markets are not perfectly effective, possibilities for profitable trade algorithms are almost infinite. 

**Can I get the source code of a Z system?**

For reasons described above, the Z systems are not available in source code - not even when you license the Zorro platfrom source. However, since the basic algorithms are published, you can hire a programmer (or our [development service](https://zorro-project.com/development.php)) for programming a replica of a Z system. The replica will trade similar, but not identical to the original system.

### See also:

[Strategy](strategy.htm), [Testing](testing.htm), [Trading](trading.htm), [Links](links.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
