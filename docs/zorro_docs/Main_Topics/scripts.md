---
title: Scripts
url: https://zorro-project.com/manual/en/scripts.htm
category: Main Topics
subcategory: None
related_pages:
- [Broker Arbitrage](brokerarb.htm)
- [Asset and Account Lists](account.htm)
- [Integrating Zorro](engine.htm)
- [Licenses](restrictions.htm)
- [Currency Strength](ccy.htm)
- [Historical Data](history.htm)
- [adviseLong, adviseShort](advisor.htm)
- [predict](predict.htm)
- [Spectral Analysis](filter.htm)
- [panel, ...](panel.htm)
- [keys](keys.htm)
- [Indicators](ta.htm)
- [Links & Books](links.htm)
- [Testing](testing.htm)
- [R Bridge](rbridge.htm)
- [contract, ...](contract.htm)
- [Performance Report](performance.htm)
- [Python Bridge](python.htm)
- [contractCPD](contractcpd.htm)
- [Dataset handling](data.htm)
- [bar](bar.htm)
- [Broker API](brokerplugin.htm)
- [Strategy Coding, 1-8](tutorial_var.htm)
- [The Z Strategies](zsystems.htm)
---

# Scripts

# Script repository

The following scripts are included as examples and utilites and can be started from the [Script] scrollbox. Please note that not all scripts will start right away. Some are for Zorro S, some need training first, some need more historical data, and some need Python, R, or modules like DeepNet or RQuantlib. Please check out the description in the source code.

### Analysis

Calculates mean, standard deviation, skew, and kurtosis of the price curve of the selected asset. 

### Benchmark

Simple RSI strategy with 10-year backtest for speed tests and comparisons. 

### Binary

Example strategy for binary options with 5-minute bets. 

### BrokerArb

[Broker arbitrage](brokerarb.htm) example. Exploits forex price differences between two brokers. Requires Zorro S and an [account list](account.htm) with entries for the two brokers. 

### BuyHold

SPY buy-and-hold strategy as a benchmark for comparison.

### CalculatePi

Example script that calculates the first 1000 digits of Pi 

### Chart

Opens a t1, t6, or t8 historical data file in the History folder and displays its content in a chart. 

### Control

Example script for the [ZorroControl](engine.htm) integration toolkit ([Zorro S](restrictions.htm) required). 

### CSVfromHistory

Small script for converting price history from Zorro's **.t6** format to CSV. 

### CSVtoHistory

Small example script for converting price history in some **.csv** formats to Zorro's **.t6** format, optionally split into separate year files. The input/output file names and the format definition can be edited in the script. 

### CSVtoOptions

Small example script for converting options EOD history in **.csv** format to Zorro's **.t8** format. The symbol and CSV format definition can be edited in the script. 

### CSVtoTicks

Small example script for converting price history in **.csv** format to Zorro's **.t1** format. The input/output file names and the format definition can be edited in the script. 

### CurrencyStrength

Strategy that exploits the relative [strength of currencies](ccy.htm) and invests in the strongest forex pair. Requires forex history. 

### DateCalculator

Displays the Windows date and the Unix date for a given calendar date that can be set with the sliders. Click [Stop] to terminate. 

### Deeplearn

