---
title: predict
url: https://zorro-project.com/manual/en/predict.htm
category: Functions
subcategory: None
related_pages:
- [polyfit, polynom](polyfit.htm)
- [Spectral Analysis](filter.htm)
- [crossOver, crossUnder](crossover.htm)
- [peak, valley](peak.htm)
- [Included Scripts](scripts.htm)
- [frechet](detect.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Seasonal Strength](season.htm)
---

# predict

## predict (int Type, vars Data, int TimePeriod, var Threshold): int

Predicts an event, such as a crossover of two curves or a peak of a curve, several bars before it happens. A [polynomial regression](polyfit.htm) function is used for the prediction. This function can be used to generate early trade signals. 

![](../images/predict.png)

Most trade systems analyze the recent price curve with functions or indicators that introduce more or less lag. This means that trade signals are always late, which reduces the profit of a system. One way to minimize lag is using low-lag functions (for instance, higher-order [lowpass filters](filter.htm) instead of moving averages). Another way is predicting the signals before they actually occur. This is the purpose of the **predict** function that works by extrapolating signal curves into the future.

### Parameters:

**Type** | Event to be predicted:  
**CROSSOVER** \- crossing of **Data** over the zero line  
**PEAK** \- Data peak  
**VALLEY** \- Data valley  
**+PARABOLIC** \- use parabolic instead of linear regression.   
---|---  
**Data** | Data series to be predicted, with a minimum length of **TimePeriod**.  
**TimePeriod** | Number of **Data** points used for the prediction. Do not use more date than a typical 'swing' of the curve.   
**Threshold** | Prediction threshold, or **0** for no threshold.   
  
### Returns

Bar offset (negative) of the predicted event, in the **-TimePeriod..0** range. **0** predicts that the event will happen in the next bar. **-TimePeriod** is returned when no event is predicted. 

### Modifies

**rMomentum** \- Predicted **Data** movement per bar at the time of the event.   

### Remarks:

  * The **predict** function does not peek into the future, but uses past data for predicting events on the future curve. 
  * It does not detect real crossovers or peaks/valleys, but predicts them only on future bars. For adding real detection of an event at a previous bar, use [crossOver](crossover.htm) or [peak](peak.htm)/[valley](peak.htm).
  * For predicting a crossover of two data series, or a series crossing over a fixed value, call **predict** with a series of the differences, f.i. **vars Difference = series(Data1[0] - Data2[0]);**. For a crossunder, reverse the differences (see example).
  * Crossovers are detected with better precision and less false signals than peaks or valleys. 
  * **Threshold** can be used to filter out 'weak' signals, f.i. from a crossing of two almost parallel lines. It is the minimum momentum of the **Data** line divided by the correlation coefficient.
  * For parabolic regression, add **+PARABOLIC** to the **type**. It detects events earlier than linear regression, but tends to produce more false signals.

### Examples:

```c
function run()
{
  vars Prices = series(price());
  var LP50 = LowPass(Prices,50);
  var LP150 = LowPass(Prices,150);
  
  var CO = predict(CROSSOVER,series(LP50-LP150),10,0.5*PIP); // predict crossover
  var CU = predict(CROSSOVER,series(LP150-LP50),10,0.5*PIP); // predict crossunder
    
  plot("LP50",LP50,0,RED);
  plot("LP150",LP150,0,BLUE);
  plot("CrossOver",CO,NEW,BLUE);
  plot("CrossUnder",CU,0,RED);
}
```

```c
// Trading with crossover vs. trading with prediction
#define USE_PREDICT
function run() 
{
  BarPeriod = 1440;
  asset("SPX500");
  vars Osc = series(StochEhlers(series(price()),10,20,10));
#ifndef USE_PREDICT  // use normal crossover
  if(crossOver(Osc,0.8)) 
    enterShort();
  if(crossUnder(Osc,0.2))
    enterLong();
#else                // use predicted crossover
  if(predict(CROSSOVER,series(Osc[0]-0.8),10,0.01) > -5) 
    enterShort();
  if(predict(CROSSOVER,series(0.2-Osc[0]),10,0.01) > -5) 
    enterLong();
#endif
}
```

Examples of signal prediction can also be found in the [Predict](scripts.htm) and [Ehlers](scripts.htm) scripts. 

### See also:

[frechet](detect.htm), [advise](advisor.htm), [polyfit](polyfit.htm), [crossOver](crossover.htm), [peak](peak.htm)/[valley](peak.htm), [predictMove](season.htm), [predictSeason](season.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
