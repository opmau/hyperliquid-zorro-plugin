---
title: BarZone,AssetMarket,AssetFrame
url: https://zorro-project.com/manual/en/assetzone.htm
category: Functions
subcategory: None
related_pages:
- [Time / Calendar](month.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [asset, assetList, ...](asset.htm)
- [Dataset handling](data.htm)
- [brokerCommand](brokercommand.htm)
- [Asset and Account Lists](account.htm)
- [run](run.htm)
- [Dates, holidays, market](date.htm)
- [BarMode](barmode.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
---

# BarZone,AssetMarket,AssetFrame

# Time zone handling

Zorro's standard time zone is UTC (Greenwich mean time), which is also the time zone of historical data, logs, and charts. [Time and date functions](month.htm) and variables are normally also based on UTC time. Alternatively, local time zones can be used either on the bar or on the asset level. For handling or converting time zones, the following variables are available:   

## BarZone

Time zone of bars (default: **UTC**). Affects bar generation: daily bars will start and end at **BarZone** midnight plus [BarOffset](barperiod.htm). For this, set **BarZone** before the first [asset](asset.htm) call. **BarZone** is also used by several date/time functions that use local time. For using specific asset-dependent time zones with intraday bars, see **AssetFrameZone** and **AssetMarketZone**. 

## HistoryZone

Time zone of the historical data files, for converting them to **UTC**. This variable is rarely needed since timestamps in historical data files should be already in UTC. If not, set **HistoryZone** to the time zone in the data. The time stamps in historical data files and [dataset](data.htm) files are then automatically converted to UTC on **dataLoad** or **dataParse**. 

## BrokerZone

Time zone of the broker plugin. This variable is rarely needed since broker plugins normally return timestamps in UTC. Otherwise the broker plugin is supposed to set **BrokerZone** with the [GET_BROKERZONE](brokercommand.htm) command. The broker API time stamps are then converted to UTC on import. 

## LogZone

Time zone of the log and chart. If not set, all dates and times in the log and chart are in UTC.  

## AssetMarketZone

Time zone of the current [asset](asset.htm); initally read from the **Market** field of the [asset list](account.htm), otherwise copied from **BarZone** or set by script. 

## AssetFrameZone

Time zone of the current [asset](asset.htm) for daily trading; used to set **AssetFrame** to a daily **[TimeFrame](barperiod.htm)** that begins at **[FrameOffset](barperiod.htm)** in local time. 

### Type:

**int** , **UTC** for UTC time (default), **EST** for New York, **CST** for Chikago, **WET** for London, **CET** for Frankfurt, **AEST** for Sydney, **JST** for Tokyo, or any number from **-23..+23** that gives the time zone offset in hours to UTC. Daylight saving time is used, except for UTC and for time zones at or beyond JST.  

## AssetFrame

Asset specific time frame, automatically set by **AssetFrameZone**. **0** when the current [asset](asset.htm) had no price quotes in the current bar or when its market is closed; negative number of skipped bars when the market opens; **1** otherwise. Normally used to set [TimeFrame](barperiod.htm) **= AssetFrame** for skipping bars outside market hours, or for trading on different time zones (see example). 

### Type:

**int** , read/only  

### Remarks:

  * **BarZone, HistoryZone** , and **BrokerZone** affect the sampling of bars and thus must be set before loading history with the first [asset](asset.htm)() call. The asset-specific **AssetZone** and **AssetMarket** must be set after selecting the [asset](asset.htm) and can be changed at runtime.
  * If backtests use price history in local time and no time zone is set, all time/date functions and variables are then also in local time instead of UTC. 
  * Setting a non-UTC **BarZone** generates a daily bar of 23 or 25 hours when the daylight saving period begins or ends. The [run](run.htm) function can run twice or be skipped when the clock is set backwards or forwards. This does normally not matter, but should be taken into account in strategies that strongly rely on a 24-hour bar period or on bars ending or starting at a certain time.
  * For emulating day bars of different assets with different time zones, use 1-hour bars with **AssetFrameZone** and **AssetFrame** (see example). Use [FrameOffset](barperiod.htm) for starting the emulated bar at a certain local hour.

### Examples:

```c
// trade daily at 15:30 New York time
BarPeriod = 1440;
BarOffset = 15*60+30;
BarZone = EST;
...

// trade two assets on different time zones
BarPeriod = 60;
FrameOffset = 10; // trade both assets at 10:00 am local time
while(asset(loop("EUR/USD","USD/JPY")))
{
  if(strstr(Asset,"EUR"))
    AssetFrameZone = WET;
  else if(strstr(Asset,"JPY"))
    AssetFrameZone = JST;
  TimeFrame = AssetFrame; // use a daily time frame changing at 10:00 local time
  ...
}
```

### See also:

[TimeFrame](barperiod.htm), [StartMarket](date.htm), [BarOffset](barperiod.htm), [BarMode](barmode.htm), [TickFix](ticktime.htm), [Time/Date functions](month.htm),. [asset](asset.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
