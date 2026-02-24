---
title: year, month, week, day, dow, hour, minute
url: https://zorro-project.com/manual/en/month.htm
category: Functions
subcategory: None
related_pages:
- [BarMode](barmode.htm)
- [Dates, holidays, market](date.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Time zones](assetzone.htm)
- [Bar, NumBars, ...](numbars.htm)
- [Formats Cheat Sheet](format.htm)
- [printf, print, msg](printf.htm)
- [Bars and Candles](bars.htm)
- [Status flags](is.htm)
- [tick, tock](tick.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [dayHigh, dayLow, ...](day.htm)
- [timer](timer.htm)
---

# year, month, week, day, dow, hour, minute

# Time and Calendar Functions

## year (int offset): int

Year number of the given bar, f.i. **2011**.   

## month (int offset): int

Month number of the given bar, **1..12**.   

## week (int offset): int

Week number of the given bar, **0..53** , with Monday as first day of week. 

## day (int offset): int

Day of month of the given bar, **1..31**. 

## dom (int offset): int

Number of days of the month of the given bar, **28..31**. 

## tdm (int offset): int

Trading day of the month of the given bar, **1..23**. Uses [BarZone](assetZone.htm) for determining Weekends and Holidays (see [BarMode](barmode.htm)) when counting trading days. 

## tom (int offset): int

Total number of trading days of the month of the given bar, **20..23**. **(tom(offset)-tdm(offset))** gives the days until the last trading day of the month. 

## dow (int offset): int

Day of week of the given bar, **MONDAY** (**1**), **TUESDAY** (**2**) ... **SUNDAY** (**7**).

##  ndow (int offset): int

Number of the current day of week in the current month; to be used f.i. for finding the first Monday (**1**), the third Friday (**3**), etc. 

## ldow (int zone,int offset): int

Local day of week of the given bar in the given time zone (see **timeOffset** below), considering daylight saving time (see **dst** below). 

## hour (int offset): int

Closing hour of the given bar, **0..23** , with 1 second precision. **hour(0)** gives the current hour and can be used to apply different trade tactics in the US, European, or Asia-Pacific session.  

## lhour (int zone, int offset): int

Closing local hour of the given bar in the given time zone (see **timeOffset** below), considering daylight saving time (see **dst** below). F.i. **lhour(EST)** returns the current hour at the New York Stock Exchange. 

## minute (int offset): int

Closing minute of the given bar, **0..59** , with 1 ms precision. **minute(0)** gives the current minute. 

## tod (int offset): int

Time of day of the given bar in the form **hhmm** , f.i. **2107** for 9:07 pm. A time in hhmm can be converted to minutes with **int MinuteOfDay = ((tod(0)/100)*60 + (tod(0)%100);**.

## ltod (int zone, int offset): int

Local time of day of the given bar in the form **hhmm**. 

## tow (int offset): int

Time of week of the given bar in the form **dhhmm**. **d** is the weekday number starting with Monday(1), f.i. **52107** = Friday 9:07 pm. See also [StartWeek](date.htm), [EndWeek](date.htm). 

## ltow (int zone, int offset): int

Local time of week of the given bar in the form **dhhmm**. 

## second (): var 

Closing second of the current bar. The decimals give the fraction of the second with 1 ms precision. The current second number of the day can be calculated with **var SecondOfDay = modf(wdate(0),0)*24*60*60;**. 

## second (var Time): var 

Second of the given **Time** value in Windows **DATE** format. The decimals give the fraction of the second with 1 ms precision. The returned value can be different to the seconds printed by **strdate** , which are rounded to the next full second.   

## minutesAgo (int bar): int 

Total number of minutes between the end of the bar with the given offset and the end of the current bar. 

## minutesWithin (int bar): var 

Total number of minutes from start to end of the bar with the given offset. Normally identical to [BarPeriod](barperiod.htm), but can be different due to weekends, holidays, daylight saving time, gaps due to lack of price quotes, or when called from an intrabar function.   

## workday (int offset): bool

Returns **true** when the given date or the end of the given bar is not a holiday or a weekend determined by [Holidays](date.htm), [StartWeek](date.htm), and [EndWeek](date.htm). 

## market (int zone, int offset): bool

Returns **true** when the end of the given bar is within [StartMarket](date.htm) and [EndMarket](date.htm) (see also [suspended()](suspended.htm)). 

## at (int hhmm): bool

Returns **true** when the given time in **hhmm** format is at the end or inside the current bar in the [ BarZone](assetzone.htm). 

## dst (int zone, int offset): int 

Returns **1** when daylight saving time is active at the end of the given bar, otherwise **0**. A simplified daylight saving schedule is used, based on US east coast daylight saving periods for all time zones in America, Sydney daylight saving periods for Australian and Pacific time zones, and European daylight saving periods for all other time zones. UTC and JST time zones have no daylight saving time. For years between 2000 and 2024 the real historical time switching dates are used, for all other years estimated dates from an average of the historical dates.   

### Parameters:

**bar** | Offset of the bar for which the date or time is returned; **0** for the current bar, **1** for the previous bar and so on. When multiple time frames are used, bar offsets are multiplied with [TimeFrame](barperiod.htm).  
---|---  
**offset** | Offset of the bar for which the date or time is returned; or **NOW** for the current PC time; or a day number in **DATE** format with no decimals.   
**zone** | **UTC** for UTC/GMT time, **WET** for London, **CET** for Frankfurt, **EST** for New York, **CST** for Chicago, **JST** for Tokyo, **AEST** for Sydney, or the zone offset in hours, from -12...+12 (with daylight saving) or from UTC-12..UTC+12 (without daylight saving). For the daylight saving period, US periods are used for time zones -4 or below, Australian periods are used for time zones 10, 11, 12, and European periods for all other time zones. Note that **JST** is defined as **UTC+9** since Japan uses no daylight saving.  
  
### Usage:

**hour(1)** returns the hour of the previous bar. 

## wdate (int offset): var 

Returns UTC date and time of the given bar **offset** in Windows **DATE** format. 

## ldate (int zone, int offset): var 

Returns date and time in the given time **zone** of the given bar **offset** in **DATE** format. 

## wdateBar (int n): var 

Returns UTC date and time of the bar with the given bar number (**0...[NumBars](numbars.htm)-1**) in **DATE** format. 

## wdatef (string Format, string DateTime): var

Returns date and time in **DATE** format of the given **DateTime** string. Can be used to parse date/time fields in CSV files in various [formats](format.htm). Seconds are read with decimals.  

## strdate (string Format, int offset): string

## strdate (string Format, int zone, int offset): string

## strdate (string Format, var Date): string

Returns a temporary string in the given **[Format](format.htm)** with the date and time of the given bar **offset** or **Date** ; can be used in [printf](printf.htm) statements. Predefined formats are **YMD** , **HM, HMS** , **YMDHMS**. If the **Format** string ends with **"%S."** , milliseconds are added; otherwise seconds are rounded to the next full second. Examples: **strdate(YMD,0)** returns a string with the date of the current bar, and **strdate("%y-%m-%d %H:%M:%S.",NOW)** returns the current date and time in millisecond precision.   

## nthDay(var Date, int Dow, int N): var

## nthDay(int Year, int Month, int Dow, int N): var

Returns the date of the **N** th given weekday at or after the given **Date** , or of the given **Year** and **Month**. For instance, **nthDay(2025,12,FRIDAY,3)** returns the date of the third Friday in December 2025. Useful for determining contract expiration dates.  

## dmy (int **YYYYMMDD**): var

## ymd (var Date): int

Convert a date in the **YYYYMMDD** format to the Windows **DATE** format and vice versa. For instance, **ymd(wdate(NOW) - 10)** returns the date from 10 days ago in the YYYYMMDD format.  

## utm (var Date): int

## mtu (int UnixTime): var

Convert a date in Windows **DATE** format to Unix time in seconds since 1-1-1970, resp. from Unix time to **DATE**. Unix time in milliseconds can be converted to seconds by dividing by 1000. 

### Parameters:

**Format** |  Format string, see [format codes](format.htm). Some often used format strings are predefined in **variables.h** : **YMD** = "**%Y%m%d** "; **HMS** = "**%H:%M:%S** "; **YMDHMS** = "**%y%m%d %H:%M:%S** ".  
---|---  
**offset** | [Bar](bars.htm) offset, **0** for the current bar, **1** for the previous bar and so on, or **NOW** for the current time (see remarks).   
**YYYYMMDD** | Date as integer, f.i. **20180403** for April 3, 2018.  
**Date** | Date/time in Windows **DATE** format, as returned from **wdate()**. Days are represented by whole number increments starting on Saturday, 30 December 1899, midnight UTC. The time of the day is represented in the fractional part of the number. This format allows easy and precise date comparison down to a microsecond (= **0.000001/(24*60*60)**).  
**DateTime** | Date/time string in various formats, see [format codes](format.htm).   
  
### Usage:

**printf("\nDate: %s",strdate(YMD,0));** prints the current bar date in YYYYMMDD format. **wdatef("%d.%m.%y","15.9.16")** returns the DATE value of September 15, 2016. 

## timeOffset (int zone, int days, int hour, int minute): int

Returns the offset (in bars) of the last bar closing at the given time, or a negative offset for a bar in the future. 

### Parameters:

**zone** | **UTC** for UTC time, **ET** for New York, ******WET** for London, **CET** for Frankfurt, **AEST** for Sydney, **JST** for Tokyo, or a number giving the zone offset in hours to UTC. Daylight saving time is considered in non-UTC zones from 2000 to 2024.  
---|---  
**days** | Number of days in the past, or **0** for the day of the current bar, or a negative number for days in the future.  
**hour** | Hour of the day, 0..23. If above 23, **days** is adjusted accordingly.   
**minute** | Minute of the hour, 0..59. If above 59, **hour** is adjusted accordingly.   
  
### Usage:

**timeOffset(ET,1,16,0)** returns the bar offset of yesterday's NYSE closing time. **timeOffset(ET,0,9,30)** returns **0** when the NYSE is just opening. **if(0 == timeOffset(0,0,TIME/100,TIME%100))** is fulfilled when the current time is at the **HHMM** value given by **TIME**. 

### Remarks:

  * If not stated otherwise, all dates and times reflect the time stamp of the Close price at the end of the bar. Dates and times are normally UTC, also called "GMT" or "Zulu" time (Greenwich mean time without daylight saving time). Timestamps in historical data are supposed to be either UTC, or to be a date only with no additional time part, as for daily bars. It is possible to use historical data with local timestamps in the backtest, but this must be considered in the script since the date and time functions will then return the local time instead of UTC, and time zone functions cannot be used.
  * All date and time functions return **0** or invalid values in the [INITRUN](is.htm) when no asset was yet loaded and thus the bars and date ranges are not yet set up. For getting the current time at script start, use **NOW** for the offset. When **offset** is **0** or omitted, the end time of the current bar is returned. When **offset** is set to **NOW** , the current UTC time of the PC clock is returned. When a value > 30000 is used for **offset** , it is assumed a day number in **DATE** format, and the corresponding year, month, or day is returned. The **offset** parameter is affected by [TimeFrame](barperiod.htm), f.i. when **TimeFrame** is **6** and **offset** is **3** , the time of **3*6 = 18** bars ago is returned. Note that not all time/date functions support the special **offset** values. 
  * Time functions with offset **0** return the current timestamp when called inside a [tick](tick.htm) function.
  * Be careful when checking time, because not all hours or minutes have an associated bar. For instance, **if(hour == 17)** will fail when no bar ends between 17:00:00 and 17:59:59 UTC. Better use the **at** function or check a time range.
  * Date and time functions require a [LookBack](lookback.htm) period of at least the highest used or returned bar offset.
  * Most date and time functions return **int** , but some return **var** \- take care of that when using the functions in a [printf](printf.htm) statement. 
  * In live trading the time functions are based on the broker's server time when supported by the API; otherwise on the PC time. The script must take care that rounding differences and small time deviations don't cause problems. For instance, the weekday (**dow(0)**) exactly at midnight can return different values depending on whether the server time is a second slower or faster than the PC time. So a tick can have a 23:59:59 time stamp, while the PC clock has already the next day at 00:00:00. When using daily bars, check the whole time and not only the day, or set the [bar offset](barperiod.htm) so that the bars do not change exactly at midnight.
  * Time printing uses the Windows COleDateTime class, which rounds a **DATE** value to the next second. Therefore, minutes and seconds in time strings can differ by about 0.5 seconds to the **minute** and **second** functions, which round the time to the next millisecond.

### Examples:

```c
// buy at UTC midday on the last day of the month, 
// sell 2 trading days later
if(NumOpenLong == 0) { // nothing yet bought this month
  if((tom(0) == tdm(0)) && hour() >= 12)
    enterLong();
} else if(tdm(0) >= 2)
    exitLong();

// print New York local time to the logprintf(strdate("# NY: %H:%M",EST,0));

// close all position at 16:15 local time
if(at(1615)) {
  printf("\nClosing all positions");
  exitLong("**");
  exitShort("**");
}

// convert a time stamp from EST to UTC
TimeStamp -= (EST+dst(EST,TimeStamp))/24.;

// calculate the next quarterly expiration date
int Month = month(wdate(NOW)); 
Month = ((Month+2)/3)*3; // next quarter: 3, 6, 9, 12
int ExpireDate = ymd(nthDay(year(NOW),Month,FRIDAY,3));
```

### See also:

[bar](bars.htm), [dayHigh](day.htm), [timer](timer.htm), [BarZone](assetzone.htm), [BarMode](barmode.htm), [format codes](format.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
