---
title: Historical Data Import and Conversion | Zorro Project
url: https://zorro-project.com/manual/en/import.htm
category: Main Topics
subcategory: None
related_pages:
- [Historical Data](history.htm)
- [Dataset handling](data.htm)
- [assetHistory](loadhistory.htm)
---

# Historical Data Import and Conversion | Zorro Project

#  Importing Historical Data - Sample Scripts

When purchasing historical data from a data vendor, you'll usually get it in CSV formats, JSON formats, or proprietary formats such as NX2. Since most data vendors change their data formats all the time, there are innumerable format variants around. Most formats can be directly converted to the [Zorro data format](history.htm) with the [ dataParse](data.htm) or [dataParseJSON](data.htm) functions and a supplied format string. Sometimes a two-step process is needed for modifying, splitting, or merging data fields that are very different to the target format. Some data has poor quality and requires detecting and removing bad records. If the data file contains multiple symbols, they must be parsed out in a loop and stored in multiple history files.

Below you'll find a collection of example scripts for non-trivial data conversion tasks. Example records of the source format are displayed in the initial comments. If you don't want to write a conversion script yourself, contact [ info@opgroup.de](mailto:info@opgroup.de) for hiring our data conversion service. 

### Price data (t1, t2, t6)

```c
// BitMex crypto to t1 
//id, exchange, market, tradeString, price, quantity, total, time_local, type 
//12311611 BMEX XBT/USD 14838153 957.57 940 900115.8 2016-12-31 00:01:29 BUY

string Format = "+0||||f|||%Y-%m-%d %H:%M:%S|s";
string InName = "CSV\\BMEX_XBTUSD_20170101_20171231.txt"; 
string OutName = "XBTUSD_2017.t1";

void main()
{
  dataNew(1,0,0);
  int Records = dataParse(1,Format,InName);
  printf("\n%d records read",Records);
  for(i=0; i<Records; i++) {
    T1* t1 = dataAppendRow(2,2);
    t1->time = dataVar(1,i,0);
    t1->fVal = dataVar(1,i,1);
    string buysell = dataStr(1,i,2);
    if(buysell[0] != 'B') t1->fVal = -t1->fVal; // negative bid quote
// display progress bar and check [Stop] button
    if(!progress(100*i/Records,0)) break;
  }
  if(Records) dataSave(2,OutName);
}
```

```c
// Bittrex multiple cryptos to M1 t6 
//id,exchange,symbol,unix date,price,amount,sell
//1667839,BT,SLSBTC,1475314100533,0.00087836,0.85360253,false

string Format = "+0,,,%t,f,f,s";

// merge ask + bid to a single T6 record with spread
void addToT6(T6* Tick,DATE Time,float Price,float Vol)
{
  static var Bid = 0, Ask = 0;
  if(Price < 0) { // bid? convert to ask & spread
    Bid = Price; 
    Price = ifelse(Ask > 0,Ask,-Bid);
  } else
    Ask = Price;
  if(Tick->time == 0.) { // new record
    Tick->time = Time;
    Tick->fOpen = Tick->fHigh = Tick->fLow = Price;
  }
  Tick->fClose = Price;
  Tick->fLow = min(Tick->fLow,Price);
  Tick->fHigh = max(Tick->fHigh,Price);
  if(Bid < 0) Tick->fVal = max(0.,Price + Bid);
  Tick->fVol += Vol;
}

void main()
{
  var N = assetList("AssetsBittrex");
  string Name;
  while(Name = loop(Assets)) 
  {
    string InName = strf("CSV\\%s.csv",Name);
    string OutName = strf("History\\%s.t6",Name);
    dataNew(1,0,0);
    int Records = dataParse(1,Format,InName);
    printf("\n%s: %d records read",Name,Records);
    dataNew(2,0,0);
    T6* Tick = dataAppendRow(2,7);
    int Minutes = 0;
    for(i=0; i<Records-1; i++) {
      var Time = dataVar(1,i,0),
      var Price = dataVar(1,i,1),
      var Vol = dataVar(1,i,2);
      string Sell = dataStr(1,i,3);
      if(Sell[0] == 't') Price = -Price; // bid quote
      if(Tick->time - 1./1440 > Time) { // 1 min distance
        Tick = dataAppendRow(2,7);
        Minutes++;
      }
      addToT6(Tick,Time,Price,Vol);
    }
    printf(", %d minutes",Minutes);
    if(Minutes) dataSave(2,OutName);
  }
}
```

