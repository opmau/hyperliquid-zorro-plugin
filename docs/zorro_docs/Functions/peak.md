---
title: peak, valley
url: https://zorro-project.com/manual/en/peak.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [fuzzy logic](fuzzy.htm)
- [predict](predict.htm)
- [crossOver, crossUnder](crossover.htm)
- [rising, falling](rising.htm)
---

# peak, valley

## peak(vars Data) : bool 

## valley(vars Data) : bool 

Determines if the **Data** series had a maximum (**peak**) or minimum (**valley**) at the previous bar. 

## peak(vars Data, int TimePeriod) : int 

## valley(vars Data, int TimePeriod) : int 

Returns the bar offset of the last maximum (**peak**) or minimum (**valley**) of the **Data** series, or **0** when the series had no peak or valley within the **TimePeriod**. 

### Parameters:

**Data** | Time [series](series.htm)  
---|---  
**TimePeriod** | Number of series elements to test  
  
### Returns:

**true** or bar offset at the peak or valley, **false** or **0** if no peak and valley was found. 

### Modifies

**rMomentum** \- **Data** movement in percent per time frame; indicates the 'sharpness' of the peak or valley. 

### Algorithm:

```c
bool peak(var* Data) { 
  rMomentum = min(Data[1]-Data[2],Data[1]-Data[0])/Data[0]*100.;
  return (Data[2] <= Data[1]) && (Data[1] > Data[0]); 
}
bool valley(var* Data) {
  rMomentum = min(Data[2]-Data[1],Data[0]-Data[1])/a[0]*100.;
  return (Data[2] >= Data[1]) && (Data[1] < Data[0]); 
}
```

### Remarks:

  * The **Data** series must have a minimum length of **3** resp. the given **TimePeriod**.. 
  * A 'flat line' of the **Data** series for several bars will not produce a peak or valley signal. 
  * For a fuzzy logic version that also detects 'almost peaks' and 'almost valleys', use [peakF](fuzzy.htm) / [valleyF](fuzzy.htm). 
  * For detecting only 'strong peaks' or 'strong valleys', compare **rMomentum** with a percent threshold. 
  * For checking if a peak occurred exactly **n** bars before, use **peak(Data+n)**. 
  * For predicting a future peak or valley, use the [predict](predict.htm) function. 

### Example:

```c
function run()
{
  vars Trends = series(LowPass(series(price()),1000),3);
  if(valley(Trends))
    enterLong();
  if(peak(Trends))
    enterShort();
}
```

### See also:

[crossOver](crossover.htm), [crossUnder](crossover.htm), [rising](rising.htm), [falling](rising.htm), [peakF](fuzzy.htm), [valleyF](fuzzy.htm), [predict](predict.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
