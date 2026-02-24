---
title: distribute, assign, knapsack
url: https://zorro-project.com/manual/en/renorm.htm
category: Functions
subcategory: None
related_pages:
- [markowitz](markowitz.htm)
- [TrainMode](opt.htm)
- [optimize](optimize.htm)
- [filter, renorm](cfilter.htm)
- [Money Management](optimalf.htm)
---

# distribute, assign, knapsack

# Portfolio distribution functions

## distribute (var* Weights, var* Values, int N, int M, var* Caps) : var 

## distribute (var* Weights, var* Values, int N, int M, var Cap) : var 

Calculate portfolio weights for a subset of **M** assets out of a **N-** asset portfolio. The **M** largest positive elements of the **Values** array are taken; the other elements are ignored. The resulting weights are stored in the **Weights** array and normalized so that they sum up to **1**. A weight limit can be applied either individually (**Caps** array) per element, or globally (**Cap**) for all elements. Weights are clipped at the limit, and the remainders are distributed among the other positive weights. The total weight sum can end up less than **1** if the weight limit restriction were otherwise violated or if the **Values** are all zero.This function is normally used to distribute asset weights in a portfolio dependent on individual parameters such as momentum. 

### Parameters:

**Weights** | Output array of size **N** to receive **M** normalized weights. The other elements are **0**.  
---|---  
**Values** | Input array of size **N** , for instance projected profits, momentums, or Sharpe ratios of portfolio components.  
**N** | Number of elements in the **Weights** , **Values** , and **Caps** arrays.  
**M** | Max number of resulting nonzero weights, or **0** for distributing all **N** weights.   
**Caps** | Array of weight limits in the **0..1** range, or **0** for no weight limit, or **-1** for ignoring the component.  
**Cap** | Global limit in the **0..1** range to be applied to all weights, or **0** for no weight limits.  
  
### Returns

Sum of weights after applying the limits.  

## assign (int* Items, var* Costs, var* Weights, int N, var Budget) : var 

Given current **Costs** and portfolio **Weights** of **N** assets, distribute the given **Budget** according to asset weights. The **Weights** array can be generated with the **distribute** or [markowitz](markowitz.htm) functions. Due to the integer amounts, normally not the whole budget can be assigned. The function assigns in several steps as much of the budget as possible, and returns the rest. 

### Parameters:

**Items** | Output array of size **N** to receive the weight-equivalent amounts for all assets.  
---|---  
**Costs** | Input array of size **N** containing the current asset cost, such as price or margin cost.  
**Weights** | Input array of size **N** containing the normalized portfolio weights.The sum must be **1**.  
**N** | Number of elements of the arrays.   
**Budget** | Available capital to distribute, for instance the current account equity.  
  
### Returns

Budget remainder that could not be assigned.  
  

## knapsack (int* Items, var* Costs, var* Values, int N, var Budget, var Cap) : var 

Given current **Costs** and expected **Values** of **N** assets, calculate the optimal asset amounts that maximize the total value, while keeping the total cost below the given **Budget**. The function performs an unbounded knapsack optimization. It can be used to optimally distribute small capital among a portfolio of stocks or ETFs. 

### Parameters:

**Items** | Output array of size **N** to receive the optimal amounts for all assets.  
---|---  
**Costs** | Input array of size **N** containing the current asset cost, such as price or margin cost.  
**Values** | Input array of size **N** containing projected prices or expected profits.  
**N** | Number of elements of the arrays.   
**Budget** | Available capital to distribute, for instance the current account equity.  
**Cap** | Maximum **Budget** weight per asset in the **0..1** range (f.i. **0.5** = max 50% of **Budget**), or **0** for no asset limits.   
  
### Returns

Total expected value of the portfolio.  

### Remarks

  * For distributing small capital that is only sufficient for a few shares, calculate first the asset weights with the [markowitz](markowitz.htm) or **distribute** functions, then use the weights as **Values** for a **knapsack** optimization.
  * For a large budget that can buy many shares, weight-based capital distribution is normally preferable to knapsack optimization. The knapsack algorithm allocates most of the capital to a few assets with the highest value/price ratio. This results sometimes in better backtest performance, but produces a less diversified portfolio with accordingly higher risk.
  * For a fixed ratio of different asset classes \- for instance, 70% equities and 30% bonds - call **distribute** twice with stocks and bonds, then multiply the stock weights with **0.7** and the bond weights with **0.3**.
  * For optimizing a portfolio rebalancing system, make sure to [setf(TrainMode,TRADES)](opt.htm) because trade size matters. Make also sure to select an asset before calling [optimize](optimize.htm).

### Examples (see also the Alice6 system from the Black Book):

```c
// knapsack test
void main() 
{
  var wt[5] = {30,40,70,80,90};
  var val[5] = {40,50,100,110,130};
  int Items[5];
  var Total = knapsack(Items,wt,val,5,500,0); 
  printf("\nItems: %i %i %i %i %i => %.2f",
    Items[0],Items[1],Items[2],Items[3],Items[4],Total);
}
```

```c
// portfolio rebalancing
void run() 
{
  ...
  assetList("MyPortfolio"); // max 100 assets
  static var Weights[100],Strengths[100];
  if(month(0) != month(1)) // rebalance any month
  {
// calculate asset strengths
    while(asset(loop(Assets)))
      Strengths[Itor1] = RET(30*PERIOD);
// enter positions of the the 4 strongest assets
    distribute(Weights,Strengths,NumAssetsListed,4,0);
    int i;
    for(i=0; i<NumAssetsListed; i++) {
      asset(Assets[i]);
      int NewShares = Balance * Weights[i]/priceClose(0) - LotsPool;
      if(NewShares > 0)
        enterLong(NewShares);
      else if(NewShares < 0)
        exitLong("",0,-NewShares);
    }
  } 
}
```

### See also:

[renorm](cfilter.htm), [OptimalF](optimalf.htm), [markowitz](markowitz.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