```c
// Tickdata .csv to .t1, ask/bid 
//DateTime,Ask,Bid,Ask Volume,Bid Volume
//04.05.2003 21:00:00.914,1.12354,1.12284,0,0

string Format = "+%d.%m.%Y %H:%M:%S,f1,f2";
string InName = "CSV\\tick_mxnusd.csv"; 
string OutName = "History\\MXNUSD_%.t1";

void main()
{
  int i,Year;
  for(Year = 2010; Year <= 2022; Year++) {
    dataNew(1,0,0);
// read only records from current year
    int Records = dataParse(1,Format,InName,strf(".%i",Year));
    printf("\n%i - %d rows read",Year,Records);
    if(!Records) return;
    dataNew(2,0,0);
    for(i=0; i<Records; i++) {
      var Time = dataVar(1,i,0);
      var Price = dataVar(1,i,2); // bid price
      T2* Quote = dataAppendRow(2,2);
      Quote->time = Time;
      Quote->fVal = -Price;
      Price = dataVar(1,i,1); // ask price
      Quote = dataAppendRow(2,2);
      Quote->time = Time;
      Quote->fVal = Price;
    }
    dataSave(2,strf(OutName,Year)); // store year dataset
    printf("\n%s saved",strf(OutName,Year));
  }
}
```

```c
// t2 order book data to t1 tick data

string InName = "History\\BTCUSD_%i.t2";
string OutName = "History\\BTCUSD_%i.t1";
int StartYear = 2017, EndYear = 2022;

void main() 
{
  int Year;
  for(Year = EndYear; Year >= StartYear; Year--) {
    int Records = dataLoad(1,strf(InName,j),3);
    printf("\n%i records",Records);
    if(!Records) return;
    T2* Quotes = dataStr(1,0,0);
    var LowAsk = 100000;
    var HighBid = -100000;
    var CurrentTime = 0;
    int i,N=0;
    for(i=0; i<Records; i++,Quotes++)
    {
      if(CurrentTime > Quotes->time) { 
        T1* t1 = dataAppendRow(2,2);
        t1->time = CurrentTime;
        t1->fVal = HighBid;
        T1* t1 = dataAppendRow(2,2);
        t1->time = CurrentTime;
        t1->fVal = LowAsk;
        N+=2;
        LowAsk = 100000;
        HighBid = -100000;
      }
      CurrentTime = Quotes->time;
      if (Quotes->fVal > 0) // ask
        LowAsk= min(Quotes->fVal,LowAsk);
      else if (Quotes->fVal < 0) // bid
        HighBid = max(Quotes->fVal,HighBid);
    }
    dataSave(2,strf(OutName,j),0,N);
    printf("=> %i bars",N);
  }
}
```

### Options data (t8)

```c
// Append OptionsDX data to t8
//[QUOTE_UNIXTIME], [QUOTE_READTIME], [QUOTE_DATE], [QUOTE_TIME_HOURS], [UNDERLYING_LAST], [EXPIRE_DATE], [EXPIRE_UNIX], [DTE], [C_DELTA], [C_GAMMA], [C_VEGA], [C_THETA], [C_RHO], [C_IV], [C_VOLUME], [C_LAST], [C_SIZE], [C_BID], [C_ASK], [STRIKE], [P_BID], [P_ASK], [P_SIZE], [P_LAST], [P_DELTA], [P_GAMMA], [P_VEGA], [P_THETA], [P_RHO], [P_IV], [P_VOLUME], [STRIKE_DISTANCE], [STRIKE_DISTANCE_PCT]
//1638392400, 2021-12-01 16:00, 2021-12-01, 16.000000, 450.440000, 2021-12-01, 1638392400, 0.000000, 0.985740, 0.000470, 0.007560, -0.415280, 0.006070, 3.964130, 0.000000, 147.870000, 300 x 300, 139.610000, 142.100000, 310.000000, 0.000000, 0.010000, 0 x 4522, 0.030000, 0.000000, 0.000030, 0.000430, -0.004940, 0.000000, 2.521350, 0.000000, 140.400000, 0.312000

string Format = "+,%Y-%m-%d %H:%M,,,f5,i7,,,,,,,,,f4,,,f2,f1,f6,f8,f9,,,,,,,,,f10,,";
string InName = "CSV\\spy_eod_%d%02d.txt";
string OutName = "History\\SPY_2022.t8"; // file to append to

void parse(int Year,int Month)
{
  int Records = dataParse(2,Format,strf(InName,Year,Month));
  if(!Records) return;
  printf("\n%d new records",Records); 
  for(i=0; i<Records; i++) { 
// any record holds a call and a put contract, so split it in two
    CONTRACT* C = dataAppendRow(1,9);
    C->Type = CALL;
    C->time = dataVar(2,i,0);
    C->fAsk = dataVar(2,i,1);
    C->fBid = dataVar(2,i,2);
    C->fVol = dataVar(2,i,4);
    C->fUnl = dataVar(2,i,5);
    C->fStrike = dataVar(2,i,6);
    C->Expiry = dataInt(2,i,7);
    C = dataAppendRow(1,9);
    C->Type = PUT;
    C->time = dataVar(2,i,0);
    C->fAsk = dataVar(2,i,9);
    C->fBid = dataVar(2,i,8);
    C->fVol = dataVar(2,i,10);
    C->fUnl = dataVar(2,i,5);
    C->fStrike = dataVar(2,i,6);
    C->Expiry = dataInt(2,i,7);
  }
}

int Year = 2022,Month1 = 4,Month2 = 9;

void main() 
{
  dataNew(1,0,0);
  for(j=Month1; j<=Month2; j++)
    parse(Year,j);
  dataSort(1);
  int N = dataLoad(2,OutName,9);
  printf("\n%i previous records",N); 
  N = dataMerge(2,1);
  printf("\n%i total records",N); 
  if(N) {
    dataSave(2,OutName,0,0);
    printf("\nOk!");
  }
}
```

