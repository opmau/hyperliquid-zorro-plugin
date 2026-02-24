---
title: Colors
url: https://zorro-project.com/manual/en/colors.htm
category: Functions
subcategory: None
related_pages:
- [optimize](optimize.htm)
- [panel, ...](panel.htm)
- [color](color.htm)
- [plot, plotBar, ...](plot.htm)
- [Performance Report](performance.htm)
---

# Colors

# Colors

## ColorUp

## ColorDn

Colors for the white and black candles (default: **ColorUp** = **0x00CCCCCC** = bright grey, **ColorDn** = **0x00222222** = dark grey). If both colors are **0** , no price candles are plotted in the main chart. 

## ColorEquity

Color for the equity bars (default: **0x00666699** = blue-grey). If at **0** , no equity curve is plotted in the main chart. 

## ColorDD

Color for the "underwater equity" bars (default: **0x00FF3333** = red). If at **0** , no underwater equity is plotted in the main chart. 

## ColorWin

## ColorLoss

Colors for the winning and losing trades (default: **ColorWin** = **0x0033FF33** = green, **ColorLoss** = **0x00FF3333** = red). If both colors are **0** , no trades are plotted in the main chart. 

## ColorBars[3]

Colors for the bars on [parameter charts](optimize.htm). Default: **ColorBars[0]** = objective (red), **ColorBars[1]** = wins (blue), **ColorBars[2]** = losses (blue-grey). 

## ColorPanel[6]

Default colors for the cells on a [control panel](panel.htm). **ColorPanel[0]** = text background (light grey), **ColorPanel[1]** = number background (reddish), **ColorPanel[2]** = editable (white), **ColorPanel[3]** = button (blue), **ColorPanel[4]** = highlighted text (red), **ColorPanel[5]** = grayed out text (grey). 

### Range:

**0..0xFFFFFFFF**

### Type:

**long**

### Remarks:

  * Colors can be defined as hexadecimal numbers just as in HTML pages, in the form **0xTTRRGGBB** (f.i. **0x000000FF** = opaque blue, **0x80FF0000** = half transparent red). The transparency**TT** runs from **0** (full opaque) over **80** (half transparent) to **FF** (full transparent). 
  * For convenience reasons, use the macro **rgb(Red,Green,Blue)** with colors in the **0.255** range.
  * Some colors are predefined: **RED** , **GREEN** , **BLUE** , **CYAN** , **DARKBLUE** , **LIGHTBLUE** , **PURPLE** , **YELLOW** , **MAGENTA** , **ORANGE** , **DARKGREEN** , **OLIVE** , **MAROON** , **SILVER** , **GREY** , **BLACK**. **+TRANSP** can be added for 50% transparency. 
  * For generating color ranges dependent on a variable, use the [color](color.htm) function. 

### Example:

```c
function run()
{
  ColorEquity = rgb(0,255,0); // green equity bars, same as 0x0000FF00
  ColorDD = 0; // no underwater equity
  ...
}
```

### See also:

[plot](plot.htm), [color](color.htm), [plotScale](plotscale.htm), [performance](performance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
