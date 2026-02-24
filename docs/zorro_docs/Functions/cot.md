---
title: COT
url: https://zorro-project.com/manual/en/cot.htm
category: Functions
subcategory: None
related_pages:
- [normalize, zscore](norm.htm)
- [Licenses](restrictions.htm)
- [Zorro.ini Setup](ini.htm)
- [Dataset handling](data.htm)
- [asset, assetList, ...](asset.htm)
- [Seasonal Strength](season.htm)
---

# COT

# Commitment Of Traders Report

The Commitments of Traders (COT) is a market report by the Commodity Futures Trading Commission (CFTC), listing the holdings of participants in futures of financial instruments, metals, and other commodities. It is believed by some traders to give insight into the upcoming development of those markets. The CFTC releases a new report every Friday at 3:30 p.m. Eastern Time, and the report reflects the commitments of traders on the prior Tuesday. 

*** The COT report by Quandl is currently unavailable due to the move behind a NASDAQ paywall. We will update the URLs and data format in a future version, and make it again available for free. ***

The following functions return selected parts of the COT report, and work likewise in backtest and live trading:

## COT (int Handle, string Code, int Field): var 

Downloads the COT report with the given Quandl **Code** and stores it in the dataset with the given **Handle**. Returns the position determined by the **Field** number for the time of the current bar. If the dataset already exists, the data is not loaded again, but only the position returned. 

## COT_CommercialPos (int Handle, string Code): int 

As before, but returns the net commercials position, i.e. the difference producer + swap dealer longs - producer + swap dealer shorts.

## COT_CommercialIndex (int Handle, string Code, int TimePeriod): var 

As before, but returns the commercials net position [normalized](norm.htm) over the given **TimePeriod** and scaled to 0..100.

##  COT_OpenInterest (int Handle, string Code): var 

As before, but returns the open interest.

### Returns:

Positions, net positions, or index. 

### Parameters:

**Handle** |  Dataset handle for storing the report. Every report needs a dedicated handle.  
---|---  
**Code** | The Quandl code number of the report; normally 6 digits or letters. Can be looked up under [ https://www.quandl.com/data/CFTC-Commodity-Futures-Trading-Commission-Reports](https://www.quandl.com/data/CFTC-Commodity-Futures-Trading-Commission-Reports). The **"CTFC/"** prefix and **"_F_ALL"** suffix of the Quandl database are automatically added.  
**Field** | The field of the report: **1** = open interest, **2** = producer long positions, **3** = producer short positions, **4** = swap dealer long positions, **5** = swap dealer short positions, **6** = noncommercials long positions, **7** = noncommercials short positions.   
**TimePeriod** | The number of bars for normalizing the index.  
  
### 

### Remarks:

  * For retrieving the COT report, [Zorro S](restrictions.htm) and a Quandl key is required (to be entered in [Zorro.ini](ini.htm)). 
  * The report is only downloaded when needed at the begin of the strategy. Subsequent function calls access the dataset only. Downloaded reports are stored in the **History** folder.
  * The source code of COT functions can be found in **contract.c** , which must be included. Note that the functions use the new COT report structure with the field names listed in the comment. If you're downlaoding a COT report with a different structure, copy the **COT** functions in your script and adapt the conversion string as described under [dataParse](data.htm). 

### Example:

```c
#include <contract.c>

// Database symbols and codes below are just for illustration.
// Current available symbols can be found on the Quandl / NASDAQ website.
function run()
{
  StartDate = 2018;
  EndDate = 2023;
  BarPeriod = 1440;
  LookBack = 12*20;
  set(PLOTNOW);
  
  assetAdd("GOLD","YAHOO:GC=F");
  asset("GOLD");
  string COT_Code = "088691";
  var Ind = COT_CommercialIndex(1,COT_Code,6*20); // gold COT report
  plot("Fast Index",Ind,NEW,RED);
  var Ind2 = COT_CommercialIndex(1,COT_Code,12*20);
  plot("Slow Index",Ind2,0,BLUE);
  var Ind3 = COT_OpenInterest(1,COT_Code);
  plot("Open Interest",Ind3,NEW,BLACK);
}
```

### See also:

[asset](asset.htm), [season](season.htm), [dataFromQuandl](data.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
