---
title: Historical data
url: https://zorro-project.com/manual/en/history.htm
category: Functions
subcategory: None
related_pages:
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Dates, holidays, market](date.htm)
- [assetHistory](loadhistory.htm)
- [Dataset handling](data.htm)
- [Bars and Candles](bars.htm)
- [System strings](script.htm)
- [contractCPD](contractcpd.htm)
- [Broker API](brokerplugin.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [price, ...](price.htm)
- [asset, assetList, ...](asset.htm)
- [Licenses](restrictions.htm)
- [Data Import](import.htm)
- [Included Scripts](scripts.htm)
- [HTTP functions](http.htm)
- [Asset and Account Lists](account.htm)
- [File access](file_.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
---

# Historical data

# Historical data (and how to get it)

Historical price data is used in [Test] and [Train] mode, and can also be used in [Trade] mode for pre-loading the [LookBack](lookback.htm) period. The needed data period is determined with [StartDate](date.htm) and [EndDate](date.htm). If historical prices from the desired period are missing, they can be downloaded with the [assetHistory](loadhistory.htm) function or the **Download** script (see below). All downloaded prices are usually aligned to UTC time. The broker's time zone does not matter. Therefore it's normally no problem to use price data from different sources for the simulation and for trading. 

Zorro stores historical price data in files the **History** folder. The file name begins with the asset name, and the file extension indicates the content:

  * **.t1** for plain ask/bid quotes (ticks), 
  * **.t2** for order book (market depth) data with volume,
  * **.t6** for candles with optional spread and volume, 
  * **.t8** for futures and options with strike and expiration. 

Any historical data file is simply a list of structs. Any struct represents a candle, quote, contract, or order book entry. The struct size - the number of elements besides the time stamp - is indicated by the number after the "**t** " extension. Thus, a **.t6** file consists of **T6** structs (see below) with a time stamp and 6 data elements. For avoiding extremely large files that are awkward to handle, price history is normally split in multiple years. The file name has then the year number attached; for instance, **"AAPL_2010.t2"** contains **AAPL** order book data from **2010**. Those multi-year files are automatically detected and loaded. 

Some files, such as high resolution options data, can be too large even for year splitting. They can then be split in months or days, and loaded by script at the begin of any month or day in the backtest. If additional data besides prices, spread, volume and timestamp is required, it can be separately imported from arbitrary user-specific formats in a [dataset](data.htm). The **.t6** price data files included with Zorro are normally split in years and contain a list of 1-minute candles in **T6** format. The data resolution is format independent, but should be at least as high or higher than the [bar resolution](bars.htm) in backtests.

The same asset can have different price histories, for instance EOD data for strategies with daily bars, M1 data for day trading and T1 data for scalping. The historical data files can be distinguished with a postfix character to the file name, for instance **"AAPLeod.t6"** for EOD data with no year number, **"AAPL_2018m1.t6"** for M1 candle data split into years and **"AAPL_2018.t1"** for tick data. The script can then select the right price history through the [History](script.htm) variable. For instance, **"History = *eod.t6"** selects the EOD data, **"History = *m1.t6"** the M1 data and **"History = *.t1"** the tick data. The default is ***.t6** for bars of one minute or more, and ***.t1** for bars of less than one minute. 

Historical data files contain either **T1** , **T2** , **T6** , or **CONTRACT** structs in descending timestamp order (newest price first).The structs are defined in the **trading.h** header:

```c
typedef struct T1{  DATE  time; // UTC timestamp of the tick in DATE format  float fPrice; // trade/ask/bid price, positive for ask, negative for bid} T1;
 
typedef struct T2
{
  DATE time;  // UTC timestamp in DATE format
  float fPrice; // trade/ask/bid price, negative for bid
  float fVol; // trade volume or ask/bid size
} T2; 
 
typedef struct T6
{
  DATE  time; // UTC timestamp of the close, DATE format
  float fHigh,fLow;	
  float fOpen,fClose;	
  float fVal;  // additional data, usually ask-bid spread
  float fVol;  // additional data, usually volume
} T6;
 
typedef struct CONTRACT
{
  DATE  time;   // UTC timestamp in DATE format
  float fAsk,fBid; // premium without multiplier
  float fVal,fVol;  // additional data, like delta, open interest, etc.
  float fUnl;   // underlying price (unadjusted)
  float fStrike; // strike price
  long  Expiry; // YYYYMMDD format
  long  Type;   // PUT, CALL, FUTURE, EUROPEAN, BINARY
} CONTRACT;
```

All years and assets should have the same historical file format; mixing different formats would produce an error message. Data in **.t2** order book format or **.t8** options format can be used additionally to the historical price data, since they are loaded with different functions ([orderUpdate](ordercpd.htm) and [contractUpdate](contractcpd.htm)). Underlying prices (**fUnl** in the **CONTRACT** struct) and order book data is normally unadjusted, while historical prices in **.t1** or **.t6** files are usually split and dividend adjusted. Some strategies require both types of prices.

The structs are stored in descending time order, i.e. the most recent tick comes first. Timestamps are in OLE **DATE** format, which is a **double float** that counts days since midnight 30 December 1899. Hours, minutes, seconds, and milliseconds are simply fractions of days. If the tick covers a time period, its timestamp is the UTC time of its last price, i.e. the time of the Close. If you have price data based on local time, convert it to UTC (you can find some example code [here](brokerplugin.htm)). Daily (**D1**) price history with timestamps containing the date only - i.e. no time of day - is supposed to be from the market close time. A bar time offset, such as 15:40, is then automatically set up when the user does not define a different offset. If the original price data contains bid prices, convert them to ask prices by adding the spread. If the price data contains time stamps not from the close of a tick, but from the open or middle price, correct them with the [TickFix](ticktime.htm) variable if needed for the strategy. In most cases it has little effect on backtest results if ticks contain ask, bid, or trade prices, or if time stamps are from the tick open or the close. 

The **fOpen** , **fClose** , **fHigh** and **fLow** data streams can be separately accessed with [price](price.htm) functions. The **fVal** and **fVol** data can be used to store additional data, such as spread, quote frequency, trade volume, implied volatility, delta, swap, etc, and accessed with [ market](price.htm) functions. All data streams are automatically synchronized to the streams of the first [asset](asset.htm) call in the script; so it does not matter if some data is only available in daily or weekly 'ticks'. 

Outliers in historical data are automatically detected and removed. If this is not desired, for instance with high volatility assets such as cryptocurrencies or penny stocks, disable [outlier detection](outlier.htm) or raise the detection level.

### Import and conversion

Data can be downloaded from brokers, free online source, data vendors, or the Zorro [download page](https://zorro-project.com/download.php). You can get it in different resolutions, and - with stocks - either split and dividend adjusted, or unadjusted. You normally use adjusted data for backtests. Forex M1 data and stock D1 data is free, but data in higher resolution or for other assets is not. M1 history for US Stocks and D1 options history can be purchased from us in **.t6** and **.t8** format, or from data vendors in CSV or JSON formats. When downloading data from a broker, make sure to select the 'last trade' price type if available, and adjust stock prices to avoid artifacts by splits and dividends. Purchased data usually contains adjusted trade prices.

For using data files with a resolution of less than one minute (such as **.t1** or **.t2** files), [Zorro S](restrictions.htm) is required. For importing price or options data files from external sources, convert them to a list of T1, T2, T6, or CONTRACT structs, optionally divide them into years or months, and store them in binary files with the format and name described above. 

Data from online sources or vendors in CSV or JSON formats can be converted to **.t1** , **.t2** , **.t6** , or **.t8** with the [dataParse](data.htm) function. Examples can be found under [Import](import.htm). The **Strategy** folder also contains several [small scripts](scripts.htm) for download and conversion from popular formats. **Download.c** (see below) downloads historical data in **.t6** or **.t1** format from Yahoo, IB, FXCM, or Oanda. **CSVtoHistory** converts data from popular .csv formats, f.i. from HistData, Quandl, or Yahoo, to the Zorro **.t6** format. Example code for automatically downloading **.csv** price data from the Internet or from a provider API can be found on the [http](http.htm) page. A script for generating artificial **.t8** historical data for options from the yield rate, dividends, and underlying price history can be found [here](http://www.financial-hacker.com/algorithmic-options-trading/). Example scripts for converting history files from complex **.csv** or **.nxc** formats to the **.t1** format can be found [here](http://www.financial-hacker.com/hacking-hft-systems/).

For looking into **.t8** , **.t6** , **.t2** , or **.t1** files and displaying their content, use the **[History](scripts.htm)** or [Chart](scripts.htm) scripts.

### The Download script

Historical data for many common tasks is avaiable on the [Zorro Download Page](https://zorro-project.com/download.php). Further data is available from brokers, online price sources, and data vendors. For downloading data from your broker, a small script - **Download.c** \- is included in the Zorro distribution. It can be used not only for retrieving price data, but also for updating asset parameters. You can download either the history of a single asset, or do a bulk download of all assets of a given [asset list](account.htm). 

The script opens a tiny control panel with a few toggle buttons (blue) and entry fields (yellow): 

History |   
---|---  
AAPL | Either asset symbol, or asset list file name  
2020-2024 | Time period to download  
M1 | Data resolution: M1, T1, T2, or D1  
Ask/Bid | Data type: Ask/Bid, Bid, Trades, Unadjusted, Parameters  
Broker | Data source: Broker, Stooq, Yahoo, AlphaVantage  
Download! | Click here to start the download.  
  
**Asset** or **Asset List**

Enter here the symbol of the asset in the form needed by the broker or price source, f.i. **"AAPL.US"** for downloading AAPL data from Stooq. If the name contains the word **"Assets"** or ends with **".csv"** (f.i. **"AssetsFXCM.csv"**), all assets of the asset list with the given name are downloaded. 

**Data resolution**

  * **M1** loads data in 1-minute OHLC ticks from the selected broker, and stores it in year files. This is the resolution that you normally use for backtests.
  * **T1** and **T2** stores single price quotes to **.t1** or **.t2** year files in high resolution (Zorro S required). 
  * **D1** loads End-of-Day data from online sources to a single multi-year file.

**Data type**

  * **Ask/Bid** are the default data that you normally use for backtests. Candles usually contain ask prices.
  * **Bid** stores data from bid prices, for special purposes.
  * **Trades** stores trade prices and volume (if supported by the broker).
  * **Unadjusted** selects EOD prices from Internet data sources that are not split and dividend adjusted. They are used for special purposes, f.i. options trading. 
  * **Parameters** downloads no prices, but updates the asset parameters from the selected broker (if supported), and stores them in **Log\Assets.csv**. Asset parameters not provided by the broker are taken from the current asset list.

**Data source**

  * **Broker** downloads data from the broker account selected in the Zorro panel.
  * **Stooq** downloads historical EOD data from Stooq (see [assetHistory](loadhistory.htm)). 
  * **Yahoo** downloads historical EOD data from Yahoo Finance (see [assetHistory](loadhistory.htm)). 
  * **AlphaVantage** downloads historical EOD data from AlphaVantage; an AV key is required (see [ assetHistory](loadhistory.htm)). 

Not all combinations make sense. For instance, you can normally not download M1 data or asset parameters from online data sources. Check with your broker which assets are available. It is recommended to download forex data from the same broker that you use for trading, since it's broker dependent. If a price history file of the same asset already exists, the new prices are appended at the end. If the file was already updated on the same or previous day, no data is downloaded. If you tried to download nonexisting data from a data source, you'll normally get a file containing an error message as response. Zorro won't interpret that file, but you can find it in your **History** folder as the most recent file, open it with a text editor, and see what's wrong.

After all is downloaded, click [Stop] to end the session.

### See also:

[Bars](bars.htm), [file](file_.htm), [ dataset](data.htm), [data import](import.htm), [asset parameters](pip.htm), [assetHistory](loadhistory.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
