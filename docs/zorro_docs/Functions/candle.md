---
title: Candle patterns
url: https://zorro-project.com/manual/en/candle.htm
category: Functions
subcategory: None
related_pages:
- [Time zones](assetzone.htm)
- [BarMode](barmode.htm)
- [adviseLong, adviseShort](advisor.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [run](run.htm)
- [Indicators](ta.htm)
- [frechet](detect.htm)
---

# Candle patterns

# Traditional Candle Patterns

Japanese rice market candle patterns, returning an integer value of **-100** for bearish patterns, **+100** for bullish patterns, and **0** for no pattern match. They use the current asset price series and detect the pattern at the current bar. All candle pattern functions are listed below in alphabetical order. They are based on the TA-Lib indicator library by Mario Fortier ([**www.ta-lib.org**](http://www.ta-lib.org)). 

**Disclaimer:** Traditional candle patterns are implemented in Zorro for the sake of completeness, but are not really recommended for serious algorithmic trading. The patterns have been established by Japanese traders for the local rice markets in the 18th century. They probably were indeed useful back then. But no serious tests found any predictive value in any of the patterns for today's stock and forex markets. If you still want to use them, be aware that the same price curve can produce very different candle patterns dependent on [time zone](assetzone.htm), [bar mode](barmode.htm), and price type. Many of the patterns won't appear in assets that are traded around the clock, such as forex pairs, because their candles have normally no difference between close and next open. For finding patterns with real predictive value, use the [pattern analyzer](advisor.htm).

## CDL2Crows(): int

Two Crows, a bearish candle pattern.   

## CDL3BlackCrows(): int

Three Black Crows.  

## CDL3Inside(): int

Three Inside Up/Down.  

## CDL3LineStrike(): int

Three-Line Strike.  

## CDL3Outside(): int

Three Outside Up/Down.  

## CDL3StarsInSouth(): int

Three Stars In The South.  

## CDL3WhiteSoldiers(): int

Three Advancing White Soldiers.  

## CDLAbandonedBaby(var Penetration): int

Abandoned Baby. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLAdvanceBlock(): int

Advance Block.  

## CDLBeltHold(): int

Belt-hold.  

## CDLBreakaway(): int

Breakaway. Bullish + Bearish.  

## CDLClosingMarubozu(): int

Closing Marubozu.  

## CDLConcealBabysWall(): int

Concealing Baby Swallow.  

## CDLCounterAttack(): int

Counterattack.  

## CDLDarkCloudCover(var Penetration): int

Dark Cloud Cover. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLDoji(): int

Doji, single candle pattern. Trend reversal.  

## CDLDojiStar(): int

Doji Star. Bullish + Bearish.  

## CDLDragonflyDoji(): int

Dragonfly Doji.  

## CDLEngulfing(): int

Classic Engulfing Pattern. Bullish + Bearish. 

## CDLEngulfing0(): int

Engulfing pattern including close-open equality, therefore also usable for Forex. Bullish + Bearish. Source code in **indicators.c**. 

## CDLEveningDojiStar(var Penetration): int

Evening Doji Star. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLEveningStar(var Penetration): int

Evening Star. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLGapSideSideWhite(): int

Up/Down-gap side-by-side white lines.  

## CDLGravestoneDoji(): int

Gravestone Doji.  

## CDLHammer(): int

Hammer. Bullish.   

## CDLHangingMan(): int

Hanging Man. Bearish.  

## CDLHarami(): int

Harami Pattern. Bullish + Bearish.  

## CDLHaramiCross(): int

Harami Cross Pattern.  

## CDLHignWave(): int

High-Wave Candle.  

## CDLHikkake(): int

Hikkake Pattern. Bullish + Bearish.  

## CDLHikkakeMod(): int

Modified Hikkake Pattern.  

## CDLHomingPigeon(): int

Homing Pigeon.  

## CDLIdentical3Crows(): int

Identical Three Crows.  

## CDLInNeck(): int

In-Neck Pattern.  

## CDLInvertedHammer(): int

Inverted Hammer.  

## CDLKicking(): int

Kicking.  

## CDLKickingByLength(): int

Kicking - bull/bear determined by the longer marubozu.  

## CDLLadderBottom(): int

Ladder Bottom.  

## CDLLongLeggedDoji(): int

Long Legged Doji.  

## CDLLongLine(): int

Long Line Candle.  

## CDLMarubozu(): int

Marubozu.  

## CDLMatchingLow(): int

Matching Low.  

## CDLMatHold(var Penetration): int

Mat Hold. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLMorningDojiStar(var Penetration): int

Morning Doji Star. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLMorningStar(var Penetration): int

Morning Star. Parameters: **Penetration** (Percentage of penetration of a candle within another candle).  

## CDLOnNeck(): int

On-Neck Pattern. 

## CDLOutside(): int

Engulfing including wicks. Bullish + Bearish. Source code in **indicators.c**.  

## CDLPiercing(): int

Piercing Pattern.  

## CDLRickshawMan(): int

Rickshaw Man.  

## CDLRiseFall3Methods(): int

Rising/Falling Three Methods.  

## CDLSeperatingLines(): int

Separating Lines.  

## CDLShootingStar(): int

Shooting Star.  

## CDLShortLine(): int

Short Line Candle.  

## CDLSpinningTop(): int

Spinning Top.  

## CDLStalledPattern(): int

Stalled Pattern.  

## CDLStickSandwhich(): int

Stick Sandwich.  

## CDLTakuri(): int

Takuri (Dragonfly Doji with very long lower shadow).  

## CDLTasukiGap(): int

Tasuki Gap.  

## CDLThrusting(): int

Thrusting Pattern.  

## CDLTristar(): int

Tristar Pattern.  

## CDLUnique3River(): int

Unique 3 River.  

## CDLUpsideGap2Crows(): int

Upside Gap Two Crows.  

## CDLXSideGap3Methods(): int

Upside/Downside Gap Three Methods.  
  

### Returns:

**-100** for a bearish pattern, **+100** for a bullish pattern, and **0** for no pattern match at the current bar.**  
**

### Remarks:

  * The **TA-Lib** function prototypes are defined in **include\functions.h**. The source code of the TA-Lib indicators is included in the **Source** folder. 
  * For different time zones, use [BarZone](assetzone.htm) or add a [BarOffset](barperiod.htm) (if you think it matters). 
  * At the initial [run](run.htm) of the strategy when no price history was yet loaded, all CDL functions return **0**.

### Example:

```c
function run()
{
  set(PLOTNOW);  MaxBars = 500;  PlotScale = 8;
// mark patterns with triangles on the chart
  if(CDLDoji())    plot("Doji",1.002*priceHigh(),TRIANGLE4,BLUE);  if(CDLHikkake() > 0)    plot("Hikkake+",0.998*priceLow(),TRIANGLE,GREEN);  if(CDLHikkake() < 0)    plot("Hikkake-",1.002*priceHigh(),TRIANGLE4,RED);

// go long 3 bars on a bullish Hikkake, short on a bearish Hikkake
  LifeTime = 3;
  if(CDLHikkake() > 0)
    enterLong();
  else if(CDLHikkake() < 0)
    enterShort();
}
```

### See also:

[Indicators](ta.htm), [curve form detection](detect.htm), [pattern analyzer](advisor.htm). 

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
