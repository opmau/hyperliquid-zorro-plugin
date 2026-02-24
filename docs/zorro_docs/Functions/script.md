---
title: Predefined strings: Script, Algo, Asset, History...
url: https://zorro-project.com/manual/en/script.htm
category: Functions
subcategory: None
related_pages:
- [Multiple cycles](numtotalcycles.htm)
- [algo](algo.htm)
- [trade management](trade.htm)
- [asset, assetList, ...](asset.htm)
- [tick, tock](tick.htm)
- [for(open_trades), ...](fortrades.htm)
- [Asset and Account Lists](account.htm)
- [loop](loop.htm)
- [Money Management](optimalf.htm)
- [Zorro.ini Setup](ini.htm)
- [assetHistory](loadhistory.htm)
- [optimize](optimize.htm)
- [set, is](mode.htm)
- [Asset Symbols](symbol.htm)
- [String handling](str_.htm)
- [brokerCommand](brokercommand.htm)
- [report](report.htm)
- [Included Scripts](scripts.htm)
---

# Predefined strings: Script, Algo, Asset, History...

# Predefined and user-supplied strings

## RootName

Name root for the exported parameters, rules, and factors (default = script name). Can be changed for sharing training data with other scripts. If [LogNumber](numtotalcycles.htm) is nonzero, the logs are also saved under the **RootName**. The name must not contain spaces or special characters. 

## Algo

The current algorithm identifier (read/only), set up by the [algo](algo.htm) function or a [TMF](trade.htm). Algo identifiers should be short and must not contain spaces or special characters. 

## Asset

The current asset name (read/only), set up initially by the [Asset] scrollbox. Is automatically changed at runtime with the [asset](asset.htm) function, a [tick](tick.htm) function, a [TMF](trade.htm) or a [trade loop](fortrades.htm). 

## AssetPrev

The name of the previously selected asset; in a [trade loop](fortrades.htm) the name of the asset that was selected before the loop. 

## AssetBox

The name of the currently visible asset in the [Asset] scrollbox (read/only). 

## Assets

NULL-terminated **string** array, automatically populated with the names of all available assets in the [asset list](account.htm) (read/only). Can be used either as a [**loop**](loop.htm) parameter, or used for enumerating assets, f.i. **for(N = 0; Assets[N]; N++)**. The asset names are valid after the first [asset](asset.htm) or [assetList](asset.htm) call. The number of assets in the list is returned by [assetList](asset.htm).

## Algos

User-supplied NULL-terminated **string** array, containing algo names to be used as a [**loop**](loop.htm) parameter. 

## Account

The name selected with the [Account] scrollbox, f.i **"Demo** " or **"FXCM"**. 

## Factors

Name suffix and extension of the file containing the [OptimalF](optimalf.htm) factors; can be set for selecting between different sets of factors. If not set up otherwise, the file begins with the script name and ends with **".fac"** , f.i. **"Z12.fac"**. Set **Factors** f.i. to **"oos.fac"** for reading the factors from the file **"Z12oos.fac"**.  

## History

User-supplied string with path, name suffix, and extension of the historical data files, for selecting between different sets and types (**.t1** , **.t2** , **.t6** , **.t8**) of price histories. If not set, **.t6** and the [HistoryFolder](ini.htm) path is assumed. A **'*'** character at the begin of the string represents the asset name with optional year number, **'?'** represents the asset name with no year number, and **'#'** represents the appended year number. **History** must be set before the first [asset()](asset.htm) or [assetHistory()](loadhistory.htm) call, but can be set differently for different assets. Examples (for AAPL 2015): **  
History = "*eod.t6";** reads price history from **AAPL_2015eod.t6** ; if not found, uses **AAPLeod.t6**.   
**History = "*adj#.t6";** reads price history from **AAPLadj_2015.t6** ; if not found, uses **AAPLadj.t6.  
History = "?.t6"; **reads price history from **AAPL.t6**.  
**History = "History\\\Temp[\\\\*.t1](file://*.t1)";** reads tick data history from **History\Temp\AAPL_2015.t1**.    
**History = "D:\\\Data\\\Options[\\\\*.t8](file://*.t8)";** reads underlying price history from option chains in **D:\Data\Options\AAPL.t8**. 

