---
title: dayOpen, Close, High, Low, Pivot
url: https://zorro-project.com/manual/en/day.htm
category: Functions
subcategory: None
related_pages:
- [Dates, holidays, market](date.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [set, is](mode.htm)
- [price, ...](price.htm)
- [Indicators](ta.htm)
- [Time / Calendar](month.htm)
- [BarMode](barmode.htm)
---

# dayOpen, Close, High, Low, Pivot

## dayOpen (int zone, int day) : var 

## dayClose (int zone, int day) : var 

## dayHigh (int zone, int day) : var 

## dayLow (int zone, int day) : var 

## dayPivot (int zone, int day) : var 

Returns the open, close, high, low, and pivot point within the [market hours](date.htm) (by default, 9:30 am until 16:00 pm) of a given working day and time zone. Can be used for seasonal trading, gap trading, or daily price action. The pivot point is defined as **(High+Low+Close)/3**.   

### Parameters:

**zone** | **UTC** for UTC time or **ET** for New York time, or a number giving the zone offset in hours to UTC. Daylight saving time is considered in non-UTC zones from 2000 to 2019.  
---|---  
**day** | Working day offset, i.e. **0** = today (see remarks), **1** = yesterday, **2** = day before yesterday, and so on. Weekends are skipped, f.i. if today is Monday, **1** refers to last Friday.   
  
### Returns

Price. 

### Usage:

**dayClose(ET,1)** returns yesterday's closing price of the current asset at the New York Stock Exchange. 

### Remarks:

  * During the [LookBack](nhistory.htm) period all day price functions return **0**.
  * The [BarPeriod](barperiod.htm) matters for precision. F.i. for a time precision of 30 minutes, use a **BarPeriod** of 30 or below. The functions do not work for bar periods above one day.
  * Time travel attempts - for instance requesting today's high, low, or close price when the market is still open - will cause a "negative price offset" warning when the [PEEK](mode.htm) flag is not set. 
  * Market hours can be changed through the global variables **[StartMarket](date.htm)** (default: **930**) and **[EndMarket](date.htm)** (default: **1600**) in the **hhmm** format. If the hours don't matter, use alternatively the [price](price.htm) functions from daily bars, or the [HH](ta.htm) / [LL](ta.htm) functions. Daily bars change at midnight UTC. By setting **StartMarket** and **EndMarket** accordingly, arbitrary time periods can be used. F.i. for getting the high-low range of the first hour of a trading day, set **StartMarket** at **930** and **EndMarket** at **1030**. **EndMarket** must be higher than **StartMarket**.
  * Some traders believe that markets turn at fixed "support and resistance" pivot levels named **S1, S2, S3, R1, R2, R3** , etc. which are calculated from **dayPivot** , **dayClose** , **dayHigh** , and **dayLow**. There are many pivot level variants, for instance:  
  
Persons Pivot levels:  
**S1 = 2*Pivot - High  
S2 = Pivot \- (High - Low)  
S3 = 2*Pivot - (2*High - Low)  
R1 = 2*Pivot - Low  
R2 = Pivot + (High \- Low)  
R3 = 2*Pivot + (High - 2*Low)  
  
**Camarilla Pivot levels:  
**S1 = Close - (High - Low) x 1.0833  
S2 = Close - (High -Low) x 1.1666  
S3 = Close - (High - Low) x 1.2500  
S4 = Close - ((High \- Low) x 1.5000  
R1 = Close + (High - Low) x 1.0833  
R2 = Close + (High -Low) x 1.1666  
R3 = Close + (High - Low) x 1.2500  
R4 = Close + ((High \- Low) x 1.5000  
**
  * The source code of the day price functions can be found in **Source\indicators.c**. 

### Example:

```c
// simple gap trading system, 
// based on a publication by David Bean 
function run()
{
  BarPeriod = 10;
  LookBack = 100*24*60/BarPeriod; // 100 days lookback 

  asset("SPX500");  
  Stop = 100*PIP;
  TakeProfit = 40*PIP;

  vars Prices = series(price());  
  var High = dayHigh(ET,1);
  var Low = dayLow(ET,1); 
  var Close = dayClose(ET,1);

// enter a trade when the NYSE opens 
  if(High > 0 && Low > 0 && Close > 0 
    && timeOffset(ET,0,9,30) == 0)
  {
    var Avg = SMA(Prices,LookBack);
    if(*Prices > Close 
      && *Prices < High
      && *Prices < Avg)
      enterShort();
          
    if(*Prices < Close 
      && *Prices > Low
      && *Prices > Avg)
      enterLong();
  }
}
```

### See also:

[price](price.htm), [timeOffset](month.htm), [market](month.htm), [BarMode](barmode.htm), [Pivot](ta.htm#pivot), [Support/Resistance](ta.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
