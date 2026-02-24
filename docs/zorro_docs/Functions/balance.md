---
title: Balance, Equity, TradeVal, MarginSum
url: https://zorro-project.com/manual/en/balance.htm
category: Functions
subcategory: None
related_pages:
- [Margin, Risk, Lots, ...](lots.htm)
- [Trade Statistics](winloss.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [Strategy Statistics](statistics.htm)
- [enterLong, enterShort](buylong.htm)
---

# Balance, Equity, TradeVal, MarginSum

# Account Parameters

## Balance

The current amount in the broker account. In live trading, this parameter is updated from the broker API if available; otherwise it is calculated from [Capital](lots.htm) \+ [Wintotal - LossTotal](winloss.htm). 

## Equity

The current value of the account including all open trades. In live trading, this parameter is updated from the broker API if available; otherwise it is calculated from **Balance +**[WinValTotal - LossValTotal](winloss.htm). 

## TradeVal

The current value of all open trades by the strategy. This is the profit or loss if all trades would be closed immediately. The value of all open trades of the account is **Equity - Balance**. 

## RiskTotal

The estimated maximum risk of all currently open trades. The risk is estimated as trade costs plus loss at either the initial stop loss distance, or at an 1% adverse price move when no stop is used. Due to trailing and exit mechanisms, the real risk is normally smaller than the estimated risk, but can also be higher in cases of large price moves or large exit slippage. 

## MarginTotal

The current allocated margin of all open trades, calculated through [MarginCost](pip.htm) under assumption of a constant margin requirement of open trades. This can be different in live trading when brokers adapt the margin requirement to the market situation or asset price. A margin call is simulated in test and training mode when [Capital](lots.htm) **> 0** and **Equity - MarginTotal** reaches zero. 

## MarginVal

The current allocated margin of the account. The account is threatened by a margin call when **Equity - MarginVal** approaches zero. In live trading, this parameter is updated from the broker API if available; in test and training it is identical to **MarginTotal**. 

### Range:

**Account currency**

### Type:

**var** , read/only 

### Remarks:

  * When connected to the broker, the variables **Balance** , **Equity** , and **MarginVal** are retrieved from the broker API and valid for the whole account, not only for the trades of the current script. Trades that are entered manually or openend by another Zorro instance contribute to them.
  * For simulating wins or losses that are not caused by trades, add the amount to [WinTotal](winloss.htm) or [LossTotal](winloss.htm).
  * The maximum risk and margin over all past bars is given by [RiskMax](statistics.htm) and [MarginMax](statistics.htm).

### Example:

```c
// stop trading when there's not enough money in the acount
if(IsTrading && Equity - MarginVal < 1000))   Lots = 0;else  Lots = 1;
```

### See also:

[enterLong](buylong.htm)/[Short](buylong.htm), [Win / Loss](winloss.htm), [Capital](lots.htm), [Statistics](statistics.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
