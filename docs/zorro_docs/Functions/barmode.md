---
title: BarMode
url: https://zorro-project.com/manual/en/barmode.htm
category: Functions
subcategory: None
related_pages:
- [setf, resf, isf](setf.htm)
- [asset, assetList, ...](asset.htm)
- [set, is](mode.htm)
- [Dates, holidays, market](date.htm)
- [Time zones](assetzone.htm)
- [series](series.htm)
- [Asset and Account Lists](account.htm)
- [tick, tock](tick.htm)
- [brokerCommand](brokercommand.htm)
- [Error Messages](errors.htm)
- [Bars and Candles](bars.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Bar, NumBars, ...](numbars.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
---

# BarMode

## BarMode

Determines bar creation and trading behavior when the market is inactive, as on weekends, outside market hours, and on business holidays. By default, any bar contains at least one valid price quote. When no price quotes arrive due to market inactivity, or when it's weekend or a holiday, no bars are created. This behavior can be changed with the following **BarMode** flags. Use **|** or **+** for combining flags and [setf](setf.htm) and [resf](setf.htm) for setting and resetting single flags. 

### Flags:

**BR_FLAT** | Generate bars even when they contain no price quote. These bars get flat candles with the close price of the last quote. If that flag is not set, bars are only created where the first [asset](asset.htm) has valid prices. If set, use **BR_WEEKEND** / **BR_MARKET** and/or the [NOFLAT](mode.htm) flag for letting indicator series skip flat periods. This flag affects bar creation and must be set before the first [asset](asset.htm) call.  
---|---  
**BR_WEEKEND** | Don't don't generate bars and don't trade on [holidays](date.htm) or between **[EndWeek](date.htm)** and **[StartWeek](date.htm)** in the bar time zone ([BarZone](assetzone.htm)), even when they contain price quotes. This flag is set by default. It affects bar creation and must be set or reset before the first [asset](asset.htm) call.  
**BR_MARKET** | Don't on't generate bars and don't trade between [ EndMarket](date.htm) and [StartMarket](date.htm) in the bar time zone ([BarZone](assetzone.htm)), even when they contain price quotes. This flag is for intraday bars only. It affects bar creation and must be set before the first [asset](asset.htm) call.   
**BR_NOSHIFT** | Don't shift data [series](series.htm) on intraday bars that end at or after [ EndMarket](date.htm) and before [StartMarket](date.htm) in the bar time zone ([BarZone](assetzone.htm)). This prevents series-based indicators from updating or generating trade signals outside market hours. Start and end time and zone can be individually set up in the [asset list](account.htm).   
**BR_LOCAL** | Automatically set [Start/EndMarket](date.htm) on any [asset](asset.htm) call to the local asset market hours ([AssetMarketStart/End](date.htm)), converted from [AssetMarketZone](assetzone.htm) to [BarZone](assetzone.htm).   
**BR_LEISURE** | Don't enter or manage trades between [EndMarket](date.htm) and [StartMarket](date.htm), on holidays, and on weekends by **[EndWeek](date.htm)** /**[StartWeek](date.htm) **in the bar time zone ([BarZone](assetzone.htm)), even when bars are created during that time.   
**BR_SLEEP** | Don't request or process price quotes when the market is closed due to **BR_WEEKEND** and **BR_MARKET**. Since no prices are updated, [tick](tick.htm) functions will not run and price-dependent events will not to be triggered during that time. Use this for special purposes only.  
**BR_LOGOFF** | Log off on weekends or when the market is closed. **BR_SLEEP** and **BR_WEEKEND** must be set. Broker API [settings](brokercommand.htm), such as order texts or price or volume types, are usually reset when logging off. Use this for special purposes only, such as working around server maintenance or API unresponsiveness on weekends. Some broker APIs do not support logging off during a trading session; in that case **BR_LOGOFF** will suspend the session until it is manually restarted.  
  

### Type:

**int**

### Remarks:

  * When **BR_FLAT** is not set, the historical time stamps of the first [asset](asset.htm) in a portfolio strategy determine the bar creation. The order of assets does not matter for bar creation when **BR_FLAT** is set.
  * When **BR_MARKET** is set, make sure that the timestamp of at least one bar per day falls within market hours - otherwise the day will have no valid bar (see [Error 014](errors.htm)).
  * When **BR_LEISURE** is set, but the system is supposed to trade around the clock on working days, set **StartMarket = EndMarket = 0;**.
  * When no bars are generated due to weekends, holidays, or market closure, the [Status] window displays "Closed".
  * For trading assets even at weekends, such as cryptocurrencies, reset **BR_WEEKEND** before sampling bars.
  * When **BR_MARKET** or **BR_WEEKEND** are used, make sure that weekend and market hours are also set up correctly in UTC or in the **BarZone**. When multiple assets have different market hours, set them up before selecting the asset. 
  * For not updating prices after or before a certain time, use a [tick](tick.htm) function and the [NOPRICE](mode.htm) flag.

### Example:

```c
function run()
{
  ...
  StartWeek = 10400; // start Monday 4 am
  EndWeek = 51900; // end Friday 7 pm
  BarMode = BR_WEEKEND+BR_SLEEP; // don't process quotes during the weekend and don't generate bars
  ...
}
```

### See also:

[Bars](bars.htm), [BarPeriod](barperiod.htm), [NumBars](numbars.htm), [BarZone](assetzone.htm), [MinutesPerDay](lookback.htm), [AssetZone](assetzone.htm), **[StartWeek/EndWeek](date.htm), [Holidays](date.htm)**

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
