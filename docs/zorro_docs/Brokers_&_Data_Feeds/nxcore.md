---
title: NxCore Plugin
url: https://zorro-project.com/manual/en/nxcore.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [assetHistory](loadhistory.htm)
- [Functions](funclist.htm)
- [Fill modes](fill.htm)
- [Broker API](brokerplugin.htm)
---

# NxCore Plugin

# NxCore Plugin 

NxCore (pronounced n'core) is a library for retreiving high-resolution live and historical price data, suited for HFT trading and HFT simulation. The latter is supported by [Zorro S](restrictions.htm). NxCore Tape files can be read and evaluated through the [assetHistory](loadhistory.htm) and [callback](funclist.htm) functions. 

NxCore tapes are usually split in 24-hour files that contain price quotes with volume and exchange time stamps of selected assets in a proprietary compressed format. For evaluating a tape, copy the **NxCoreAPI.dll** that you received together with the data in the Zorro main folder and select **NxCore** in the account scrollbox. For a script example, see [HFT Simulation](fill.htm). 

### Supported broker API functions

The NxCore plugin can be accessed with the following functions:

**login(int Mode)** ;  
// opens the NxCore DLL when **Mode** is nonzero. 

**brokerCommand(SET_HISTORY, Name);**   
// opens the tape file **Name** and calls the **callback** function for any asset in the tape, passing a pointer to a **QUOTE** struct. 

**typedef struct QUOTE {  
char Name[24]; **  _// name of the asset, with preceding 'e'_**  
var Time;**_// the exchange time stamp in microsecond resolution_**  
var Price;**_// the ask price (positive) or bid price (negative)_**  
var Size;**_// the ask or bid size_**  
} QUOTE;  
**

### See also:

[HFT fill mode](fill.htm), [broker plugin](brokerplugin.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
