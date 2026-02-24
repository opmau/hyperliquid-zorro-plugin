---
title: Algorithmic trading functions | Zorro Project
url: https://zorro-project.com/manual/en/funclist.htm
category: Functions
subcategory: None
related_pages:
- [Indicators](ta.htm)
- [Statistics, Transformations](transform.htm)
- [Spectral Analysis](filter.htm)
- [CBI](ddscale.htm)
- [Currency Strength](ccy.htm)
- [Candle Patterns](candle.htm)
- [rising, falling](rising.htm)
- [Commitment of Traders](cot.htm)
- [crossOver, crossUnder](crossover.htm)
- [fuzzy logic](fuzzy.htm)
- [dayHigh, dayLow, ...](day.htm)
- [sortData, sortIdx](sortdata.htm)
- [frechet](detect.htm)
- [slope, line](slope.htm)
- [peak, valley](peak.htm)
- [polyfit, polynom](polyfit.htm)
- [predict](predict.htm)
- [Seasonal Strength](season.htm)
- [contract, ...](contract.htm)
- [adviseLong, adviseShort](advisor.htm)
- [algo](algo.htm)
- [asset, assetList, ...](asset.htm)
- [assetHistory](loadhistory.htm)
- [distribute, knapsack](renorm.htm)
- [brokerCommand](brokercommand.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [combo, ...](combo.htm)
- [contractCPD](contractcpd.htm)
- [Dataset handling](data.htm)
- [enterLong, enterShort](buylong.htm)
- [for(open_trades), ...](fortrades.htm)
- [frame, frameSync](frame.htm)
- [loadStatus, saveStatus](loadstatus.htm)
- [login](login.htm)
- [loop](loop.htm)
- [price, ...](price.htm)
- [optimize](optimize.htm)
- [orderCVD](ordercvd.htm)
- [results](results.htm)
- [tradeUpdate](tradeupdate.htm)
- [abs](abs.htm)
- [asin, acos, atan, atan2](avar-asin.htm)
- [between](between.htm)
- [cdf, erf, dnorm, qnorm](cdf.htm)
- [floor, ceil](floor.htm)
- [normalize, zscore](norm.htm)
- [ifelse, valuewhen, ...](ifelse.htm)
- [clamp](clamp.htm)
- [sin, cos, tan](avar-sin.htm)
- [sinh, cosh, tanh](avar-sinh.htm)
- [diff](diff.htm)
- [exp](avar-exp.htm)
- [filter, renorm](cfilter.htm)
- [fmod](fmod.htm)
- [hmac](hmac.htm)
- [log](avar-log.htm)
- [min, max](min.htm)
- [modf](modf.htm)
- [pow](avar-pow.htm)
- [random, seed](random.htm)
- [series](series.htm)
- [round, roundto](round.htm)
- [sign](sign.htm)
- [sqrt](avar-sqrt.htm)
- [Time / Calendar](month.htm)
- [String handling](str_.htm)
- [rev](rev.htm)
- [vector, matrix](matrix.htm)
- [randomize](randomize.htm)
- [shift](shift.htm)
- [color](color.htm)
- [email](email.htm)
- [File access](file_.htm)
- [FTP transfer](ftp.htm)
- [putvar, getvar](putvar.htm)
- [HTTP functions](http.htm)
- [keys](keys.htm)
- [mouse](mouse.htm)
- [printf, print, msg](printf.htm)
- [panel, ...](panel.htm)
- [plot, plotBar, ...](plot.htm)
- [plotProfile, ...](profile.htm)
- [progress](progress.htm)
- [Python Bridge](python.htm)
- [R Bridge](rbridge.htm)
- [report](report.htm)
- [slider](slider.htm)
- [sound](sound.htm)
- [window](window.htm)
- [call](call.htm)
- [exec](exec.htm)
- [malloc, free, ...](sys_malloc.htm)
- [DLLs and APIs](litec_api.htm)
- [Error Messages](errors.htm)
- [Status flags](is.htm)
- [setf, resf, isf](setf.htm)
- [lock, unlock](lock.htm)
- [memory](memory.htm)
- [quit](quit.htm)
- [require, version](version.htm)
- [set, is](mode.htm)
- [timer](timer.htm)
- [wait](sleep.htm)
- [watch](watch.htm)
- [Integrating Zorro](engine.htm)
- [bar](bar.htm)
- [tick, tock](tick.htm)
- [evaluate](evaluate.htm)
- [trade management](trade.htm)
- [Functions](function.htm)
- [objective, parameters](objective.htm)
- [order](order.htm)
- [Content](content.htm)
---

# Algorithmic trading functions | Zorro Project

# Zorro algo trading functions by category

### Time series analyis

| _**Open Source**_  
---|---  
[AC](ta.htm) | Accelerator Oscillator | ✔  
[ADO](ta.htm) | Accumulation/Distribution Oscillator | ✔  
[AGC](transform.htm) | Automatic gain control | ✔  
[ADX](ta.htm) | Average Directional Movement Index | ✔  
[ADXR](ta.htm) | Average Directional Movement Rating | ✔  
[Alligator](ta.htm) | Alligator 3-line indicator | ✔  
[ALMA](ta.htm) | Arnaud Legoux Moving Average | ✔  
[Amplitude](filter.htm) | Amplitude of series | ✔  
[AO](ta.htm) | Awesome Oscillator | ✔  
[APO](ta.htm) | Absolute Price Oscillator | ✔  
[Aroon](ta.htm) | Aroon Indicator | ✔  
[AroonOsc](ta.htm) | Aroon Oscillator | ✔  
[ATR](ta.htm) | Average True Range, original  | ✔  
[ATRS](ta.htm) | Average True Range, simple MA | ✔  
[AvgPrice](ta.htm) | Average Price | ✔  
[BandPass](transform.htm) | Bandpass filter |   
[BBands](ta.htm) | Bollinger Bands | ✔  
[BBOsc](ta.htm) | Bollinger Bands oscillator | ✔  
[Beta](ta.htm) | Beta value | ✔  
[BOP](ta.htm) | Balance Of Power | ✔  
[Butterworth](filter.htm) | Butterworth filter | ✔  
[CBI](ddscale.htm) | Cold Blood Index |   
[CCI](ta.htm) | Commodity Channel Index | ✔  
[CCYI](ta.htm) | Correlation Cycle Indicator | ✔  
[CCYIR](ta.htm) | Correlation Cycle Indicator rate of change | ✔  
[CCYIState](ta.htm) | Correlation Cycle Market State | ✔  
[ccyMax](ccy.htm) | Strongest Forex pair | ✔  
[ccyMin](ccy.htm) | Weakest Forex pair | ✔  
[ccyReset](ccy.htm) | Initialize currency strength | ✔  
[ccySet](ccy.htm) | Define currency strength | ✔  
[ccyStrength](ccy.htm) | Get currency strength | ✔  
[CDL...](candle.htm) | 60 traditional candle patterns | ✔  
[CGOsc](ta.htm) | Center Of Gravity oscillator | ✔  
[ChandelierLong](ta.htm) | Chandelier exit long | ✔  
[ChandelierShort](ta.htm) | Chandelier exit short | ✔  
[Chikou](ta.htm) | Ichimoku Chikou line | ✔  
[CI](ta.htm) | Choppiness Index | ✔  
[CMO](ta.htm) | Chande Momentum Oscillator | ✔  
[concave](rising.htm) | Curve concavity |   
[ConnorsRSI](ta.htm) | Connors RSI indicator | ✔  
[Coral](ta.htm) | Coral indicator | ✔  
[Correlation](transform.htm) | Pearson correlation coefficient | ✔  
[COT](cot.htm) | Commitment Of Traders report | ✔  
[COT_CommercialPos](cot.htm) | COT commercials net position | ✔  
[COT_CommercialIndex](cot.htm) | COT index | ✔  
[COT_OpenInterest](cot.htm) | COT open interest | ✔  
[Covariance](transform.htm) | Covariance coefficient | ✔  
[crossOver](crossover.htm) | Curve cross over |   
[crossOverF](fuzzy.htm) | Fuzzy cross over |   
[crossUnder](crossover.htm) | Curve cross under |   
[crossUnderF](fuzzy.htm) | Fuzzy cross under |   
[CTI](ta.htm) | Correlation Trend Indicator | ✔  
[dayClose](day.htm) | Day close | ✔  
[dayHigh](day.htm) | Day high | ✔  
[dayLow](day.htm) | Day low | ✔  
[dayOpen](day.htm) | Day open | ✔  
[dayPivot](day.htm) | Day pivot | ✔  
[DChannel](ta.htm) | Donchian Channel | ✔  
[DCOsc](ta.htm) | Donchian Channel Oscillator | ✔  
[Decycle](ta.htm) | Ehlers' Decycler | ✔  
[DEMA](ta.htm) | Double Exponential Moving Average | ✔  
[Divergence](ta.htm) | Pricve / Oscillator peak line divergence | ✔  
[DominantPeriod](filter.htm) | Fundamental price oscillation |   
[DominantPhase](filter.htm) | Fundamental price phase |   
[DPO](ta.htm) | Detrended Price Oscillator | ✔  
[DX](ta.htm) | Directional Movement Index | ✔  
[EMA](ta.htm) | Exponential Moving Average | ✔  
[ER](ta.htm) | Efficiency Ratio | ✔  
[FIR3](filter.htm) | Finite Impulse Response filter, 3 taps |   
[FIR4](filter.htm) | Finite Impulse Response filter, 4 taps |   
[FIR6](filter.htm) | Finite Impulse Response filter, 6 taps |   
[falling](rising.htm) | Curve falling |   
[fallingF](fuzzy.htm) | Curve falling, fuzzy |   
[findIdx](sortdata.htm) | Find element |   
[Fisher](transform.htm) | Fisher transform | ✔  
[FisherInv](transform.htm) | Inverse Fisher transform | ✔  
[FisherN](transform.htm) | Fisher transform with normalization | ✔  
[FractalDimension](transform.htm) | Fractal Dimension | ✔  
[FractalHigh](ta.htm) | High Fractal indicator | ✔  
[FractalLow](ta.htm) | Low Fractal indicator | ✔  
[frechet](detect.htm) | Frechet pattern detection |   
[Gauss](transform.htm) | Gauss filter |   
[HAClose](ta.htm) | Haiken Ashi Close | ✔  
[HAHigh](ta.htm) | Haiken Ashi High | ✔  
[HALow](ta.htm) | Haiken Ashi Low | ✔  
[HAOpen](ta.htm) | Haiken Ashi Open | ✔  
[HH](ta.htm) | Highest High | ✔  
[HMA](ta.htm) | Hull Moving Average  | ✔  
[HighPass](filter.htm) | Wide highpass filter |   
[HighPass1](filter.htm) | 1-pole highpass filter |   
[HighPass2](filter.htm) | 2-pole highpass filter | ✔  
[HTDcPeriod](ta.htm) | Hilbert transform cycle period | ✔  
[HTDcPhase](ta.htm) | Hilbert transform cycle phase | ✔  
[HTPhasor](ta.htm) | Hilbert transform phasor components | ✔  
[HTSine](ta.htm) | Hilbert transform sine wave | ✔  
[HTTrendline](ta.htm) | Hilbert transform instantaneous trendline | ✔  
[HTTrendMode](ta.htm) | Hilbert transform trend indicator  | ✔  
[Hurst](transform.htm) | Hurst exponent | ✔  
[IBS](ta.htm) | Internal Bar Strength | ✔  
[Ichimoku](ta.htm) | Ichimoku indicator | ✔  
[KAMA](ta.htm) | Kaufman Adaptive Moving Average | ✔  
[KAMA2](ta.htm) | KAMA with individual settings | ✔  
[Keltner](ta.htm) | Keltner channel | ✔  
[Laguerre](filter.htm) | Laguerre lowpass filter | ✔  
[line](slope.htm) | Line position at a given bar | ✔  
[LinearReg](transform.htm) | Linear regression | ✔  
[LinearRegAngle](transform.htm) | Linear regression angle | ✔  
[LinearRegIntercept](transform.htm) | Linear regression intercept | ✔  
[LinearRegSlope](transform.htm) | Linear regression slope | ✔  
[LL](ta.htm) | Lowest Low | ✔  
[LowPass](filter.htm) | Lowpass filter |   
[LSMA](ta.htm) | Least Squares Moving Average | ✔  
[MACD](ta.htm) | Moving Average Convergence/Divergence | ✔  
[MACDExt](ta.htm) | MACD with various MA types | ✔  
[MACDFix](ta.htm) | MACD with standard parameters | ✔  
[MAMA](ta.htm) | MESA Adaptive Moving Average | ✔  
[MovingAverage](ta.htm) | Moving Average with given type | ✔  
[MovingAverage](ta.htm)  
[VariablePeriod](ta.htm) | Moving Average with variable period | ✔  
[MatchIndex](transform.htm) | Index of best match | ✔  
[MaxVal](transform.htm) | Highest value | ✔  
[MaxIndex](transform.htm) | Index of highest value  | ✔  
[Median](transform.htm) | Median filter |   
[MedPrice](ta.htm) | Center price of candle | ✔  
[MidPoint](ta.htm) | Center value of period | ✔  
[MidPrice](ta.htm) | Center price of period | ✔  
[MinVal](transform.htm) | Lowest value  | ✔  
[MinIndex](transform.htm) | Index of lowest value  | ✔  
[MinMax](transform.htm) | Lowest and highest values  | ✔  
[MinMaxIndex](transform.htm) | Indexes of lowest and highest values  | ✔  
[MMI](ta.htm) | Market Meanness Index | ✔  
[MinusDI](ta.htm) | Minus Directional Indicator | ✔  
[MinusDM](ta.htm) | Minus Directional Movement | ✔  
[Mode](transform.htm) | Most frequent value  | ✔  
[Mom](ta.htm) | Momentum | ✔  
[Moment](transform.htm) | Mean, variance, skew, kurtosis | ✔  
[MovingAverage](ta.htm) | Moving Average with various MA types | ✔  
[NATR](ta.htm) | Normalized Average True Range | ✔  
[Normalize](transform.htm) | Normalize to -1 .. +1 | ✔  
[NumInRange](transform.htm) | Count ranges in interval | ✔  
[NumDn](transform.htm) | Count of falling elements | ✔  
[NumRiseFall](transform.htm) | Length of streak | ✔  
[NumUp](transform.htm) | Count of rising elements | ✔  
[NumWhiteBlack](ta.htm) | Difference of white and black candles | ✔  
[OBV](ta.htm) | On Balance Volume | ✔  
[peak](peak.htm) | Curve peak |   
[peakF](fuzzy.htm) | Curve peak, fuzzy |   
[Percentile](transform.htm) | Percentile |   
[PercentRank](transform.htm) | Percent rank |   
[Pivot](ta.htm#pivot) | Pivot point | ✔  
[PlusDI](ta.htm) | Plus Directional Indicator | ✔  
[PlusDM](ta.htm) | Plus Directional Movement | ✔  
[polyfit](polyfit.htm) | Polynomial regression |   
[polynom](polyfit.htm) | Regression polynomial |   
[PPO](ta.htm) | Percentage Price Oscillator | ✔  
[predict](predict.htm) | Curve peak / crossover prediction |   
[predictMove](season.htm) | Predict price move by statistics | ✔  
[predictSeason](season.htm) | Predict price move by seasonal analysis | ✔  
[ProfitFactor](ta.htm) | Ratio of positive to negative returns | ✔  
[QLSMA](ta.htm) | Quadratic Least Squares Moving Average | ✔  
[R2](transform.htm) | Determination coefficient |   
[Resistance](ta.htm) | Resistance line | ✔  
[RET](ta.htm) | Rate of change between two points | ✔  
[rising](rising.htm) | Curve rising |   
[risingF](fuzzy.htm) | Curve rising, fuzzy |   
[ROC](ta.htm) | Rate of change | ✔  
[ROCP](ta.htm) | Rate of change percentage | ✔  
[ROCR](ta.htm) | Rate of change ratio | ✔  
[ROCL](ta.htm) | Logarithmic return | ✔  
[ROCR100](ta.htm) | Rate of change ratio, 100 scale | ✔  
[Roof](ta.htm) | Ehlers' roofing filter | ✔  
[RSI](ta.htm) | Relative Strength Index, original | ✔  
[RSIS](ta.htm) | Relative Strength Index, simple MA | ✔  
[RVI](ta.htm) | Ehlers' Relative Vigor Index | ✔  
[SAR](ta.htm) | Parabolic SAR | ✔  
[SemiMoment](transform.htm) | Mean, downside variance, skew, kurtosis | ✔  
[SentimentLW](ta.htm) | Williams' Market Sentiment | ✔  
[SentimentG](ta.htm) | Genesis Sentiment Index | ✔  
[ShannonEntropy](transform.htm) | Randomness metric | ✔  
[ShannonGain](transform.htm) | Expected gain rate | ✔  
[Sharpe](ta.htm) | Sharpe ratio | ✔  
[SIROC](ta.htm) | Smoothed Rate of Change | ✔  
[slope](slope.htm) | Line through minima or maxima | ✔  
[SMA](ta.htm) | Simple Moving Average | ✔  
[SMAP](ta.htm) | Average of positive values | ✔  
[SMom](ta.htm) | Smoothed Momentum  | ✔  
[Smooth](ta.htm) | Ehlers' super-smoother | ✔  
[Sortino](ta.htm) | Sortino ratio | ✔  
[Spearman](transform.htm) | Spearman's rank correlation coefficient | ✔  
[Spectrum](filter.htm) | Spectral analysis |   
[StdDev](transform.htm) | Standard deviation | ✔  
[Stoch](ta.htm) | Stochastic oscillator  | ✔  
[StochEhlers](ta.htm) | Ehlers' predictive stochastic  | ✔  
[StochF](ta.htm) | Stochastic Fast | ✔  
[StochRSI](ta.htm) | Stochastic RSI | ✔  
[Sum](transform.htm) | Sum of elements | ✔  
[SumDn](transform.htm) | Sum of falling elements | ✔  
[SumUp](transform.htm) | Sum of rising elements | ✔  
[Support](ta.htm) | Support line | ✔  
[T3](ta.htm) | Triple smoothed MA | ✔  
[TEMA](ta.htm) | Triple EMA | ✔  
[touch](crossover.htm) | Curve touches another |   
[Trima](ta.htm) | Triangular Moving Average | ✔  
[Trix](ta.htm) | TEMA rate of change | ✔  
[TrueRange](ta.htm) | True range | ✔  
[TSF](transform.htm) | Time Series Forecast | ✔  
[TSI](ta.htm) | Trend Strength Index |   
[TypPrice](ta.htm) | Typical price | ✔  
[UltOsc](ta.htm) | Ultimate Oscillator | ✔  
[UO](ta.htm) | Universal Oscillator  | ✔  
[Variance](transform.htm) | Variance | ✔  
[valley](peak.htm) | Curve valley |   
[valleyF](fuzzy.htm) | Curve valley, fuzzy |   
[Volatility](ta.htm) | Annualized volatility  | ✔  
[VolatilityC](ta.htm) | Chaikin Volatility indicator  | ✔  
[VolatilityMM](ta.htm) | Min/Max volatility  | ✔  
[VolatilityOV](ta.htm) | Empirical volatility  | ✔  
[WCLPrice](ta.htm) | Weighted Close Price | ✔  
[WillR](ta.htm) | Williams' Percent Range | ✔  
[WMA](ta.htm) | Weighted Moving Average | ✔  
[yield](contract.htm) | Riskfree interest rate | ✔  
[ZigZag](ta.htm) | ZigZag indicator | ✔  
[ZMA](ta.htm) | Zero-lag Moving Average | ✔  
|  |   
  
### Markets & trading

|   
[adviseLong](advisor.htm)/[Short](advisor.htm) | Machine learning indicator |   
[algo](algo.htm) | Select algorithm |   
[asset](asset.htm) | Select asset |   
[assetAdd](asset.htm) | Add asset to list |   
[assetHistory](loadhistory.htm) | Download price history |   
[assetList](asset.htm) | Select asset list |   
[assetSource](loadhistory.htm) | Set up online data source |   
[assetType](asset.htm) | Type of asset |   
[assign](renorm.htm) | Convert portfolio weights |   
[brokerAccount](brokercommand.htm) | Retrieve account status |   
[brokerAsset](brokercommand.htm) | Download asset parameters |   
[brokerCommand](brokercommand.htm) | Send special command to broker |   
[brokerRequest](brokercommand.htm) | Send a REST API request to broker |   
[brokerTrades](brokercommand.htm) | Download list of open positons |   
[cancelTrade](selllong.htm) | Undo trade |   
[combo](combo.htm) | Combine options to a combo | ✔  
[comboAdd](combo.htm) | Add option to a combo |   
[comboContract](combo.htm) | Return contract of a combo leg | ✔  
[comboLeg](combo.htm) | Select a combo leg |   
[comboLegs](combo.htm) | Return number of combo legs |   
[comboMargin](combo.htm) | Calculate the margin cost of a combo | ✔  
[comboStrike](combo.htm) | Return strike of given combo leg | ✔  
[comboPremium](combo.htm) | Calculate combo premium | ✔  
[comboProfit](combo.htm) | Calculate combo profit | ✔  
[comboRisk](combo.htm) | Calculate maximum possible loss | ✔  
[comboType](combo.htm) | Detect type of a combo | ✔  
[contract](contract.htm) | Select option/future contract |   
[contractCheck](contract.htm) | Check for termination |   
[contractCPD](contractcpd.htm) | Price probability analysis |   
[contractDays](contract.htm) | Contract duration |   
[contractDelta](contract.htm) | Delta value from strike | ✔  
[contractExercise](contract.htm) | Exercise option |   
[contractFind](contract.htm) | Find contract by parameter |   
[contractIntrinsic](contract.htm) | Intrinsic value | ✔  
[contractMargin](contract.htm) | Calculate contract margin | ✔  
[contractNext](contract.htm) | Next contract in chain |   
[contractPosition](contract.htm) | Get current position size |   
[contractPrice](contract.htm) | Get current value |   
[contractProfit](contract.htm) | Get current profit/loss |   
[contractRecord](contract.htm) | Save contract chain to history | ✔  
[contractRoll](contract.htm) | Roll expired contract forward | ✔  
[contractSellUnderlying](contract.htm) | Sell assigned stock from exercised contracts | ✔  
[contractStrike](contract.htm) | Strike value from Delta | ✔  
[contractUnderlying](contract.htm) | Unadjusted underlying price | ✔  
[contractUpdate](contract.htm) | Load contract chain |   
[contractVal](contract.htm) | Calculate option value | ✔  
[contractVol](contract.htm) | Calculate implied volatiltiy | ✔  
[cpd](contractcpd.htm) | Predicted price probability |   
[cpdv](contractcpd.htm) | Price at given probability |   
[dataDownload](data.htm) | Market data from online source |   
[dataFromCSV](contract.htm) | Market data from CSV file |   
[dataFromQuandl](contract.htm) | Market reports from Quandl™ | ✔  
[distribute](renorm.htm) | Calculate portfolio weights |   
[enterLong](buylong.htm)/[Short](buylong.htm) | Open position |   
[enterTrade](buylong.htm) | Add position from template |   
[exitLong](selllong.htm)/[Short](selllong.htm) | Close one or several positions |   
[exitTrade](selllong.htm) | Close selected position |   
[for(trades...)](fortrades.htm) | Enumerate trades, assets, algos |   
[frame](frame.htm) | Timeframe state |   
[frameSync](frame.htm) | Timeframe synchronization |   
[knapsack](renorm.htm) | Optimize asset amounts |   
[loadStatus](loadstatus.htm) | Load system status |   
[login](login.htm) | Connect to broker |   
[loop](loop.htm) | Loop through assets/algos |   
[marketVal](price.htm) | Bar value (spread) |   
[marketVol](price.htm) | Bar value (volume) |   
[markowitz](markowitz) | Mean-variance optimization |   
[markowitzReturn](markowitz) | Max return for given variance |   
[markowitzVariance](markowitz) | Min variance for given return |   
[optimize](optimize.htm) | Optimal parameter value |   
[orderCVD](ordercvd.htm) | Order flow analysis |   
[orderUpdate](ordercvd.htm) | Read order book |   
[price](price.htm) | Mean price at bar |   
[priceClose](price.htm) | Close at bar |   
[priceHigh](price.htm) | High at bar |   
[priceLow](price.htm) | Low at bar |   
[priceOpen](price.htm) | Open at bar |   
[priceQuote](price.htm) | Enter current price |   
[priceRecord](price.htm) | Save price to history |   
[priceSet](price.htm) | Modify price at bar |   
[results](results.htm) | Statistics of trade results |   
[saveStatus](loadstatus.htm) | Save system status |   
[seriesO/H/L/C](price.htm) | Price series |   
[suspended](suspended.htm) | Trading permission |   
[tradeUpdate](tradeupdate.htm) | Resize or update trade parameters |   
[tradeUpdatePool](tradeupdate.htm) | Synchronize virtual and pool trades |   
|  |   
  
### Math & misc

|   
**[abs](abs.htm)** | Magnitude |   
[aboveF](fuzzy.htm) | Fuzzy >= |   
[andF](fuzzy.htm) | Fuzzy && (and) |   
**[asin](avar-asin.htm)** | Arc sine |   
**[acos](avar-asin.htm)** | Arc cosine |   
**[atan](avar-asin.htm)** | Arc tangent |   
**[atan2](avar-asin.htm)** | Arc tangent, high precision |   
[belowF](fuzzy.htm) | Fuzzy <= |   
[between](between.htm) | Range check |   
[betweenF](fuzzy.htm) | Fuzzy range check |   
[cdf](cdf.htm) | Gaussian cumulative distribution |   
[ceil](floor.htm) | Round up |   
[center](norm.htm) | Center about median |   
[changed](ifelse.htm) | Change of value | ✔  
[clamp](clamp.htm) | Limits |   
[compress](norm.htm) | Scale to +/-100 |   
**[cos](avar-sin.htm)** | Cosine |   
**[cosh](avar-sinh.htm)** | Hyperbolic cosine |   
[cum](ifelse.htm) | Accumulation function | ✔  
[diff](diff.htm) | Change since last bar |   
[dnorm](cdf.htm) | Gaussian probability |   
[equalF](fuzzy.htm) | Fuzzy ==  |   
**[erf](cdf.htm)** | Statistical error function |   
**[exp](avar-exp.htm)** | Exponential |   
[filter](cfilter.htm) | 1D covolution filter |   
[fix0](ifelse.htm) | Safe division | ✔  
[floor](floor.htm) | Round down |   
[fmod](fmod.htm) | Fractional modulo |   
[fuzzy](fuzzy.htm) | Defuzzyfication |   
[genNoise](filter.htm) | Random noise | ✔  
[genSine](filter.htm) | Sine wave chirp | ✔  
[genSquare](filter.htm) | Square wave chirp | ✔  
[hmac](hmac.htm) | Cryptographic hash |   
[ifelse](ifelse.htm) | Conditional assignment |   
**[invalid](avar-log.htm)** | Invalid value |   
**[log](avar-log.htm)** | Logarithm |   
[max](min.htm) | Maximum |   
[min](min.htm) | Minimum |   
[modf](modf.htm) | Fractional part |   
[normalize](norm.htm) | Normalize to +/-100 |   
[notF](fuzzy.htm) | Fuzzy ! (not) |   
[once](ifelse.htm) | First occurrence |   
[orF](fuzzy.htm) | Fuzzy || (or) |   
**[pow](avar-pow.htm)** | Nth power, Nth root |   
[qnorm](cdf.htm) | Gaussian distribution value |   
**[random](random.htm)** | Random number generator |   
**[ref](series.htm)** | Value at bar | ✔  
**[renorm](cfilter.htm)** | Multiply and normalize |   
[round](round.htm) | Round to next integer |   
[roundto](round.htm) | Round to next step |   
[scale](norm.htm) | Center and scale to +/-100 |   
**[seed](random.htm)** | Random number initialization |   
**[sign](sign.htm)** | Sign |   
**[sin](avar-sin.htm)** | Sine |   
**[sinh](avar-sinh.htm)** | Hyperbolic sine |   
**[sqrt](avar-sqrt.htm)** | Square root |   
**[tan](avar-sin.htm)** | Tangent |   
[tanh](avar-sinh.htm) | Hyperbolic tangent |   
[valuewhen](ifelse.htm) | Conditional array access | ✔  
[zscore](norm.htm) | Z-Score |   
|  |   
  
### Time / date

|   
[at](month.htm) | Event at time of day | ✔  
[barssince](ifelse.htm) | Bars since condition became true | ✔  
[day](month.htm) | Day of month |   
[dmy](contract.htm) | YYYYMMDD to OLE time/date | ✔  
[dom](month.htm) | Days in month |   
[dow](month.htm) | Day of week |   
[dst](month.htm) | Daylight saving time |   
[hour](month.htm) | UTC hour |   
[ldow](month.htm) | Day of week at time zone |   
[lhour](month.htm) | Hour at time zone |   
[ltod](month.htm) | HHMM at time zone |   
[market](month.htm) | Market open time |   
[minute](month.htm) | Minute |   
[minutesAgo](month.htm) | Bar distance in minutes |   
[minutesWithin](month.htm) | Bar length in minutes |   
[month](month.htm) | Month at given bar |   
[mtu](month.htm) | OLE time/date from Unix time |   
[ndow](month.htm) | N-th weekday of month |   
[nthDay](contract.htm) | Date of n-th weekday of month | ✔  
[second](month.htm) | Second with microseconds |   
[tdm](month.htm) | Trading day of month |   
[timeOffset](month.htm) | Bar at given time |   
[tom](month.htm) | Trading days in month |   
[tod](month.htm) | HHMM time of day |   
[tow](month.htm) | DHHMM time of week |   
[ltow](month.htm) | DHHMM at time zone |   
[utm](month.htm) | Unix time |   
[wdate](month.htm) | OLE time/date |   
[wdateBar](month.htm) | OLE time/date at bar |   
[wdatef](month.htm) | OLE time/date from string |   
[week](month.htm) | Week number |   
[workday](month.htm) | Workday or holiday |   
[year](month.htm) | Year at given bar |   
[ymd](contract.htm) | OLE time/date to YYYYMMDD | ✔  
|  |   
  
### Data structures

|   
[atof](str_.htm) | String to var |   
[atoi](str_.htm) | String to int |   
[conv](rev.htm) | Convert array or time series |   
[dataAppend](data.htm) | Extend dataset |   
[dataAppendRow](data.htm) | Extend dataset by row |   
[dataChart](data.htm) | Plot dataset content |   
[dataClip](data.htm) | Remove records |   
[dataCol](data.htm) | Extract data column |   
[dataCompress](data.htm) | Remove duplicates |   
[dataFind](data.htm) | Find date/time in dataset |   
[dataLoad](data.htm) | Load dataset |   
[dataInt](data.htm) | Get integer from field |   
[dataMerge](data.htm) | Merge two datasets |   
[dataNew](data.htm) | Create dataset |   
[dataParse](data.htm) | Create dataset from CSV |   
[dataParseJSON](data.htm) | Create OHLC dataset from JSON |   
[dataSave](data.htm) | Save dataset |   
[dataSaveCSV](data.htm) | Save dataset to CSV |   
[dataSet](data.htm) | Set dataset field |   
[dataSize](data.htm) | Get dataset format |   
[dataSort](data.htm) | Sort dataset by date |   
[dataStr](data.htm) | Get string from field |   
[dataVar](data.htm) | Get variable from field |   
[filter](cfilter.htm) | Convolution filter |   
[matrix](matrix.htm) | Matrix / vector creation |   
[me](matrix.htm) | Matrix element |   
[matAdd](matrix.htm) | Matrix addition |   
[matMul](matrix.htm) | Matrix multiplication |   
[matScale](matrix.htm) | Matrix scaling |   
[matSet](matrix.htm) | Matrix copy |   
[matSub](matrix.htm) | Matrix subtraction |   
[matTrans](matrix.htm) | Matrix transpose |   
[randomize](randomize.htm) | Shuffle array or time series |   
[renorm](renorm.htm) | Normalize array |   
**[ref](series.htm)** | Reference past value |   
[rev](rev.htm) | Reverse array or time series |   
**[series](series.htm)** | Create time series |   
[sftoa](str_.htm) | Convert number to string |   
[shift](shift.htm) | Shift array or time series |   
[sortData](sortdata.htm) | Sort array |   
[sortIdx](sortdata.htm) | Create sort index |   
[sortRank](sortdata.htm) | Create ranking list |   
[strcat](str_.htm) | Append string |   
[strcmp](str_.htm) | Compare strings |   
[strcpy](str_.htm) | Copy string |   
[strlen](str_.htm) | String length |   
[strstr](str_.htm) | Find substring |   
[strchr](str_.htm) | Find character |   
[strrchr](str_.htm) | Find character from end |   
[strtok](str_.htm) | Tokenize string |   
[strvar](str_.htm) | Variable from ini string |   
[strtext](str_.htm) | Text from ini string |   
[strdate](str_.htm) | Time/date to string |   
[strf](str_.htm) | Variables to string |   
[strx](str_.htm) | Replace substrings |   
[strxc](str_.htm) | Replace characters |   
[strmid](str_.htm) | Strip string |   
[strcount](str_.htm) | Count characters |   
[strw](str_.htm) | Wide string |   
[stridx](str_.htm) | String to index |   
[strxid](str_.htm) | Index to string |   
[strtr](str_.htm) | Trade ID string |   
[strcon](str_.htm) | Contract ID string |   
[sprintf](str_.htm) | Print into string |   
[sscanf](str_.htm) | Parse string |   
[ve](matrix.htm) | Vector element |   
|  |   
  
### Input / output

|   
[color](color.htm) | Define color range |   
[colorScale](color.htm) | Brighten / darken color |   
[dataParse](data.htm) | Create dataset from CSV |   
[dataParseJSON](data.htm) | Create OHLC dataset from JSON |   
[email](email.htm) | Send email |   
[file_append](file_.htm) | Append data to end of file |   
[file_appendfront](file_.htm) | Append data to begin of file |   
[file_content](file_.htm) | Read content of file |   
[file_copy](file_.htm) | Copy file |   
[file_date](file_.htm) | File date |   
[file_delete](file_.htm) | Delete file |   
[file_length](file_.htm) | File size |   
[file_next](file_.htm) | Read directory |   
[file_read](file_.htm) | Read file to string |   
[file_select](file_.htm) | Open file dialog box |   
[file_write](file_.htm) | Write string to file |   
[ftp_download](ftp.htm) | Download file from FTP server |   
[ftp_upload](ftp.htm) | Upload file to FTP server |   
[ftp_getdate](ftp.htm) | Get file date and size from FTP server |   
[ftp_stop](ftp.htm) | Stop the current FTP transfer |   
[ftp_size](ftp.htm) | Size of the received file |   
[ftp_sent](ftp.htm) | Size of the sent file |   
[ftp_timestamp](ftp.htm) | Get file timestamp |   
[ftp_status](ftp.htm) | FTP transfer status |   
[ftp_log](ftp.htm) | Enables FTP logging |   
[getvar](putvar.htm) | Get system-wide variable |   
[http_transfer](http.htm) | Load data from website |   
[http_request](http.htm) | Send HTTP request  |   
[http_post](http.htm) | Start HTTP POST transfer |   
[http_proxy](http.htm) | Define a proxy server |   
[http_status](http.htm) | HTTP transfer status |   
[http_result](http.htm) | Retrieve received file |   
[http_free](http.htm) | Stop current HTTP transfer |   
[keys](keys.htm) | Send keystrokes to window |   
[mouse](mouse.htm) | Mouse position |   
[msg](printf.htm) | Message box |   
[panel](panel.htm) | Create user panel from spreadsheet |   
[panelFix](panel.htm) | Determine panel scroll area |   
[panelGet](panel.htm) | Get data from user panel |   
[panelLoad](panel.htm) | Load panel state |   
[panelMerge](panel.htm) | Merge cells on panel |   
[panelSave](panel.htm) | Save panel state |   
[panelSet](panel.htm) | Update user panel or action scrollbox |   
[plot](plot.htm) | Plot curve |   
[plotBar](plot.htm) | Plot histogram bar |   
[plotBuyHold](profile.htm) | Plot a buy-and-hold benchmark | ✔  
[plotChart](plot.htm) | Update histogram |   
[plotContract](contract.htm) | Plot option payoff diagram | ✔  
[plotCorrelogram](profile.htm) | Plot autocorrelation histogram | ✔  
[plotData](plot.htm) | Get plot data for export |   
[plotDay](profile.htm) | Daily seasonal analysis | ✔  
[plotDayProfit](profile.htm) | Daily profit histogram | ✔  
[plotGraph](plot.htm) | Plot symbol |   
[plotHeatmap](profile.htm) | Plot heatmap matrix | ✔  
[plotHistogram](profile.htm) | Plot a general histogram | ✔  
[plotMAEGraph](profile.htm) | Max adverse excursions histogram | ✔  
[plotMAEPercentGraph](profile.htm) | Max adverse excursions in percent | ✔  
[plotMFEGraph](profile.htm) | Max favorable excursion histogram | ✔  
[plotMFEPercentGraph](profile.htm) | Max favorable excursions in percent | ✔  
[plotMonth](profile.htm) | Monthly seasonal analysis | ✔  
[plotMonthProfit](profile.htm) | Monthly profit histogram | ✔  
[plotPriceProfile](profile.htm) | Price difference histogram | ✔  
[plotQuarterProfit](profile.htm) | Quarterly profit histogram | ✔  
[plotTradeProfile](profile.htm) | Profit distribution histogram | ✔  
[plotWeek](profile.htm) | Weekly seasonal analysis | ✔  
[plotWeekProfit](profile.htm) | Weekly profit histogram | ✔  
[plotWFOCycle](profile.htm) | WFO cycle analysis | ✔  
[plotWFOProfit](profile.htm) | Per-cycle profit histogram | ✔  
[plotYear](profile.htm) | Annual seasonal analysis | ✔  
[printf](printf.htm) | Print message |   
[print](printf.htm) | Print to target |   
[progress](progress.htm) | Progress bar |   
[putvar](putvar.htm) | Set system-wide variable |   
[pyInt](python.htm) | Integer from Python variable |   
[pySet](python.htm) | Send variables to Python |   
[pyStart](python.htm) | Start Python session |   
[pyVar](python.htm) | Double float from Python variable |   
[pyVec](python.htm) | Array from Python variable |   
[pyX](python.htm) | Execute Python code |   
[Rd](rbridge.htm) | Double float from R expression |   
[Ri](rbridge.htm) | Integer from R expression |   
[Rrun](rbridge.htm) | R status |   
[Rset](rbridge.htm) | Send variables to R |   
[Rstart](rbridge.htm) | Start R session |   
[Rv](rbridge.htm) | Array from R expression |   
[Rx](rbridge.htm) | Execute R expression |   
[report](report.htm) | Generate performance report |   
[slider](slider.htm) | Slider input |   
[sound](sound.htm) | Play WAV file |   
[window](window.htm) | Find active window |   
|  |   
  
### System

|   
[call](call.htm) | Run function at event |   
[exec](exec.htm) | Run external program |   
[free](sys_malloc.htm) | Free memory area |   
[GetProcAddress](litec_api.htm) | Get DLL function |   
[ignore](errors.htm) | Suppress error message |   
[ifelse](ifelse.htm) | Conditional assignment |   
[is](is.htm) | System flag state |   
[isf](setf.htm) | Flag state of a variable |   
[LoadLibrary](litec_api.htm) | Open DLL |   
[lock](lock.htm) | Protect code against interruption |   
[malloc](sys_malloc.htm) | Allocate memory area |   
[memcmp](sys_malloc.htm) | Compare memory |   
[memcpy](sys_malloc.htm) | Copy memory area |   
[memory](memory.htm) | Get memory allocation |   
[memset](sys_malloc.htm) | Fill memory area |   
[of](loop.htm) | Enumerate elements in a loop |   
[once](ifelse.htm) | First condition change |   
[quit](quit.htm) | Terminate simulation |   
[realloc](sys_malloc.htm) | Change memory area |   
[require](version.htm) | Require software version |   
[resf](setf.htm) | Reset flag of a variable |   
[set](mode.htm) | Set or reset system flag |   
[setf](setf.htm) | Set flag of a variable |   
[timer](timer.htm) | Performance timer |   
[unlock](lock.htm) | Unlock code |   
[version](version.htm) | Software version |   
[wait](sleep.htm) | Pause |   
[watch](watch.htm) | Debugging info |   
[Win32 API](http://www.win32-api.narod.ru/) | Windows API functions |   
[zalloc](sys_malloc.htm) | Allocate temporary memory space |   
[zInit](engine.htm) | Initialize multiprocess communication |   
[zClose](engine.htm) | Stop process |   
[zData](engine.htm) | Exchange process information |   
[zOpen](engine.htm) | Start process |   
[zStatus](engine.htm) | Get process status |   
|  |   
  
### Optional user-supplied functions

|   
[bar](bar.htm) | Special bar definition (Renko, Kagi, etc) |   
[click](panel.htm) | Button click function |   
[callback](tick.htm) | Callback for broker API and messages |   
[cleanup](evaluate.htm) | Run once at the end |   
[error](errors.htm) | Run at any error |   
[evaluate](evaluate.htm) | Evaluate strategy results |   
[manage](trade.htm) | Trade micromanagement |   
**[main](function.htm)** | Run once at the begin. |   
[neural](advisor.htm) | External machine learning and prediction |   
[objective](objective.htm) | Parameter optimization target |   
[order](order.htm) | Special order transmission |   
[parameters](objective.htm) | Parameter optimization setup |   
[run](run) | Run at any bar |   
[tick](tick.htm) | Run at any incoming price |   
[tock](tick.htm) | Run at fixed time intervals |   
|  |   
  
### 

|   
  
### 

[Zorro Home](https://zorro-project.com), [Zorro Manual](content.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
