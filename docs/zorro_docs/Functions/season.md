---
title: season
url: https://zorro-project.com/manual/en/season.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [price, ...](price.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [plotProfile, ...](profile.htm)
- [asset, assetList, ...](asset.htm)
- [predict](predict.htm)
---

# season

# Seasonal and directional strength

## predictMove (vars Data, int Length, int Horizon, var Percentile): var 

Predicts the magnitude of a price movement, based on statistical analysis of price changes in the **Data** series. Returns the given **Percentile** of **Data** changes within the given time **Horizon**. Example: if the function returns **10** at **Horizon = 20** and **Percentile = 95** , then in **95%** of all cases the **Data** value moved by **10** or less in any direction within **20** bars (and therefore moved by more than **10** only in **5%** of all cases). For statistical significance **Length** should be large compared to **Horizon**. The minimum length of the **Data** series is **Length+Horizon**. 

## predictSeason (vars Data, int Length, int Horizon, int Season): var 

Predicts the expected price movement within a given time **Horizon** , based on the current time and seasonal movement in the **Data** series. Example: if the function returns **1.5** at **Horizon = 20** and **Season = 4** , then the **Data** series is expected to rise by **1.5** within 20 bars from now, based on the annual movement at the same date in previous years. If **Horizon == 0** , the function returns no price move, but the average seasonal **Data** value at the current date. For statistical significance the **Data** series should cover at least 3 or 4 seasons. 

### Returns:

Predicted data change / average seasonal data. 

### Parameters:

**Data** |  A data [series](series.htm), usually from price functions **[price(), priceClose()](price.htm)** ,   
---|---  
**Length** | The length of the data series, normally the [LookBack](lookback.htm) period.   
**Horizon** | Price movement duration in bars.  
**Percentile** | Price movement percentile to return, f.i. **5** for the lowest 5% or **95** for the highest 5%.   
**Season** | **1** for daily, **2** for weekly, **3** for monthly, or **4** for annual seasons.   
  
### Remarks:

  * Source code in **indicators.c**. 
  * Use [plotDay](profile.htm), [ plotWeek](profile.htm), [plotMonth](profile.htm), or [plotYear](profile.htm) for displaying seasonal dependence in a histogram.

### Example:

```c
function run()
{
  StartDate = 2010;
  BarPeriod = 1440;
  LookBack = 300;	
 
  vars Prices = series(price());
  if(!is(LOOKBACK)) {
    LifeTime = 1;
    var WeeklyMove = predictSeason(Prices,LookBack,LifeTime,2);
    if(WeeklyMove > 0)
      enterLong();
    else if(WeeklyMove < 0)
      enterShort();
  }
}
```

### See also:

[asset](asset.htm), [ROC](ta.htm#roc), [predict](predict.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
