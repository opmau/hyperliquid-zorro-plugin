---
title: random
url: https://zorro-project.com/manual/en/random.htm
category: Functions
subcategory: None
related_pages:
- [randomize](randomize.htm)
- [Detrend, shuffling](detrend.htm)
- [abs](abs.htm)
- [sign](sign.htm)
---

# random

## random() : var 

Returns a random number uniformly distributed in the range from **-1.0** to **+1.0** (borders not included). Uses the [Lehmer algorithm](https://en.wikipedia.org/wiki/Lehmer_random_number_generator) with a period of 2,147,483,646. 

## random(var Max) : var 

Returns a random number uniformly distributed from **0** up to, but not including **Max**. 

## seed(int s)

Initiates a fixed random number sequence, dependent on the "seed" **s**. Affects **random** , [randomize](randomize.htm), and [Detrend](detrend.htm).

### Example:

```c
if(is(INITRUN)) 
  seed(365);
...
if(random() > 0) ... // true in 50% of all cases
```

### See also:

[ abs](abs.htm), [sign](sign.htm), [randomize](randomize.htm), [Detrend](detrend.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
