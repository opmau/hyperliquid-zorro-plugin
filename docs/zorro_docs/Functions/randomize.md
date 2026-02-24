---
title: randomize | Zorro Project
url: https://zorro-project.com/manual/en/randomize.htm
category: Functions
subcategory: None
related_pages:
- [Detrend, shuffling](detrend.htm)
- [random, seed](random.htm)
---

# randomize | Zorro Project

## randomize(int Method, var *Out, var *In, int Length): var 

Randomizes a data array by different methods. 

### Returns:

Last value of the resulting data array; usually the accumulated return.   

### Parameters:

**Method** | **BOOTSTRAP** | Randomize the data differences by bootstrap with replacement.   
---|---|---  
| **SHUFFLE** | Randomize the data differences by montecarlo permutation.   
| **DETREND** | Detrend the data before randomizing, by subtracting the mean data difference.   
**Out** | Array to be filled with the randomized data, or **In** for modifying the original array.   
**In** | Array containing the original data.   
**Length** | Number of elements of the **In** and **Out** arrays.   
  

### Remarks:

  * This function can be used for generating randomized return distributions f.i. for White's Reality Check. The random number generator uses the [Lehmer algorithm](https://en.wikipedia.org/wiki/Lehmer_random_number_generator) with a period of 2,147,483,646.
  * Randomized data curves can contain negative data. If this is not desired, test the minimum and randomize again until it is above zero.

### Example:

```c
var OriginalReturn = EquityCurve[Length-1];
var RandomizedReturn = randomize(BOOTSTRAP,0,EquityCurve,Length);
```

### See also:

[Detrend](detrend.htm), [random](random.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
