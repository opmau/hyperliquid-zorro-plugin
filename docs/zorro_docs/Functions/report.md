---
title: report
url: https://zorro-project.com/manual/en/report.htm
category: Functions
subcategory: None
related_pages:
- [Performance Report](performance.htm)
- [Zorro.ini Setup](ini.htm)
- [String handling](str_.htm)
- [Command Line Options](command.htm)
- [Live Trading](trading.htm)
- [Asset and Account Lists](account.htm)
- [assetHistory](loadhistory.htm)
- [System strings](script.htm)
- [printf, print, msg](printf.htm)
---

# report

## report (int Number) : string

Returns a string with content dependent on the given **Number**. 

### Number:

**1** | Temporary string containing the current [performance report](performance.htm).  
---|---  
**2** | Content of the [Zorro.ini](ini.htm) file. Use [strvar](str_.htm) or [strtext](str_.htm) for parsing.  
**3** | Temporary string containing the [command line](command.htm).  
**4** | Content of the **.par** parameters file, if any. Empty in the initial run.  
**5** | Content of the **.c** rules file, if any. Empty in the initial run.  
**6** | Content of the **.fac** factors file, if any. Empty in the initial run.  
**10** | Temporary list of open trades similar to the [ status page](trading.htm). Empty when no trade was opened.  
**12** | User name from the [Login] field.  
**20** | Broker name selected with the scrollbox, f.i **"MT4"** or **"FXCM"**.  
**21** | Account name from the **Name** field in the [account list](account.htm).  
**23** | Account identifier from the **Account** field in the [account list](account.htm).  
**24** | Account currency from the **CCY** field in the [account list](account.htm).  
**25** | Current script name without folder and externsion.  
**26** | Current script folder.  
**32** | [QuandlKey](loadhistory.htm) from **zorro.ini**.  
**33** | [AVApiKey](loadhistory.htm) from **zorro.ini**.  
**34** | [IEXKey](loadhistory.htm) from **zorro.ini**.  
**35** | [EODKey](loadhistory.htm) from **zorro.ini**.  
  
### Remarks

  * Frequently used [system strings](script.htm) can be directly accessed through their name. 
  * Temporary strings are only valid until the next **report()** call.

### Example:

```c
// send a performance report every day by email
if(Live && dow(0) != dow(1)))
  email(TO,FROM,"Zorro Report",report(1),SERVER,USER,PASSWORD);
```

### See also:

[printf](printf.htm), [system strings](script.htm), [performance report](performance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
