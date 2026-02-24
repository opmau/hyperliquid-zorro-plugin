---
title: loop
url: https://zorro-project.com/manual/en/loop.htm
category: Functions
subcategory: None
related_pages:
- [System strings](script.htm)
- [Asset and Account Lists](account.htm)
- [algo](algo.htm)
- [W6 - Portfolios, Money Management](tutorial_kelly.htm)
- [run](run.htm)
- [for(open_trades), ...](fortrades.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [series](series.htm)
- [optimize](optimize.htm)
- [while, do](while.htm)
- [asset, assetList, ...](asset.htm)
---

# loop

## loop(string Name1, string Name2, ... ) : string

## loop(Assets) : string

## loop(Algos) : string

Enumerate assets or algos for training their parameters or rules separately. The **loop** function gets many strings, such as asset or algo names, as arguments. In [Test] and [Trade] mode it returns the first pointer on the first call, the next pointer on the next call and so on. It returns **0** after the last pointer of its arguments. In [Train] mode it returns the first pointer in the first component training cycle, the next one in the next cycle and so on. Component training cycles end after the last pointer. Alternatively the single [Assets](script.htm) or [Algos](script.htm) pointer can be given for enumerating asset or algo names. 

## of(string Name1, string Name2, ... ) : string

## of(int N1, int N2, ... ) : intptr_t

## of(Assets) : string

## of(Algos) : string

Enumerate pointers as above, but for general purposes and without the special behavior in [Train] mode. **of(0)** resets all open **of()** calls to their first element. 

### Returns

**Name1** on the first call, **Name2** on the next call, and so on. The last call returns **0**. 

### Parameters:

**Name1, Name2 ...** |  Up to 40 arguments, usually **strings** with asset or algo names.   
---|---  
**N1, N2 ...** |  Up to 40 integers.   
**Assets** |  [Predefined array](script.htm) with the asset names from the selected [asset list](account.htm).   
**Algos** |  [Predefined array](script.htm), to be set up by script to a list of all algo names.   
  
  
The following variables are valid after a **loop** call (not for **of**): 

## Loop1

## Loop2

Current return value of the first and second **loop** , normally the current asset/algo name. 

## Itor1

## Itor2

Current cycle number of the first and second **loop** , starting with **0**. 

## NumLoops1

## NumLoops2

Total number of cycles of the first and second **loop**.  

### Remarks:

  * In [Train] mode the **loop** function is handled in a special way. A complete training cycle is executed for every **loop** pointer. This ensures that parameters, rules, and factors are generated separately for every strategy component. The [algo](algo.htm) function can be used to identify the strategy component in the parameter, factor, or rule file (see the example in [Workshop 6](tutorial_kelly.htm)). 
  * Up to two **loop** calls in fixed order can be placed in the [run](run.htm) function, usually either an asset **loop** , or an algo **loop** , or an algo **loop** nested in the asset **loop**. For multiple loops through assets or algos, or for enumerating components outside the **run** function, use either **of()** or [for(...assets)](fortrades.htm) / [for(...algos)](fortrades.htm).
  * The **loop** and **of** functions enumerate all elements up to the end. Therefore do not abort a **loop** /**of** loop with **break** or **return** ; use **continue** for skipping elements. Call **of(0)** for resetting all **of** functions to their first element. 
  * Do not enumerate assets in a **loop** call when you only want to train common parameters, such as the holding time for a portfolio rotation. Use **of** insteead. 
  * The returned string or pointer remains valid until the next **loop** or **of** call. For storing asset or algo specific information in a loop, use a [AlgoVar](algovar.htm), or a [series](series.htm), or global arrays.

### Examples (see also Workshop 6):

```c
// of() nesting
string Str1,Str2,Str3;
while(Str1 = of("AA","BB","CC"))
while(Str2 = of("XX","YY","ZZ"))
while(Str3 = of("11","22","33"))
  printf("\n %s %s %s",Str1,Str2,Str3);

// filling a string array
string Strings[5];
for(i=0; Strings[i]=of("A","B","C","D","E"); i++);
```

```c
// portfolio strategy with 3 assets and 3 trade algos, 
// all components separately trained, using loop()
 
function tradeTrendLong()
{
  var MyParameter = optimize(...);
  ...
}
 
function tradeTrendShort()
{
  var MyParameter = optimize(...);
  ...
}

function tradeBollinger()
{
  var MyParameter = optimize(...);
  ...
}
 
function run()
{
  while(asset(loop("EUR/USD","USD/CHF","GBP/USD"))) // loop through 3 assets
  while(algo(loop("TRL","TRS","BOL"))) // and 3 different trade algorithms
  { 
    if(Algo == "TRL") tradeTrendLong();
    else if(Algo == "TRS") tradeTrendShort();
    else if(Algo == "BOL") tradeBollinger();   
  }
}
```

```c
// portfolio strategy with 3 assets and 3 trade algos, 
// with common trained parameters
 
var CommonParameter;

function tradeTrendLong()
{
  ...
}
 
function tradeTrendShort()
{
  ...
}

function tradeBollinger()
{
  ...
}
 
function run()
{
  asset("EUR/USD"); // select one of the assets for the common parameter
  CommonParameter = optimize(...);

  while(asset(of("EUR/USD","USD/CHF","GBP/USD"))) // loop through 3 assets witrh no specific optimization
  {
    algo("TRL"); tradeTrendLong();
    algo("TRS"); tradeTrendShort()
    algo("BOL"); tradeBollinger()
  }
}
```

```c
// portfolio strategy with 3 assets and 3 trade algos, 
// with a combination common and separately trained parameters, 
// using of()
 
var CommonParameter,MyParameter1,MyParameter2,MyParameter3;

function tradeTrendLong()
{
  // uses CommonParameter,MyParameter1;
  ...
}
 
function tradeTrendShort()
{
  // uses CommonParameter,MyParameter2;
  ...
}

function tradeBollinger()
{
  // uses CommonParameter,MyParameter3;
  ...
}
 
function run()
{
  ...
  asset("EUR/USD"); // select asset for optimize()

  CommonParameter = optimize(...);
  MyParameter1 = optimize(...);
  MyParameter2 = optimize(...);
  MyParameter3 = optimize(...);

  while(asset(of("EUR/USD","USD/CHF","GBP/USD"))) // loop through 3 assets
  while(algo(of("TRL","TRS","BOL"))) // and 3 different trade algorithms
  { 
    if(Algo == "TRL") tradeTrendLong();
    else if(Algo == "TRS") tradeTrendShort();
    else if(Algo == "BOL") tradeBollinger();   
  }
}
```

### See also:

[optimize](optimize.htm), [while](while.htm), [algo](algo.htm), [asset](asset.htm), [ Assets](script.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
