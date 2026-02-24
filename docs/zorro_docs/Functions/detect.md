---
title: Frechet Distance
url: https://zorro-project.com/manual/en/detect.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [predict](predict.htm)
- [adviseLong, adviseShort](advisor.htm)
- [polyfit, polynom](polyfit.htm)
---

# Frechet Distance

## frechet (vars Data, int TimeFrame, var Scale, var* Pattern) : var 

Calculates the Frechet distance between the recent part of a data series and a predefined curve. Returns a percent value equivalent to the similarity between the two curves. This function can be used for comparing a price curve part with a template, thus detecting cups, zigzags, or similar patterns in the price curve. 

### Parameters:

**Data** | The [series](series.htm) to be compared.  
---|---  
**TimeFrame** | The number of bars in the series to be compared, or **0** for using the length of the pattern. Determines the horizontal size of the pattern.   
**Scale** | The vertical size of the pattern (f.i. **10*PIP** for detecting a 10 pips tall pattern)****. Use a negative scale for inverting the pattern.  
**Pattern** | The pattern shape to be detected in the series, given by an array of positive values that starts with the oldest value and ends with **0** as an end mark.  
  
### Returns

Similarity between **Data** and **Pattern** in percent, normally in the **20..80** range. 

### Remarks:

  * Related blog article: [ Pattern recognition with the frechet distance](https://robotwealth.com/pattern-recognition-with-the-frechet-distance/)
  * For determining a pattern array, paint the pattern on squared paper. The array values are equivalent to number of squares below any pattern segment. 
  * The algorithm is based on the **Fréchet distance** , a measure of similarity between two curves, often used for handwriting recognition. For the algorithm, imagine a dog walking along one curve and the dog's owner walking along the other curve. They are connected by a leash and walk from the start point to the end point of the curve. Both may vary their speed and even stop anytime, however neither can backtrack. The Fréchet distance is the length of the shortest possible leash required for traversing the curves in this manner.
  * The absolute values of the pattern array don't matter, as it is normalized to **Scale** before comparison. The pattern is also aligned with the minimum of the **Data** series. For automatically adapting the pattern size to the data amplitude, set **Scale = MaxVal(Data,TimeFrame) - MinVal(Data,TimeFrame);**. 
  * Because series arrays have reverse time order, the pattern array is also reversed before comparison. This must be considered when comparing the pattern with a data array that is not a series. 

### Example:

```c
//detect 10-pip 10-bar cup formations in the price curve
function run()
{
  vars Price = series(price());
  static var cup[10] = { 6,3,2,1,1,1,2,3,6,0 };
  plot("Cup Similarity",frechet(Price, 0, 10*PIP, cup),NEW,RED);
}
```

### See also:

[predict](predict.htm), [advise](advisor.htm), [polyfit](polyfit.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
