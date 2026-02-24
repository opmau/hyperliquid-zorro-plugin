---
title: Margin, Risk, Lots, Capital
url: https://zorro-project.com/manual/en/lots.htm
category: Functions
subcategory: None
related_pages:
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [set, is](mode.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [Spread, Commission, ...](spread.htm)
- [Log Messages](log.htm)
- [Performance Report](performance.htm)
- [Status flags](is.htm)
- [Trade Statistics](winloss.htm)
- [TrainMode](opt.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [enterLong, enterShort](buylong.htm)
- [Live Trading](trading.htm)
- [Monte Carlo Analysis](montecarlo.htm)
- [Equity Curve Trading](trademode.htm)
---

# Margin, Risk, Lots, Capital

# Trade parameters 3 - investment control

The trade size - the number of contracts or currency units purchased - can be determined in four different ways. For directly ordering a certain number of contracts, use **Amount** or **Lots**. For investing a certain sum of money, use **Margin**. For risking a certain sum of money by determining the worst case loss, use **Risk**. 

## Lots

Trade size given by an integer number of lots (default = **1** ; max = [LotLimit](pip.htm)). 1 lot is defined as the smallest possible order size of the selected asset. Thus, a lot can be a multiple or a fraction of a contract, determined by [LotAmount](pip.htm). Since it's the minimum, the trade size can never be less than 1 lot (see remarks). In binary trading mode ([BINARY](mode.htm) flag) or if **Margin** is used for the trade size, **Lots** determines the minimum number of lots to be opened per trade (normally **1**). If **Lots** is **0** , no trades are opened. 

## Amount

Alternative fractional trade size in multiples or fractions of 100,000 units for currencies, and of 1 contract or 1 unit for anything else (default = **0** = trade size given by **Lots**). This number is independent of the broker's minimum order size and is similar to a 'MT4 lot' for currencies (see remarks). If a nonzero **Amount** amounts to less than one lot, 1 lot is opened. 

## Margin

Alternative trade size by invested margin, in units of the account currency (default = **0.00000001** for always opening at least 1 lot). With no leverage, **Margin** is simply the money invested to purchase the asset. On a leveraged account, **Margin** is a part of the trade size - for instance 1% at 1:100 leverage - deposited for opening a trade. As long as the trade is open, the margin amount is locked on the account and can not be used for further trades. If **Margin** is **0** or negative, no trades are opened. Otherwise the trade size is **Margin** divided by [MarginCost](pip.htm).   
The **Lots** variable still determines the minimum number of lots to be opened per trade. If the trade size by **Margin** is smaller than **Lots** and the **[MARGINLIMIT](mode.htm)** flag is set, trades are skipped. If the [ACCUMULATE](mode.htm) flag is set, the size of skipped trades is accumulated until it reaches the **Lots** amount. Keep **Margin** at its default value for controlling the number of lots only with the **Lots** variable.   

## Risk

Alternative trade size given by the trade risk in units of the account currency (default = **0** = no risk limit). The risk is the theoretical maximum that a trade can lose; it is determined from trade size, [stop loss](stop.htm) distance, [commission](spread.htm), and [spread](spread.htm) (formula: RiskPerLot = PIPCost /PIP * (Spread + abs(Stop-Price)) + CommissionPerLot; Margin = **Risk** /RiskPerLot * MarginCost). Since risk is undefined when a trade has no stop loss, the **Risk** parameter must always be given in combination with [Stop](stop.htm). If the risk of a trade at the given **Margin** is higher than the **Risk** variable, the trade margin is accordingly reduced.   
When the **[RISKLIMIT](mode.htm)** flags is set, trades are skipped when even with a trade size of 1 lot the trade risk is still higher than twice the given **Risk** value. When **Margin** is not set, the trade size is only limited by **Risk** ; this can lead to extreme trade sizes and subsequent margin calls when the [Stop](stop.htm) distance is tight. Due to minimum lot amounts and/or deviations of entry price and current price, the real trade risk displayed in the [trade log](log.htm) can deviate from the set up **Risk** value, especially with tight [Stop](stop.htm) distances. 

## Capital

Initial invested capital in units of the account currency (default = **0** = no initial capital). This has no direct effect on trading, but on calculating the strategy [performance](performance.htm) in the simulation. Set this to the initial capital when the strategy reinvests profits; Zorro then calculates **CAGR** instead of **AR/ROI** and determines performance parameters from the initial invested capital instead of the required capital. If the free capital (**Capital** minus loss minus total margin on leveraged accounts) becomes negative, Zorro will set the [MARGINCALL](is.htm) flag and liquidate trades for reducing the margin. Make sure to set **Capital** well above the [required capital](performance.htm#capital) on leveraged accounts, and to reinvest in a way that margin calls are avoided. Setting **Capital** to a negative amount assumes temporary remargining and disables trade liquidation. 

## ScholzBrake

Global loss limit in units of the account currency (default = **0** = no loss limit). This limit was enforced by a special 2021 German tax law invented by finance minister Olaf Scholz. The law applies taxes on all winning trades, while for losing trades only EUR 10,000 per year can be deducted. It is meanwhile revoked due to its blatant nonsense, but if ever such a law appears again, set **ScholzBrake** to the maximum allowed loss at session start and at the begin of any year. All further losses will then decrease it until it reaches zero. Trades are then suspended until **ScholzBrake** is set again. In [Trade] mode the **ScholzBrake** variable is decreased by all losses of Zorro instances that have set it and are trading on real accounts on the same PC. It is updated once per bar and at any trade. See also [LossGlobal](winloss.htm) and [Scholz tax](performance.htm). 

### Type:

**var**   

## MaxLong

## MaxShort

Maximum number of open and pending long or short trades with the same asset and algo. If the limit is reached, [enter](buyLong.htm) calls do not enter more trades, but they still close reverse positions and update the entry, stop, profit target, life time, and entry time parameters of open or pending trades to the values that the new trade would have. If set to a negative number, open or pending trades are not updated. The maximum numbers are ignored in [Train] mode unless the [LIMITS](opt.htm) flag is set. 

### Type:

**int  
**

### Remarks:

  * **Lot** has different meanings dependent on platform and broker. Normally, 1 lot is the smallest order unit. The trade size is always an integer number of lots, and there is no such thing as a 'fractional' or 'partial' lot. The lot amount - the number of contracts or units equivalent to one lot, available through the [LotAmount](pip.htm) variable - depends on the broker, the account, and the asset type. Forex brokers can offer mini lot and micro lot accounts. 1 mini lot is equivalent to 10,000 contracts and about $100 margin; 1 micro lot is 1000 contracts and about $10 margin. On stock broker accounts, the lot amounts and margins for forex trading are usually higher and the leverage is smaller. For CFDs, some brokers offer lot sizes that are a fraction of one contract (f.i.1 lot = 0.1 contracts). For options and futures, the lot size is determined by the [Multiplier](contracts.htm). Brokers normally list their trade parameters, including the lot amount per asset, on their websites or trading platforms.  
In some platforms, the term **'lot'** has a special meaning. In the MT4™/MT5™ platforms, 1 'MT4 Lot' is 100000 Forex contracts, or various amounts of other contracts (example [ here](https://www.hercules.finance/faq/is-lot-sizecontract-size-different-for-each-cfd-symbol-on-hotforex-mt4/)). Except for Forex, 1 MT4 Lot can mean a different trade size dependent on the broker, and is not equivalent to the **Amount** variable, which is always 1 contract. For entering forex trade volume in MT4 Lot units, the **Amount** variable can be used. Forex MT4 lots can be converted to real lots with the formula **Lots = MT4Lots * 100000/LotAmount**. 
  * The margin per lot can be determined with the **[MarginCost](pip.htm)** variable. The risk of a trade - the maximum possible loss at a given [Stop](stop) distance - is **Lots * (Stop** /**PIP) *[PIPCost](pip.htm)**. The number of lots equivalent to a given margin is **Lots = Margin/[MarginCost](pip.htm)**. 
  * All parameters that affect subsequent trades, such as **Margin** , **Risk** , **Lots,** must be set before calling [enterLong](buylong.htm) / [enterShort](buylong.htm). The number of lots, the current price, and the risk is displayed in Zorro's message window when an order is placed (see [Trading](trading.htm)). 
  * **Margin** , **Risk** or **Lots** could be set up in real time with a slider (see script example). This allows to quickly adapt the trade risk to the market situation, or to disable trades temporarily in case of a market crash. When either **Margin** or **Lots** are zero, no trade is opened. 
  * Because orders can only be placed in a multiples of one lot, the actual margin can be bigger or smaller than the given **Margin** value. When the **[MARGINLIMIT](mode.htm)** flag is set, trades are not executed when the required margin is more than twice the **Margin** value; increasing **Margin** will then increase both the trade size and the number of trades, while increasing **Lots** only increases the trade size. 
  * The **Capital** variable is used for determining the effectivity of the reinvestment algorithm. It calculates performance parameters based on the given capital, not on the equity curve, and thus ignores the setting of the Monte Carlo **[Confidence](montecarlo.htm)** level. 
  * **ScholzBrake** affects only the opening of trades, so the loss limit can be exceeded by trades that are still open when it is reached. For preventing taxes, set **ScholzBrake** to a value slightly below the EUR 10,000 loss limit. 
  * In [Train] mode, trades always open 1 lot, phantom trades are converted to normal trades, and **MaxLong** /**MaxShort** have no effect. This way all trades equally contribute to the training result. This behavior can be modified with the [TrainMode](opt.htm) variable. 

### Examples:

```c
// set margin from slider
Margin = slider(1,500,0,2000,"Margin","Average margin in $");

// activate the Scholz Brake
if(is(INITRUN) || year(0) != year(1))
  ScholzBrake = 9900;
```

### See also:

[enterLong/Short](buylong.htm), [Stop](stop.htm), [Spread](spread.htm), [PIP](pip.htm), [PIPCost](pip.htm), [MarginCost](pip.htm), [Win/LossPayout](spread.htm), [TradeMode](trademode.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
