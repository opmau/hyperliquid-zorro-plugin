---
title: Brokers, VPS
url: https://zorro-project.com/manual/en/brokers.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Spread, Commission, ...](spread.htm)
- [Licenses](restrictions.htm)
- [FXCM](fxcm.htm)
- [Oanda](oanda.htm)
- [IG](ig.htm)
- [Dukascopy](dukascopy.htm)
- [Broker API](brokerplugin.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [order](order.htm)
- [Asset and Account Lists](account.htm)
- [Broker Arbitrage](brokerarb.htm)
- [The Z Strategies](zsystems.htm)
- [Getting Started](started.htm)
- [Trading Strategies](strategy.htm)
---

# Brokers, VPS

# Zorro and the Brokers 

Any algorithmic trading strategy needs a connection from the Zorro platform to a broker or exchange for buying or selling assets. Normally the broker also provides live prices, price history, and other data such as order book content or option chains. Most brokers offer free demo accounts (also called practice, paper, or game accounts) where trading can be tested without risking real money. A demo account with a broker can usually be opened in 5 minutes on their website. In most cases you'll get an MT4 account, and can start trading with Zorro's MT4 bridge. Opening a real account for trading with real money takes longer, as the broker has to confirm your identity through some ID and address verification process. With serious brokers, be prepared to fill in lengthy forms about your risk preferences and prior trading experiences.

### Finding a broker 

Every broker uses a different trade model, offers a different universe of assets, and has different values for spread, commission, and other asset and account parameters. Here are some simple criteria for selecting the best broker for small-budget algo trading:

  * **Market Maker / No Dealing Desk (NDD).** If a broker is a 'market maker', they take the other side of your trades. So you're trading against the broker, not against the market. Obviously, those broker do not want you to win, even though they claim otherwise. Market maker brokers are quite common for Forex, CFD, or binary options. Brokers with 'no dealing desk' (also referring to themselves as **ECN** "**E** lectronic **C** ommunication **N** etwork" or **STP** "**S** traight **T** hrough **P** rocessing" brokers) claim to have no such conflict of interest, since they transfer your orders to **exchanges** or **liquidity providers**. Therefore NDD/ECN/STP brokers are preferable to Market Makers. At least in theory*. 
  * **Initial capital.** Some brokers require a minimum initial deposit, such as $10000. Theoretically you can immediately withdraw 90% of your deposit when the account is opened. Still, it is preferable to select a broker with small initial capital requirement. 
  * **Lot size.** Determines you minimum forex order volume. The smaller, the better. Many brokers offer micro lot accounts, where 1 lot is equivalent to 1000 forex units. Some offer nano lot accounts (1 lot = 100 contracts), and some, such as Oanda, have no minimum lot size at all.
  * **Leverage.** The higher, the better, because you can always reduce your leverage, but not increase it. Accounts in the US have often lower leverage than in the rest of the world. Most brokers offer about 30:1 .. 100:1 for Forex and CFDs, and 2:1 for stocks and ETFs.
  * **Spread and Commission.** Naturally, the smaller the trading costs, the better. Especially short-term strategies only work below a certain trading cost level. For comparing costs, a formula for commission -> spread conversion can be found under [Commission](spread.htm). 
  * **Customer service.** It is incredible what sorts of request and complaints come in daily on a broker's service desk. Still, the service must react promptly and solve problems professionally. You should be able to contact the service anytime via email or chat.
  * **Payment.** In case you need to remargin quickly, the broker should offer a fast way to deposit money. Bank wire transfers can take days, money transfer via Paypal or credit card only minutes until the balance appears on your account. Of course, the broker should also offer a fast way to withdraw profits.
  * **Free subscription.** Some brokers provide a free Zorro S subscription with a new account; look under [Zorro S](restrictions.htm) for details.

All brokers listed under '**Affiliate** ' on the [Zorro download page](https://zorro-project.com/download.php) fulfill the above criteria. If the broker offers the choice between several account types, select the account with the **smallest lot size** and the **highest leverage**. Maybe you've read in a trading book to avoid high leverage as it implies "high risk". Not so: A high leverage account just means that you're free to determine your own leverage. Risk comes from trading a too big volume, not from a high leverage account. Small minimum lot sizes are preferable as you can trade with less capital and can better adjust the trade volume. For low-budget forex trading strategies, a micro lot or nano lot account is mandatory. 

* In reality, NDD forex brokers have been found to be in control of their main liquidity provider and in this way indirectly hold positions against their clients. Some NDD brokers are rumored to classify their clients in categories A and B, depending on their trade success. Trades from A-clients - usually, algorithmic traders - are transferred to liquidity providers since they tend to win. Trades from B-clients - the vast majority - are not. You should aim for being an A-client.

### Connecting to a broker 

Zorro can trade with all brokers that offer at least one of the following ways connecting to them:

  * A trading API. That's a software library with direct access to the broker's price and trading servers. The free Zorro version comes with trading API modules for several major brokers, such as **[FXCM](fxcm.htm)** , [Oanda](oanda.htm), [IG](ig.htm), or [Dukascopy](dukascopy.htm) (more broker API modules are available with [Zorro S](restrictions.htm)). Just enter your user ID and password on the Zorro panel and you'll be automatically connected. Any broker API can be relatively easily implemented with some programming knowledge through a [Broker Plugin DLL](brokerplugin.htm).   

  * **"Meta"-Trader 4/5** trading platform. Install the [MT4/5 Bridge](mt4plugin.htm), enter your account number in the Zorro user ID field and you'll be automatically connected. Zorro runs in this mode as an "Expert Advisor" (EA) on the platform and can thus trade with any broker that supports that popular platform. This way you have thousands of brokers to choose from. If you have the choice, MT4 is normally preferable over MT5.  

  * A web based trading platform, or a trading program where trades can be simply placed with key strokes or mouse clicks. Zorro can control other programs by sending key or mouse commands to their user interface. In this mode, Zorro can retrieve the current price data from a free demo account with a supported broker A, and uses the **[order](order.htm)** function to hit buttons on the web based trading platform from your real broker B.  

  * If your broker does not support any of those methods, you can trade manually by using Zorro as a signal provider. For this, run your strategy on a free demo account of a supported broker A, and enter or exit a trade with your real broker B when you see or hear that Zorro opens or closes a trade. If your strategy has not a too-short time frame, it won't matter much when a short delay lies between Zorro's and your trade entry. Zorro plays the sound file **trade.wav** each time when it enters a trade, and **win.wav** / **loss.wav** each time when it closes it. Those files are located in the **Zorro** main folder, and you can replace them with an alarm sound if you want to get alerted every time for entering a trade. Alternatively, Zorro can write trade signals to a spreadsheat or other file for externally triggering trades.

### Backtesting a broker

In backtests, Zorro can simulate any broker and account through [asset specific parameter sets](spread.htm). The broker and account specific parameters, such as rollover fee, spread, lot size, pip cost, etc. are taken from a spreadsheet file in the **History** folder. This file can be either downloaded from the broker API, or edited with Excel or with the script editor for simulating different accounts and assets. For details see [Asset List](account.htm). 

### Taking advantage of a broker

Market maker brokers can not only play tricks on you, you can also play tricks on them. One of the most favored is [broker arbitrage](brokerarb.htm). Since Zorro S can trade with several brokers at the same time, you can compare asset prices from broker A with the same asset from broker B, and enter a long position with the cheap broker and a short position with the other. You can this way collect risk-free profits when the price difference must temporarily exceeds the spread and transaction costs. Of course, brokers get upset when they learn about those practices. So you should not actually brag about them on trader forums.

### Trading cryptocurrencies at a digital exchange

The [Z10 strategy](zsystems.htm) is a passive portfolio management system for cryptocurrencies. Trading with Ethereum, Ripple, or other digital currencies is a bit different to trading with normal assets, since you normally need bitcoin for this. Some digital exchanges accept funding in Bitcoin only; you'll then need to purchase bitcoin first for opening an account with them. Here are the steps:

  * Register with a **bitcoin exchange service** where you can buy or sell bitcoin for dollars or Euro; examples: Coinbase™ or Bity™. Register there, enter your credit card or bank account data, and buy bitcoin. Credit card transfers are usually more or less instant, bank transfers can take some days.
  * Exchanges can host your money, but if you don't trust them, get a **wallet** for storing your bitcoin treasure on your PC**.** Example: Copay™. To transfer coin from your exchange account into your wallet, it displays a receiver address that you enter at the service's website.
  * Register an account with a **digital exchange** , like Bittrex™ or Binance™. For depositing bitcoin, the exchange shows you a receiver address to which you send coin from your wallet, or that you can use as withdrawal address for the bitcoin exchange service.
  * As soon as the bitcoins appear in your account at the exchange, you can fire up Z10 and start trading.
  * For withdrawing your bitcoin profits, take the above steps in opposite direction. Needless to say that any step usually requires a small bitcoin transaction fee.

### See also:

[Zorro](started.htm), [Strategy](strategy.htm), [account list](account.htm), [broker plugin](brokerplugin.htm), [MT4 plugin](mt4plugin.htm), [broker arbitrage](brokerarb.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
