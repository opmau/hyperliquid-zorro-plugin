---
title: results
url: https://zorro-project.com/manual/en/results.htm
category: Functions
subcategory: None
related_pages:
- [Trade Statistics](winloss.htm)
- [for(open_trades), ...](fortrades.htm)
- [Strategy Statistics](statistics.htm)
---

# results

## results (int Mode, int Num): var

Sums up the results of the most recent trades by different criteria. 

### Parameters:

**Mode** | Type of the result sum, a combination of:**  
0** \- sum up profits  
**+1** \- return number of trades**  
+2** \- sum up entry/exit price differences  
**+3** \- return average entry price  
**+8** \- consider only open trades  
**+16** \- consider only closed trades  
**+32** \- consider only trades of current asset / algo combination   
**+64** \- consider only winning trades   
**+128** \- consider only losing trades  
**+256** \- consider only long trades  
**+512** \- consider only short trades   
  
---|---  
**Num** | Number of trades to consider. Enter a large number for all trades.  
  

### Remarks:

  * Pending trades or pool trades are not included in the sum, but phantom trades are included.
  * The trade open time is relevant for the trade sequence order.
  * Unlike [Trade statistics](winloss.htm) that are updated at any **TickTime** , **results** returns the current trade statistics immediately. 

### Examples:

```c
var NumWins = results(1+64,30);	// Number of winning and won trades
var NumLosses = results(1+128,30);	// Number of losing and lost trades
var Wins = results(2+64,30)/PIP;	// Total win in pips
var Losses = results(2+128,30)/PIP;	// Total loss in pips
var AvgProfit = ifelse(NumWins+NumLosses > 0,
  (Wins-Losses)/(NumWins+NumLosses),0.); // Average profit per trade
var WinRate = ifelse(NumWins+NumLosses > 0,
  NumWins/(NumWins+NumLosses);	// Win rate of last 30 trades
```

### See also:

[Trade statistics](winloss.htm), [for(trades)](fortrades.htm), [strategy statistics](statistics.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
