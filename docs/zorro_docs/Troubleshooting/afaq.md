---
title: F.A.Q.
url: https://zorro-project.com/manual/en/afaq.htm
category: Troubleshooting
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [Links & Books](links.htm)
- [Asset and Account Lists](account.htm)
- [Migrating Scripts](conversion.htm)
- [Historical Data](history.htm)
- [W4a - Indicator implementation](tutorial_lowpass.htm)
- [Conversion from C++, C#](litec_c.htm)
- [Dataset handling](data.htm)
- [Troubleshooting](trouble.htm)
- [Variables](aarray.htm)
- [Trade Statistics](winloss.htm)
- [trade management](trade.htm)
- [for(open_trades), ...](fortrades.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [tick, tock](tick.htm)
- [Bars and Candles](bars.htm)
- [BarMode](barmode.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Time zones](assetzone.htm)
- [series](series.htm)
- [shift](shift.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [File access](file_.htm)
- [HTTP functions](http.htm)
- [assetHistory](loadhistory.htm)
- [IB / TWS Bridge](ib.htm)
- [Oanda](oanda.htm)
- [FXCM](fxcm.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [enterLong, enterShort](buylong.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Virtual Hedging](hedge.htm)
---

# F.A.Q.

# Support - Frequently Asked Questions

When you're a Zorro [Sponsor](restrictions.htm), you are entitled to a time period of free technical support by email. Otherwise you can post your question on the user forum, or subscribe a support ticket in the web store. For technical support please email your question to**support (at) opgroup.de** , and give your support ticket number or your user name that is displayed in Zorro's startup message. 

General questions about financial trading are answered on the [Trading FAQ page](https://zorro-project.com/faq.php). The most frequent technical questions are listed below. 

### General questions

Q. I am a C programmer and have read the Zorro tutorial. Does it still make sense for me to get the Black Book or take the Algo Bootcamp?.  
A. Depends on your experience with strategy development. The [Black Book](links.htm) and especially the [Algo Bootcamp](https://zorro-project.com/docs.php) cover all details involved in writing a trading strategy.

Q. How do I **search** for a certain topic in the manual that pops up when I click on [Help]?  
A. Click on [Search]. 

Q. Can I copy the Zorro folder on a **USB drive** and run Zorro from there?   
****A. Yes. Make sure that you have full access rights to that drive.

Q. How can I  add **another asset** to the Asset scrollbox?  
A. Open the **History\AssetsFix.csv** spreadsheet with Excel or with the script editor, and add a new line with the parameters of the new asset. The details are described in the manual under [Asset List](account.htm). For backtesting that asset you'll also need to download its price history.

Q. Why do I get  different results for the trading systems from the tutorial / the Black Book?  
A. Because you're testing them probably with different [ asset parameters](account.htm). Margin, leverage, and transaction cost change from week to week and from broker to broker. 

Q. Why do I get  different backtest results with Zorro than with my other trading platform?  
A. Read [here](conversion.htm) why.

Q. How can I get  historical data for training and backtests?  
A. Recent data for the Z systems is available on the [Download page](https://zorro-project.com/download.php). Forex, index, or EOD data can be downloaded with the [ Download script](history.htm). M1 data for US stocks and EOD options data can be purchased from [ info@opgroup.de](mailto:info@opgroup.de?subject=Historical data request). 

Q. How can I use an MT4 **expert advisor** with Zorro?  
A. Read [here](conversion.htm) how to convert MT4 indicators or "expert advisors" to Zorro scripts. 

Q. I have the C source code of an  indicator. How can I use it with Zorro?  
A. Coding indicators is described in [workshop 4a](tutorial_lowpass.htm) and in Petra's articles on [Financial Hacker](https://financial-hacker.com/). The plain C code can normally run unchanged, but you need to remove its framework, such as main() functions, DLL export declarations, or C library headers. You can find an example [here](conversion.htm#c). Read [here](litec_c.htm) about the differences between standard C code and Zorro's "lite-C" code. 

Q. Sometimes I edit a strategy, but the changes I made are **not reflected** in the test. Sometimes I can't even save my changes, and cannot find the **.log** file or the **.csv** trade history in the Log folder. What happened to the changed files?  
A. You have probably installed Zorro not in the recommended folder, but in a folder with only limited access (such as Program Files). 

Q. How can I compile my script to an **executable** for avoiding to have its source code on a server?  
A. Select **Compile to Exe** in the [Action] scrollbox. Your script will be compiled to a machine code **.x** file from which the source code can not be retrieved anymore. You need [Zorro S](restrictions.htm) for this. 

Q. I have my **own set of signals** in an Excel file with time stamps. How can I test them with Zorro?  
A. Export them from Excel to a **.csv** file. In your Zorro script, import them in a dataset with the [dataParse](data.htm) function. Then use the [dataFind](data.htm) function at any bar to find the record matching the current date/time, and retrieve your signals from the record. 

Q. I made a  small change to my script, but now get  totally different results in the backtest. Surely this must be a Zorro bug?  
A. You can quickly find out by following the procedure under [Troubleshooting](trouble.htm). Generate logs from both script versions and compare them. Especially, compare the first two trades from start to end. You'll normally see then immediately the reason of the different results. 

Q.  My script **does not work**. I get: "Error 111 - Crash in run()". I don't know what's wrong - how can I fix it?  
A. Finding and fixing bugs is also described under [Troubleshooting](trouble.htm). 

Q. It does not still work. I have a support ticket - please  ** fix my script!**  
A. Zorro Support can answer technical questions, help with technical issues, and suggest solutions. But we are not permitted to write, modify, analyze, debug, or fix user scripts. If you need someone to debug and fix your code, we can put you in contact with a programmer. 

Q. What's the **price for coding** a strategy?  
A. Depends on the strategy. Development fees start at **EUR 170** for a simple indicator-based strategy. Complex options trading strategies or machine learning systems can be in the **EUR 1000** or **2000** range. Send a description of your system and you'll get a quote for the development. The simpler the system, the smaller the cost. If you want, download our [NDA](https://zorro-project.com/op_NDA.pdf) before.

### Strategy development

Q. I'm using global variables in my script. When I run it the second time, it **behaves differently** than the first time.  
A. Static and global variables keep their content until the script is modified or compiled. If necessary, set them to their initial values on script start, like this: **if(Init) MyStaticVar = 0;**. Read more about local, global, and static variables [here](aarray.htm). 

Q. How can I find out  if the last trade was **a winner or a loser**?  
A. If [ LossStreakTotal](winloss.htm) is **> 0**, the last trade was lost.

Q. How do I place two pending orders so that the order that's opened first **cancels the other** one?  
A. Place the first pending order, and check in its [TMF](trade.htm) if the entry condition of the second order is fulfilled. In that case enter the second order at market, and close the first while it's still pending. 

Q. How can I count the **number of winning trades** within the last N trades?  
A. Use a **[for(all_trades)](fortrades.htm)** loop, and count the number of closed trades. As soon as the number is above the total number of closed trades (**[NumWinTotal+NumLossTotal](winloss.htm)**) minus **N** , start counting the number of winning trades.

Q. How can I place two different profit targets so that **half of the position** closes when the first one is hit, and the rest on the second?  
A. The best and simplest solution is opening two trades with two different profit targets, each with half the lot volume. If your broker does not allow opening several trades at the same time, use [OrderDelay](timewait.htm) for delaying the second trade a few seconds. 

Q. How can I run a function **on every price tick** of a certain asset?  
A. Use a [tick](tick.htm) function. It will be executed every time when a new price quote arrives. Note that the tick execution frequency will be different in real trading and in the backtest, due to the fewer price ticks in M1 price data. If this is an issue for the particular strategy, use [t1](history.htm) price data. 

Q. My candles appear  time shifted compared to the same candles in another trading platform.  
A. The other platform probably uses a different time zone, and/or timestamps from the candle open prices. Zorro uses UTC time and timestamps from the candle close prices. Read [here](bars.htm) about bars and time stamps. 

Q. How can I skip all bars that fall **outside market hours**?  
A. Bars are normally added whenever price quotes of the traded assets arrive from the broker API, regardless of the market hour. For skipping them, either set the [BR_MARKET](barmode.htm) flag, or use the [TimeFrame](barperiod.htm) mechanism with individual [AssetMarket](assetzone.htm) time zones. If you only want to skip bars for a certain indicator, you can use a [static series](series.htm) and [shift](shift.htm) it during market hours.

Q. How can I **store variables** so that a new trading session automatically starts with their their last values?  
A. Use [AlgoVar](algovar.htm) or [TradeVar](trade.htm) variables. They are preserved among sessions. If you have to store a lot more data, save and load them with [file](file_.htm) functions. 

Q. How can I **evaluate online data** that is available on a website?  
A. Use a [http](http.htm) function (for accessing data directly from a website) or the [dataDownload](data.htm) or [assetHistory](loadhistory.htm) functions (for retrieving data from a web service). Google, Stooq, AlphaVantage, Quandl, CryptoCompare, and many other services provide historical or current data for most existing assets and indices, so you can use them for live trading as well as in the backtest. 

### **Live trading**

Q. How many  Zorro S instances can simultaneously**** trade on a PC or VPS?   
****A. The number of  parallel  running programs on a PC is limited by the  available  memory, the internet bandwidth, and the CPU performance. In the case of Zorro, on an average  PC the limit is about  8..10 instances. 

Q. I'm getting **error messages** when I try to connect to IB / Oanda / FXCM / MT4.  
A. Read the manual about [IB](ib.htm) / [Oanda](oanda.htm) / [FXCM](fxcm.htm) / [MT4](mt4plugin.htm) etc for learning how to connect to your broker and which assets are available. The 3 most frequent reasons for connection failure are wrong login data (user / password), a wrong account type (demo / real) or a server outage (at weekends or holidays). The reasons of the errors are printed in the Zorro log or in the case of MT4 or MT5, in their "Experts Log". 

Q. I want to trade with the MT4 bridge, but my MT4 has **no Zorro EA**.  
A. You must install the MT4 bridge as described [here](mt4plugin.htm). Please also read the  [remarks ](mt4plugin.htm#remarks)that cover all known issues of MT4 or MT5. 

Q. Ok, I have now installed the  MT4 bridge, but it doesn't work. I'm getting error messages.  
A. You made a mistake with the installation.  Just try again. You will succeed when you follow the instructions in the manual – we guarantee that. You can repeat the MT4/MT5 bridge installation as often as you want.   
Since installing the bridge involves an unzip and copy operation on your PC, some basic PC knowledge is required. If you cannot do it, ask someone for assistance. Alternatively, oP group offers a [Zorro/MT4 installation](https://zorro-project.com/docs.php) service by remote access. This, also, has 100% success guarantee.

Q. I've set the **stop loss** in my script, but it does not appear in my broker's trading platform.  
A. Zorro won't tell your broker about your stop loss unless you set it up to do so. Read under [StopFactor](stop.htm) how a stop loss differs in Zorro and in your broker's platform.

Q. My system does **not close trades** in live trading. It worked in the backtest.  
A. In the US, NFA compliant brokers do not allow you to close Forex trades (no joke!); instead you have to open a position in opposite direction. Zorro handles this automatically in [NFA compliant mode](mode.htm#nfa).

Q. My live system does often **not open or close trades** \- - and I'm sure that I don't live in the US.  
A. There can be many reasons for rejecting orders, but unless you're trading outside market hours or with a very illiquid asset, it won't happen often. You can see the reason of the rejection in the error message from your broker (such as: "No tradeable price"). If an open order failed, [enterLong](buylong.htm) / [enterShort](buylong.htm) return **0**. 

Q. Where is the **live chart**? How can I visualize the price curve and the trade entries and exits in real time?   
****A. For live charts use your broker's trading platform. Zorro is purely for automated trading and does not display live charts, aside from the profit curve on the trade status page.

Q. The candles on the Zorro chart are **different to the candles** that I see on the MT4 chart, and the prices in the Zorro log are also different to the open/close prices in my platform log.   
**** A. Zorro displays generally ask prices in logs and on charts, while some platforms, f.i. MT4, display bid prices on charts by default. And your live chart is usually based on a different time zone and thus will display different candles. Check the help file of your platform about switching it to ask prices and UTC time zone.

Q. How can I **change the sounds** for entering, winning, or losing trades?  
**** A. In your Zorro folder you'll find sound files with names like "**win.wav** ", "**loss.wav** ", or "**open.wav** ". Replace them with your own sounds in **.wav** format. Use **Mute** in **Zorro.ini** for permanently disabling sounds.

Q. I run the same strategy with the same broker on two different VPS. Both versions open **different trades** although they should be identical.  
**** A. With the same broker and account you'll normally get rather similar trades. But they are not necessarily identical, especially not with Forex or CFDs since any trade entry and exit is based on a individual price quote. Differences in latency can also contribute to different trade entries and exits.

Q. I'm a Zorro S owner and use two Zorros for trading two strategies on the same real money account. In the moment when the second Zorro logs in to the account, the first one **logs off** and the server indicator gets red.  
**** A. Your account is limited to a single session by your broker. Contact your broker and ask for activation of multiple sessions for your account.

Q. Sometimes my system opens  trades that **do not appear** in the broker platform.  
A. Those are [phantom trades](lots.htm), used for equity curve trading or for [virtual hedging](hedge.htm). You can distinguish them in the log from real trades by their winged brackets **{..}** as opposed to the rectangular brackets **[..]** of real trades. 

### 

[► latest version online](javascript:window.location.href = 'https://manual.conitec.net' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
