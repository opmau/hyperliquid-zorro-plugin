---
title: loadStatus
url: https://zorro-project.com/manual/en/loadstatus.htm
category: Functions
subcategory: None
related_pages:
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [Trade Statistics](winloss.htm)
- [Live Trading](trading.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [putvar, getvar](putvar.htm)
- [brokerCommand](brokercommand.htm)
- [assetHistory](loadhistory.htm)
---

# loadStatus

## saveStatus(string FileName)

## loadStatus(string FileName)

Saves and loads the current trading status to a **.trd** file in the **Data** folder. The file contains the status of open real and phantom trades, the status of the sliders, and component specific variables ([AlgoVar](algovar.htm)). This function can be used to resume trading at the last stored state in case of a server crash or reboot, or to let a fallback server take over the trading in case of a hardware failure.   

### Parameters:

**FileName** | Name and path of the file, or **0** for a default name like **"Data\\\scriptname.trd"**.   
---|---  
  

## SaveMode

This variable determines by several flags what's saved or loaded in the **.trd** file in [Trade] mode or when calling the [saveStatus](loadstatus.htm)/[loadStatus](loadstatus.htm) functions. 

### Flags:

**SV_SLIDERS** \- save/load slider positions (default).  
**SV_ALGOVARS** \- save/load all [AlgoVars](algovar.htm) (default).   
**SV_ALGOVARS2** \- save/load all [AlgoVars2](algovar.htm).   
**SV_TRADES** \- save/load all open trades (default).   
**SV_STATS** \- save/load [statistics data](winloss.htm), to be continued in the next session**.  
SV_BACKUP** \- after loading, save a backup of the **.trd** file (default).   
**SV_HTML** \- after loading trades in [Test] mode, generate a HTML file with the list of open trades. 

### Type:

**int**

### Remarks:

  * Depending on **SaveMode** , the system status is automatically loaded in [Trade] mode at session start from a **Data\\*.trd** file.The file is updated at any trade or at the end of a [trading](trading.htm) session. Set **SaveMode = 0** for preventing the automatic saving and loading. In [Test] mode the **saveStatus** /**loadStatus** functions can be used for saving or loading.
  * The flags can be combined by adding (see example), and can be separately set or reset with the **setf** and **resf** macros.
  * For properly resuming statistics from the previous session, the end of that session must be within the [ LookBack](lookback.htm) period of the current session.
  * All needed assets and algos must have been selected in the initial run for loading their **AlgoVars** ; slider ranges must have been set in the initial run for loading slider positions. Static variables that have an effect on trade handling must be restored with [putvar](putvar.htm) / [getvar](putvar.htm).
  * Trades can be alternatively loaded directly from the broker account with the [brokerTrades](brokercommand.htm) function. In that case make sure that the **SV_TRADES** flag is not set. The broker API must support the **GET_TRADES** command. 

### Example:

```c
function run()
{
  if(Bar >= 1) { // default asset is selected after first run
    AlgoVar[0] = 123;
    SaveMode = SV_ALGOVARS+SV_ALGOVARS2; 
    saveStatus("Data\\Test.trd");
    AlgoVar[0] = 456;
    printf("\n %.0f",AlgoVar[0]);
    loadStatus("Data\\Test.trd");
    printf("\n %.0f",AlgoVar[0]);
    quit("ok!");
  }
}
```

### See also:

[AlgoVar](algovar.htm), [assetHistory](loadhistory.htm), [Trading](trading.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