```c
// Append HistoricalOptionsData to t8
//underlying, underlying_last, exchange, optionroot, optionext, type, expiration, quotedate,strike,last,bid,ask,volume,openinterest,impliedvol,delta,gamma,theta,vega,optionalias,IVBid,IVAsk
//SPY,290.13,*,SPY190621C00295000,,call,06/21/2019,04/12/2019,295,3.59,3.59,3.62,8192,34153,0.102,0.4048,0.03,-16.1222,49.0036,,0.1017,0.1023

string Format = "+,f5,,,,s8,i7,%m/%d/%Y,f6,,f2,f1,f4,f3";
string InName = "CSV\\SPY_2021.csv";
string OutName = "History\\SPY_2021.t8";

void parse()
{
  int Records = dataParse(1,Format,InName);
  if(!Records) return;
  printf("\n%d new records",Records); 
  for(i=0; i<Records; i++) 
  {
    string PC = dataStr(1,i,8);
    dataSet(1,i,8,(int)ifelse(*PC=='p',PUT,CALL));
// fix expiry that is stored in reverse order
    int Expiry = dataInt(1,i,7);
    int MMDD = Expiry/10000;
    int YYYY = Expiry-MMDD*10000;
    dataSet(1,i,7,YYYY*10000+MMDD);
  }
}

void main() 
{
  dataNew(1,0,0);
  parse();
  dataSort(1);
  int N = dataLoad(2,OutName,9);
  printf("\n%d old SPY records",N); 
  N = dataMerge(2,1);
  printf("\n%d total SPY records",N); 
  if(N) {
    dataSave(2,OutName,0,0);
    printf("\nOk!");
  }
}
```

```c
// iVolatility H1 to t8
//ReceivingDate ReceivingTime,ID,Underl Symbol,Expiry,Strike,C/P,A/E,Symbol,Bid,Ask,Bid Time,Ask Time, Bid Size,Ask Size,Bid Exchange,Ask Exchange,Volume,?,Underlying,
//2014-06-24 9:30, 23083, AMZN7, 2014-06-27 0:00, 250, C, A, AMZN7, 140627C00250000, 74.85, 79.45, 2014-06-24 16:02, 2014-06-24 16:02, 0, 0, *, *, 0, 0.263868, 327.24

string Name = "AMZN";
string Format = "+%Y-%m-%d %H:%M:%S,,,i7,f6,s8,,,f2,f1,,,,,,,f4,,f5,";
string InName = "CSV\\options_%d-%02d_000.csv";
string OutName = "History\\%s_%d.t8";
int StartMonth = 1, EndMonth = 12, Year = 2014;

void main()
{
  int i,j,k;
  for(Year = 2014; Year <= 2019; Year++) {
    dataNew(2,0,0);
    for(i = EndMonth; i >= StartMonth; i--) 
    {
      string FileName = strf(InName,Year,i);
      if(!file_date(FileName))
        continue;
      dataNew(1,0,0);
      int Records = dataParse(1,Format,FileName); 
      printf("\n%d records",Records);
      var LastValidPrice = dataVar(1,0,5);
      int Dropped = 0;
      for(k = 0; k < Records; k++)
      { 
        CONTRACT* C = dataStr(1,k,0);
        if(C->fUnl < 0.7*LastValidPrice) {
          Dropped++; continue; // remove outlier
        }
        LastValidPrice = C->fUnl;
        C->Type = ifelse(dataStr(1,k,8) == "C",CALL,PUT);
        C->Expiry /= 10; // remove the '0' from the time
        dataAppend(2,1,k,1); // append dataset
      }
      printf(" - %d dropped",Dropped);
      if(!progress(100*(EndMonth-i)/(EndMonth-StartMonth+1),0)) return;
   }
   dataSort(2);
   dataSave(2,strf(OutName,Name,Year)); 
}
```

