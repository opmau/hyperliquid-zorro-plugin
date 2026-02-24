---
title: ccyStrength
url: https://zorro-project.com/manual/en/ccy.htm
category: Functions
subcategory: None
related_pages:
- [asset, assetList, ...](asset.htm)
---

# ccyStrength

# Currency strength functions

## ccySet(var Strength)

Adds the given **Strength** value to the currency and subtracts it from the counter currency of the current Forex pair. The **Strength** value can be any indicator or other price function that indicates a "strength" or "weakness" of a currrency, for instance ROC or RSI. 

## ccyReset() 

Resets the stored strengths of all currencies. Usually called at the begin of every bar before the strengths are summed up by **ccySet**. 

## ccyStrength(string Currency): var 

Returns the average strength of the given **Currency** (3 letters, f.i. **"USD"**). The average strength is the strength sum of all Forex pairs beginning with **Currency** , minus the strength sum of all Forex pairs ending with **Currency** , divided by the number of Forex pairs. If a currency pair is given (7 letters, f.i. **"EUR/USD"**), the function returns the difference of both average strengths. 

## ccyMax(): string 

Returns the currency pair with the strongest currency and the weakest counter currency. 

## ccyMin(): string 

Returns the currency pair with the weakest currency and the strongest counter currency. 

### Parameters:

**Strength** | Strength value, for instance the price [rate of change](ta.htm#roc). Must be in the same range for all currency pairs.   
---|---  
**Currency** | Either a single currency (**"USD"**) or a currency pair (**"EUR/USD"**).  
  
### Remarks:

  * Currency strength functions can be used to detect currency price shocks that affect several Forex markets simultaneously (see example). 
  * The forex pair name matters: Use the standard names like **"EUR/USD"** , not variants like **"EURUSD"** , **"EUR-USD"** etc.
  * Parameters passed to **ccySet** should be normalized or a percentage or probablitly value so that they are directly comparable for all currencies.
  * The source code of the currency strength functions is contained in **Source\indicators.c**. 

### Example:

```c
// Currency Strength Strategy /////////////////////
// Exploits price shocks f.i. by CHF cap and Brexit

function run()
{
  BarPeriod = 60;
  ccyReset();	// reset strengths at begin of any bar
  string Name;
  while(Name = (loop(Assets)))
  {
    if(assetType(Name) != FOREX) 
      continue; // Currency pairs only
    asset(Name);
    vars Prices = series(priceClose());
    ccySet(ROC(Prices,1)); // store price change as strength
  }
  
// get currency pairs with highest and lowest strength difference
  string Best = ccyMax(), Worst = ccyMin();
  var Threshold = 1.0;

  static char OldBest[8], OldWorst[8];	// static for keeping contents between runs
  if(*OldBest && !strstr(Best,OldBest)) { // new strongest asset?
    asset(OldBest);
    exitLong();
    if(ccyStrength(Best) > Threshold) {
      asset(Best);
      enterLong();
    }
  } 
  if(*OldWorst && !strstr(Worst,OldWorst)) { // new weakest asset?
    asset(OldWorst);
    exitShort();
    if(ccyStrength(Worst) < -Threshold) {
      asset(Worst);
      enterShort();
    }
  }

// store previous strongest and weakest asset names  
  strcpy(OldBest,Best);
  strcpy(OldWorst,Worst);
}
```

### See also:

[asset](asset.htm), [ROC](ta.htm#roc)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
