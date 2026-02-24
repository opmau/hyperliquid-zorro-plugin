---
title: Fuzzy Logic
url: https://zorro-project.com/manual/en/fuzzy.htm
category: Functions
subcategory: None
related_pages:
- [Comparisons](comparisions.htm)
- [between](between.htm)
- [peak, valley](peak.htm)
- [rising, falling](rising.htm)
- [crossOver, crossUnder](crossover.htm)
- [series](series.htm)
- [adviseLong, adviseShort](advisor.htm)
- [optimize](optimize.htm)
---

# Fuzzy Logic

# Fuzzy logic functions 

Fuzzy logic is a form of many-valued logic. In contrast with traditional binary logic that deals only with **true** or **false** , fuzzy logic functions have a truth value that ranges in degree between **1** and **0** , from 'completely true' over 'almost true' to 'completely false'. Trade signals derived with fuzzy logic are often less susceptible to random noise in the price data, especially with complex logic expressions. 

The following functions return a fuzzy truth value: 

## equalF (var v1, var v2): var 

## aboveF (var val, var border): var 

## belowF (var val, var border): var

Fuzzy comparison, equivalent to the binary **[==](comparisions.htm)** , **[>](comparisions.htm)** and [**<**](comparisions.htm)****operators.

##  betweenF (var val, var lower, var upper): var

Fuzzy [between](between.htm) function. 

## peakF (vars Data): var 

## valleyF (vars Data): var 

Fuzzy [peak](peak.htm) and [valley](peak.htm) functions. 

## risingF (vars Data): var 

## fallingF (vars Data): var 

Fuzzy [rising](rising.htm) and [falling](rising.htm) functions. 

##  crossOverF (vars Data1, vars Data2): var

## crossUnderF (vars Data1, vars Data2): var

## crossOverF (vars Data, var border): var

## crossUnderF (vars Data, var border): var

Fuzzy [crossOver](crossover.htm) and [crossUnder ](crossover.htm)functions. 

## andF (var a, var b): var

## orF (var a, var b): var

## notF (var a, var b): var

Fuzzy logic functions, equivalent to the [&&](comparisions.htm), [||](comparisions.htm), and [!](comparisions.htm) operators. 

### Parameters:

**a, b** | Fuzzy truth values, **0.0 .. 1.0**  
---|---  
**Data, Data1, Data2** | Data [series](series.htm).   
**val, v1, v2** | Values to compare.  
**lower, upper, border** | Comparison borders.  
  
### Returns

Fuzzy truth, from **0.0** (completely false) to **1.0** (completely true)   

## fuzzy (var a): bool

Defuzzyfication, converts a fuzzy truth value to binary **true** or **false**. Use this function to convert the result of a fuzzy logic expression back to binary logic. 

## eq (var v1, var v2): bool

Fuzzy comparison, returns **true** when the parameters differ less than **FuzzyRange** , otherwise **false**. 

### Parameters:

**a** | Fuzzy truth value, **0.0 .. 1.0**  
---|---  
  
### Returns

Boolean **true** or **false**   

The following system variables affect the fuzzy logic calculation: 

## FuzzyRange

Determines the 'fuzziness' range of the input data (default: **0** = no fuzziness). When comparing two variables, the truth value goes from **0** to **1** within that range. Set this to a small fraction of the price volatility, or f.i. to **0.1*PIP** for comparing moving averages, or to **0.1** for comparing percentage values. The smaller this value, the 'less fuzzy' is the logic. At the default value **0** the logic is binary. The **FuzzyRange** variable is also used for classifying [signal patterns](advisor.htm) for price action trading. 

## FuzzyLevel

Determines the level above which fuzzy true becomes binary **true** (default: **0.5**); used by the **fuzzy** function.  

### Remarks:

  * All fuzzy logic functions work just like their binary counterparts, with the difference that the output value depends on the 'closeness' of the result. For instance, **equalF** already returns a nonzero value when the two compared values are close, and **crossOverF** will already return nonzero when two lines are so close that they almost touch. The functions use the same number of bars as their binary counterparts. 
  * The **FuzzyRange** variable is critical for the result of fuzzy logic expressions, and can be subject to an [optimize](optimize.htm) process. 

### Example:

```c
// Fuzzy version of a Workshop 4 variant ///////////////////
function run()
{
  vars Price = series(price());
  vars Trend = series(LowPass(Price,1000));
  vars Signals = series(0);
 
  Stop = 4*ATR(100);
  FuzzyRange = 0.001*ATR(100);

  var Valley = valleyF(Trend),
    Peak = peakF(Trend);
  Signals[0] = orF(Valley,Peak); // store the signal
  if(fuzzy(Valley))
    exitShort();
  if(fuzzy(andF(Valley,notF(Sum(Signals+1,3)))))
    enterLong();
  if(fuzzy(Peak))
    exitLong();
  if(fuzzy(andF(Peak,notF(Sum(Signals+1,3)))))
    enterShort();  
}

// Binary version for comparison  
function run()
{
  vars Price = series(price());
  vars Trend = series(LowPass(Price,1000));
  vars Signals = series(0);
 
  Stop = 4*ATR(100);
  bool Valley = valley(Trend),
    Peak = peak(Trend);
  if(Valley or Peak)
    Signals[0] = 1; // store the signal
  if(Valley)
    exitShort();
  if(Valley and Sum(Signals+1,3) == 0)
    enterLong();
  if(Peak)
    exitLong();
  if(Peak and Sum(Signals+1,3) == 0)
    enterShort();  
}
```

### See also:

[crossOver](crossover.htm), [crossUnder](crossover.htm), [rising](rising.htm), [falling](rising.htm), [peak](peak.htm), [valley](peak.htm), [comparisons](comparisions.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
