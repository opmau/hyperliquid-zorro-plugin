---
title: Zorro.ini
url: https://zorro-project.com/manual/en/ini.htm
category: Main Topics
subcategory: None
related_pages:
- [Script Editor](npp.htm)
- [Historical Data](history.htm)
- [Licenses](restrictions.htm)
- [R Bridge](rbridge.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Python Bridge](python.htm)
- [assetHistory](loadhistory.htm)
- [Command Line Options](command.htm)
- [report](report.htm)
- [String handling](str_.htm)
- [Testing](testing.htm)
- [Live Trading](trading.htm)
---

# Zorro.ini

#  Zorro.ini / ZorroFix.ini

The **Zorro.ini** or **ZorroFix.ini** files can be edited for setting up some Zorro properties when the program is started. For this, open it in the **Zorro** folder with the [script editor](npp.htm) or any other plain text editor, and edit or enter the following parameters. Enter text strings between the double quotes.

Line | Default | Description  
---|---|---  
**Comma = 0** | Decimal point. | Set this to **1** for using a comma instead of a full stop for the decimal mark in exported spreadsheet files. For European Excel™ versions.   
**Mute = 0** | Sound enabled. | Set this to **1** for disabling the trading sounds. Useful when Zorro is trading from your bedroom.   
**Security = 0** | Remember login data. | Set this to **1** for forgetting the password, and to **2** for also forgetting the user name when Zorro is closed. Recommended when other people have access to your PC.   
**AutoConfirm = 0** | Ask for closing trades. | Set this to **1** for suppressing the messagebox "Close open trades?" at the end of a trading session, and don't close the open trades.   
**AutoCompile = 0** | Compile always. | Set this to **1** for compiling only modified scripts. Sets the **AUTOCOMPILE** flag. Saves a few seconds when testing scripts. Note that static or global variables are then not reset on subsequent runs.   
**CleanLogs = 30** | Delete logs after 30 days. | Set this to **0** for never deleting old logs and charts, otherwise to the minimum number of days for keeping logs, charts, and history cache.   
**CleanData = 0** | Never delete data files. | Set this to the minimum number of days (f.i. 365) for keeping previously trained rule, factor, and parameter files.   
**WebFolder = "Log"** | Log | Set this to the folder for storing the [trade status page](trading.htm#status). For viewing live results via Internet, many Windows servers use **C:\inetpub\wwwroot**.   
**StrategyFolder = "Strategy"** | Zorro\Strategy | Set this to a different folder containing the **.c** , **.cpp** , **.x** , or **.dll** scripts. Do not change the Strategy folder when using command line options or VC++ projects.  
**HistoryFolder = "History"** | Zorro\History | Set this to the folder containing [historical data](history.htm) files (***.t6** , ***.t1** , **`.t2** , ***.t8**). Useful when many Zorro installations share a common price history on a LAN server. Asset and account lists are still loaded from the local **History** folder.  
**HistoryScript = "Chart"** | Chart.c | Set this to the script to be started when clicking on a [historical data](history.htm) file (***.t6** , ***.t1** , **`.t2** , ***.t8**). "Chart" plots a chart, "History" opens the data viewer ([Zorro S](restrictions.htm) required).  
**RTermPath = ""** | Empty | Set this to the path to the **RTerm.exe** program for the [R bridge](rbridge.htm) and the [NEURAL](advisor.htm) machine learning method.   
**PythonPath = ""** | Empty | Set this to the **32-bit Python** folder for the [Python bridge](python.htm) of the 32-bit Zorro verison.   
**PythonPath64 = ""** | Empty | Set this to the **64-bit Python** folder for the [Python bridge](python.htm) of the 64-bit Zorro version.   
**VCPath = ""** | Empty | Set this to the **Visual Studio™ Build** folder that contains the **vcvars32.bat** and **vcvars64.bat** files (for instance, **"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build** "). For directly running C++ scripts; [Zorro S](restrictions.htm) required.   
**EditorPath = "Notepad++\Notepad++.exe"** | Notepad++ | Set this to the path to the text editor for scripts, logs, and performance reports.   
**ViewerPath = "ZView.exe"** | ZView | Set this to the path to the **.png** image viewer for charts and histograms.   
**QuandlKey = ""** | Empty | Set this to your Quandl API key. Keys for other data sources are also available, see [assetHistory](loadhistory.htm).   
**ZorroKey = ""** | Empty | Set this to your Zorro S subscription token.  
**Action = "Name: Commandline"** | Some examples | Assign a [Zorro command line](command.htm) to the [Action] scollbox under the given **Name**. Put your favorite scripts here. The scrollbox accepts up to 20 lines. Some command line options are available in [Zorro S](restrictions.htm) only.  
**Action = "Name: Program!Parameters"** | Some examples | Assign an external program to the [Action] scollbox under the given **Name**.  
  
### Remarks:

  * The **Zorro.ini** file is overwritten anytime when Zorro is installed or updated. For keeping your individual settings, have them in a file **ZorroFix.ini** , which overrules **Zorro.ini** when both files are present. 
  * Entries can be parsed by script with the [report()](report.htm) function and [strvar](str_.htm)**()** or [strtext](str_.htm)**()**. This way user-specific entries can be added to the **ini** file.
  * Paths can be either absolute or relative to the Zorro folder. Make sure that all path, folder, and file names are plain letters or numbers with no blanks, greeks, or other non-letter characters. Although Zorro itself has no problem with that, some plugins and modules have.
  * Changing folders is also possible with Windows folder redirection, using the **mklink** command or User Configuration > Policies > Windows Settings > Folder Redirection.

### See also:

[Testing](testing.htm), [Trading](trading.htm), [Command line](command.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