## Curves

User-supplied string with path, name, and extension of the file containing the exported equity or balance curves of all [optimize](optimize.htm) parameter variants in [Train] mode (see [export](export.htm#balance)). Also used for storing the daily equity curve in [Test] mode. The [LOGFILE](mode.htm) flag must be set for exporting curves. If the file already exists, the curves are appended to the end. If **Curves** is not set, no curves are exported in [Train] mode, and in [Test] mode the equity/balance curve is exported to a ***.dbl** file in the **Log** folder. 

## SymbolTrade

The trading [symbol](symbol.htm) of the current asset as set up in the asset list, or the plain asset name when the asset has no assigned trading symbol. Does not contain the optional broker or price source name. Can be modified with [strcpy](str_.htm) (see example). 

## SourceTrade

The trading [account](account.htm) name assigned to the current asset as set up in its [symbol](symbol.htm), or an empty strig for the currently selected account. This account is used for sending orders and [broker commands](brokercommand.htm). Can be modified with [strcpy](str_.htm). 

## SymbolLive

## SourceLive

## SymbolHist

## SourceHist

The symbols and account names assigned to the current asset for live and historical prices. Can be modified with [strcpy](str_.htm).

##  ZorroFolder

The folder in which Zorro was installed, with trailing backslash, f.i. **"C:\Program Files\Zorro\"** (read/only). This is not necessarily the root folder of the data files - see the remarks about [UAC](started.htm#uac). 

## LogFolder

The folder for the log files and reports; if not set, they are stored in the **Log** subfolder. 

## WebFolder

The folder for the HTML page that displays the trade status. If not set up otherwise, the HTML documents are generated in the **Log** subfolder. Can be set up in the [Zorro.ini](ini.htm) file. 

### Type:

**string**   

### Remarks:

  * Make sure not to assign temporary strings to the above string constants. Temporary strings will lose their content after the next command. For making them permanent, copy them to a static **char** array of sufficient size.
  * Make very sure not to assign new content to read/only strings. This will not only not work, it can even cause subsequent errors when system strings are overwritten.
  * More strings, performance data and trade lists, broker/account info, and file contents are available through the [report](report.htm) function. 

### Examples:

```c
Script = "MyScriptV2"; // store and load parameters under "MyScriptV2.par"
History = "*s1.t6";    // read historical data from f.i. "EURUSD_2013s1.t6"
History = "?.t6";    // enforce historical data with no year number, f.i. "EURUSD.t6"
WebFolder = "C:\\inetpub\\vhosts\\httpdocs\\trading";  // VPS web folder
assetAdd(Asset,strf("%s:%s",NewSource,SymbolLive)); // change source of current asset
```

```c
// change a future's symbol quarterly expiration date
void updateSymbol()
{
  string ExpDate = strstr(SymbolTrade,"FUT-");
  if(!ExpDate) return; // no future 
  int OldExpiry = atoi(ExpDate+4);
  int Month = month(wdate(NOW)); 
  Month = ((Month+2)/3)*3; // next quarter: 3, 6, 9, 12
  int NewExpiry = ymd(nthDay(year(NOW),Month,FRIDAY,3));
  if(OldExpiry == NewExpiry) return;
  memcpy(ExpDate+4,sftoa(NewExpiry,8),8);
  strcpy(SymbolLive,SymbolTrade);
  strcpy(SymbolHist,SymbolTrade);
  printf("#\n%s new contract",SymbolLive);
}
```

### See also:

[algo](algo.htm), [asset](asset.htm), [ report](report.htm), [LogNumber](numtotalcycles.htm), [included scripts](scripts.htm)
