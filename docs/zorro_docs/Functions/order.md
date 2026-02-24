---
title: order
url: https://zorro-project.com/manual/en/order.htm
category: Functions
subcategory: None
related_pages:
- [keys](keys.htm)
- [printf, print, msg](printf.htm)
- [trade management](trade.htm)
- [Brokers & Data Feeds](brokers.htm)
- [enterLong, enterShort](buylong.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [login](login.htm)
- [DLLs and APIs](litec_api.htm)
- [Broker API](brokerplugin.htm)
- [Functions](funclist.htm)
---

# order

## order (int type): int 

User-supplied function that is called by the trade engine every time when Zorro enters or exits a trade. Can be used to access external broker APIs, control external trade platforms by sending [key strokes](keys.htm) or mouse clicks, or just pop up a [message](printf.htm) for manually entering or exiting a trade. 

### Parameters:

**type** \- **1** for entering, **2** for exiting a trade. **  
**

### Returns:

**0** when the trade could not be opened or closed, **1** otherwise. 

### Remarks:

  * If an **order** function is defined in the script, it replaces the trade functions of the selected broker plugin.
  * All [trade variables](trade.htm) f.i. for determining the asset and algo are available in the order function.

### Example:

```c
int order(int type)
{
  string bs = "Buy";
  if(type == 2) bs = "Sell";
  string ls = "Long";
  if(TradeIsShort) ls = "Short";
  play("alert.wav");
  return msg("%s %s!",bs,ls);
}
```

### See also:

[Brokers](brokers.htm), [enterShort/Long](buylong.htm), [exitShort/Long](selllong.htm), [msg](printf.htm), [login](login.htm), [API](litec_api.htm), [broker plugin](brokerplugin.htm), [user supplied functions](funclist.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
