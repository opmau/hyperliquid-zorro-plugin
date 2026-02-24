---
title: color
url: https://zorro-project.com/manual/en/color.htm
category: Functions
subcategory: None
related_pages:
- [Colors](colors.htm)
- [plot, plotBar, ...](plot.htm)
- [panel, ...](panel.htm)
---

# color

## color (var Value _,_ int Color1, int Color2, _int Color3, int Color4_) : int 

Returns the color corresponding to a position within a range of up to 4 colors; used for plotting heatmaps or multiple curves. 

### Parameters:

**Value** | The color position within the range in percent; from **0** for **color1** until **100** for **color4**.   
---|---  
**Color1..Color4** | The color range, starting with **color1** and ending with **color4**. For 2-color or 3-color ranges, **color3** and **color4** can be omitted. See [Colors](colors.htm) for the color format.   
  
### Returns

Color within the range

## colorScale (int Color, var Scale) : int 

Returns the color multiplied with a scaling factor. 

### Parameters:

**Color** | The color to be scaled.   
---|---  
**Scale** | The scaling factor in percent, < 100 for reducing the brightness and > 100 for increasing it.   
  
### Returns

**Color * Factor**. 

### Remarks:

  * **color(Value,WHITE,BLACK)** produces a greyscale. 
  * **colorScale(Color,110)** increases the brightness of **Color** by 10%.
  * For putting a color together from RGB values, use the expression **(Red <<16)+(Green<<8)+Blue**.  

### Example:

```c
for(i=0; i<N; i++)  for(j=0; j<N; j++)    plotGraph("Heatmap",j+1,-i-1,SQUARE,color(Correlations[i][j]*100,BLUE,RED));
```

### See also:

[plot](plot.htm), [](panel.htm)[Colors](colors.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