Short-term machine learning strategy from the article on [Financial Hacker](https://financial-hacker.com/). Requires R with Caret and a deep learning package. Can switch between libraries Deepnet, H2O, MxNet, and Keras/Tensorflow. 

### Deeplearn+

64-bit version of the DeepLearn script, for Python and PyTorch.

### Detrend

Displays the effect of detrending or randomizing price curves. 

### Distribution

Price distribution chart, comparing the distributions of two assets.

### Download

Script for adding new assets or downloading the price history of single assets or asset lists; details under [History](history.htm).

### DownloadEOD

Script for accessing a website, and downloading, converting, and storing price data in Zorro .t6 format, as an example for using HTTP and dataset functions. 

### DTree

Multi-asset [decision tree](advisor.htm) example. Demonstrates why out-of-sample tests for machine learning are mandatory. Requires forex history. 

### Ehlers

The system from the paper "Predictive Indicators for Effective Trading Strategies" by John Ehlers, converted to lite-C by DdlV. With and without [crossover prediction](predict.htm). 

### Filter

Test Zorro's [spectral filter](filter.htm) and cycle detection functions with a sine chirp. 

### Gap

A simple gap trading system based on a book by David Bean as an example of time dependent trading (not really profitable). 

### GapFinder

Finds gaps in historical data and displays them with a red line. 

### Grid

For your grid trading experiments. Better do not trade this system with real money!

###  History

Displays the content of .t8, .t6, and .t1 historical data files in a spreadsheet. Uses the [panel](panel.htm) function and thus requires [Zorro S](restrictions.htm); a history viewer for the free Zorro version is available on the download page. 

### Impulse

Test the impulse response of moving averages, smoothing filters, and lowpass filters. 

### InvestCalculator

Calculates the investment from initial capital and accumulated profit with the square root rule. The exponent can be set with the slider from 1.000 (linear investment) up to 2.000 (square root investment). Click [Stop] to terminate. 

### Keystrokes

Demonstrates controlling other applications by sending [key strokes](keys.htm) or mouse clicks to them. Opens Windows Notepad, writes something in it, saves the text, and closes Notepad. 

### Indicatortest

Displays the curves of popular [indicators](ta.htm). 

### Luxor

System from a [trading book](links.htm) by Jaeckle and Tomasini. Textbook example of an overfitted strategy: better don't trade it outside the backtest period used in the book. GBP/USD history required. 

### Mandelbrot

Example script that uses the Windows API for producing Mandelbrot fractals. 

### Martingale

For your martingale experiments. Better do not trade this one with real money! 

### MinWinRate

Scalping debunker. Plots a histogram of the required win rates for a given asset and trading cost dependent on trade duration in minutes. 

### MRC

Framework for a [Montecarlo Reality Check](testing.htm). Enter the script name to be tested, and run it in [Train] mode. 

### MultiTrain

Example of training both rules and parameters. 

### _New

Strategy template generated by clicking on [New Script]. Rename it afterwards. Don't use scripts names that are very long or have blanks or other non-letter characters. Source in **Source\Template.c**. 

### OpenGL

Example script that demonstrates using external libraries, in this case the OpenGL 3D graphics library. 

### OptimizeTest

Walk-forward optimization of a 12 parameters portfolio strategy with the ascent, brute force, or genetic optimizer. Useful for comparing optimization speed and methods. Forex history required. 

### OptionsCalculator

Calculates the value and delta of call and put options with the Black-Scholes formula. Underlying price, strike distance to the price, and expiration can be set with the sliders, other parameters can be set up in the script. Click [Stop] to terminate. 

### PanelTest

Example script that demonstrates a complex spreadsheet-defined strategy [control panel](panel.htm) ([Zorro S](restrictions.htm) required).

###  Payoff

Interactive script that displays payoff diagrams of arbitrary option combinations. The strike distance can be set with a slider. [R](rbridge.htm) and the [RQuantLib package](contract.htm) must be installed. 

### Perceptron

Simple [machine learning](advisor.htm) system with walk forward analysis. 

### Performance

Script with a function for evaluating arbitrary values from the [performance report](performance.htm), and an example that stores the Montecarlo parameters in a .csv file and prints the 50% level. 

### Predict

Predicts peaks, valleys, and crossovers in a price curve several bars in advance. 

### PriceDist

Compares the price ranges and price distributions of two assets (EUR/CHF and EUR/USD) for finding grid-trading-worth inefficiencies. EUR/CHF history required. 

### Process

Example of two interacting Zorro [processes](engine.htm). Move the "Other" slider and see the effect on the other Zorro ([Zorro S](restrictions.htm) required). 

### PythonTest

Short script for testing the correct installation of [Python](python.htm). 

### RandomPrice

Plots a price curve together with a random curve. Let fellow traders guess which one is which. 

### RandomWalk

Displays a histogram of short-term price movements by random and by real prices.

### RecordT1

Records live prices of the current asset to a **.t1** file in [Trade] mode. A click on [Save] stores the file. [Zorro S](restrictions.htm) required. 

### Regime

Script for testing the response of some indicators to a market regime change from cycles to trending. 

### Rtest

Short script for testing the correct installation of [R](rbridge.htm). 

### Sentiment

Connects to the Interactive Brokers TWS, downloads the SPY options chain, and plots today's [market sentiment](contractcpd.htm) histogram. [Zorro S](restrictions.htm) required. 

### Simulate

Script for generating a chart and [performance report](performance.htm) from a trade list in **.csv** format. Demonstrates how to import data from a spreadsheet to a [dataset](data.htm). The script can be modified for importing other trade lists in different csv formats. 

### SpecialBars

Script illustrating the use of Range, Renko, Haiken Ashi, or Point-and-Figure [bars](bar.htm) for single-asset and multiple-asset strategies. 

### Spectrum

Spectral analysis chart example. SPX500 history required. 

### TradeCosts

Lists the ratio of spread to daily volatility of the main assets. Find out which assests are least expensive to trade. Forex and CFD history required. 

### TradeTest

A script that opens a panel with buttons for manually opening and closing positions in different modes. Useful for testing broker/account features, f.i. if it's a NFA account, if it is FIFO compliant or if partial closing is supported. Description under [Broker Plugin](brokerplugin.htm). 

### TradeOptions

A script that opens a panel with buttons for manually opening and closing options with IB. Set strike and life time, select a combo, then click [Buy] or [Sell]. 

### Turk2

A simple market sentiment system as described in the "Mechanical Turk" article. Not very profitable, but demonstrates the principle. SPY options history required.

### WFOProfile

Framework for a [walk-forward optimization profile](testing.htm). Enter the script name to be tested, and run it in [Train] mode. 

### Workshop1 .. Workshop8

Scripts containing the code examples from the [workshops](tutorial_var.htm). Forex and SPY history required for workshops 6 and 8. 

### Z1 ... Z13

Zorro's [included trade systems](zsystems.htm). Executable systems only; no source code. Forex and CFD history required. Zorro S required for some of the sytems. 

### ZStatus

Small script for displaying the live balances and profits of all your accounts on a single panel (details [here](trading.htm#zstatus); Zorro S required).

### See also:

[Workshops](tutorial_var.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