### Data manipulation

```c
// Remove outlier spikes from .t1 data
int isOutlier(var Price,var Prev,var Next)
{
  Price = abs(Price); // could be negative bid
  if(Price > abs(Prev)/Outlier && Price < abs(Prev)*Outlier)
    return 0;
  else if(Price > abs(Next)/Outlier && Price < abs(Next)*Outlier)
    return 0;
  else {
    printf("\nOutlier %i: %.2f-%.2f-%.2f",
      ++NumOutliers,Prev,Price,Next);
    return 1;
  }
}

void main()
{
  Outlier = 1.1; // detect outliers above 10%
  string Name = file_select("History","T1 History\0*.t1\0");
  if(!Name) return;
  int Records = dataLoad(1,Name,2);
  printf("\n %i records",Records);
  if(!Records) return;
  T1 *Tick = (T1*)dataStr(1,1,0);
  for(i=1; i<Records-1; i++) {
    if(isOutlier(Tick->fVal,(Tick-1)->fVal,(Tick+1)->fVal)) {
      Records = dataDelete(1,i,i);
      i--;
    } else
      Tick++;
  }
  if(NumOutliers) {
     file_copy(strx(Name,".","o."),Name); // keep a copy of the original
     dataSave(1,Name);
  }
}
```

```c
// Compress hi-res t6 files by removing records with similar timestamps
void main()
{
  string Name = "GER30";
  var Similarity = 500; // milliseconds, min distance 
  int Year = 2015; // first year
  for(; Year <= 2022; Year++) {
    int Records = dataCompress(1,strf("History\\%s_%4d.t6",Name,Year),7,Similarity);
    printf("\n Records = %i",Records);
    if(Records)
      dataSave(1,strf("History\\%s_%4d.t6",Name,Year));
  }
}
```

```c
// merge two historical data files
void main()
{
  int Rec1 = dataLoad(1,"History\\EURUSD_2016_1.t1"),2); // earlier
  int Rec2 = dataLoad(2,"History\\EURUSD_2016_2.t1"),2); // later
  printf("\n%i + %i",Rec1,Rec2);
  int Rec3 = dataMerge(1,2);
  printf(" = %i",Rec3);
  dataSave(1,"History\\EURUSD_2016.t1");
}
```

```c
// Bittrex multiple cryptos to M1 t6 
//id,exchange,symbol,unix date,price,amount,sell
//1667839,BT,SLSBTC,1475314100533,0.00087836,0.85360253,false

string Format = "+0,,,%t,f,f,s";

// merge ask + bid to a single T6 record with spread
void addToT6(T6* Tick,DATE Time,float Price,float Vol)
{
  static var Bid = 0, Ask = 0;
  if(Price < 0) { // bid? convert to ask & spread
    Bid = Price; 
    Price = ifelse(Ask > 0,Ask,-Bid);
  } else
    Ask = Price;
  if(Tick->time == 0.) { // new record
    Tick->time = Time;
    Tick->fOpen = Tick->fHigh = Tick->fLow = Price;
  }
  Tick->fClose = Price;
  Tick->fLow = min(Tick->fLow,Price);
  Tick->fHigh = max(Tick->fHigh,Price);
  if(Bid < 0) Tick->fVal = max(0.,Price + Bid);
  Tick->fVol += Vol;
}

void main()
{
  var N = assetList("AssetsBittrex");
  string Name;
  while(Name = loop(Assets)) 
  {
    string InName = strf("CSV\\%s.csv",Name);
    string OutName = strf("History\\%s.t6",Name);
    dataNew(1,0,0);
    int Records = dataParse(1,Format,InName);
    printf("\n%s: %d records read",Name,Records);
    dataNew(2,0,0);
    T6* Tick = dataAppendRow(2,7);
    int Minutes = 0;
    for(i=0; i<Records-1; i++) {
      var Time = dataVar(1,i,0),
      var Price = dataVar(1,i,1),
      var Vol = dataVar(1,i,2);
      string Sell = dataStr(1,i,3);
      if(Sell[0] == 't') Price = -Price; // bid quote
      if(Tick->time - 1./1440 > Time) { // 1 min distance
        Tick = dataAppendRow(2,7);
        Minutes++;
      }
      addToT6(Tick,Time,Price,Vol);
    }
    printf(", %d minutes",Minutes);
    if(Minutes) dataSave(2,OutName);
  }
}
```0

  

### See also:

[History](history.htm), [dataset](data.htm), [assetHistory](loadhistory.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
