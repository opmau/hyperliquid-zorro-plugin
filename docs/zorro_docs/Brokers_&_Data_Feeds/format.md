---
title: Format codes and tables
url: https://zorro-project.com/manual/en/format.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [Time / Calendar](month.htm)
- [Dataset handling](data.htm)
- [printf, print, msg](printf.htm)
- [String handling](str_.htm)
- [Script Editor](npp.htm)
- [Variables](aarray.htm)
- [Content](content.htm)
---

# Format codes and tables

# Format Codes - C Programmer's Cheat Sheet

### DATE format codes, for [wdate](month.htm), [strdate](month.htm), **[dataParse](data.htm)**, etc:

**%a** | Abbreviated weekday name  
---|---  
**%A** | Full weekday name  
**%b** | Abbreviated month name  
**%B** | Full month name  
**%d** |  Day of month as decimal number (01..31)  
**%H** | Hour in 24-hour format (00..23)   
**%j** | Day of year as decimal number (001..366)  
**%m** | Month as decimal number (01..12)  
**%M** | Minute as decimal number (00..59)  
**%S** | Second (00..59) or second with decimals (**dataParse** , **wdatef** , 00.000000 .. 59.999999)   
**%S.** | Second with milliseconds (**strdate** only)  
**%t** | Unix time stamp in seconds, milliseconds, or microseconds since 1/1/1970 (**dataParse** , **wdatef**)  
**%U** | Week of year as decimal number, with Sunday as first day of week (00..53)   
**%W** | Week of year as decimal number, with Monday as first day of week (00..53)  
**%y** |  Year without century, as decimal number (00..99)  
**%Y** | Year with century, as decimal number (1900..2999)   
**%%** | Percent sign  
  
The **DATE** format is equivalent to a **double** / **var** variable and counts the number of days since 12-31-1899 with a resolution of 1 µsec.

### Variable / string format codes, for [printf](printf.htm), [print](printf.htm), [strf](str_.htm), etc:

Format specifiers:  
---  
**%c** | ANSI character (int, char)   
**%d** | Decimal number (int, char)  
**%e** | Exponential floating-point number (var, double)   
**%f** | Floating-point number (var, double)   
**%g** | Either **%e** or **%f** , whichever is more compact  
**%i** | Decimal number (int, char)  
**%o** | Octal number (int, char)  
**%p** | Hexadecimal pointer (void*)   
**%s** | Null-terminated text (string, char*, char[])   
**%u** | Unsigned decimal number (int, char)  
**%x** | Hexadecimal number (int, char), lowercase   
**%X** | Hexadecimal number (int, char), uppercase   
**%%** | Percent sign  
|   
Format modifiers (optional, after the %) :   
**n** | Field width, minimum number of digits.  
**.m** | Precision, maximum number of digits after the decimal.  
**-** | Left align the result within the given field width.  
**+** | Prefix the output value with a sign (+ or –).  
**0** | Add zeros until the number of digits is reached.  
**' '** | Add a blank ' ' if the output value is signed and positive.  
|   
Special characters  
**\a** | Audible alert  
**\b** | Backspace  
**\f** | Form feed  
**\n** | New line, or linefeed  
**\r** | Carriage return  
**\t** | Tab  
**\v** | Vertical tab  
**\\\** | Backslash  
**#...** | Print to log only   
  
### Regular expressions, for the [Npp](npp.htm) search functions

Code | Matches | Example  
---|---|---  
***** | Preceding item zero or more times; likeÂ **{0,}**. | zo*Â matches "z" and "zoo".  
**+** | Preceding item one or more times; likeÂ **{1,}**. | zo+Â matches "zo" and "zoo", but not "z".  
**?** | Preceding item zero or one time; likeÂ **{0,1}**. | zo?Â matches "z" and "zo", but not "zoo".  
**^** | The position at string start.  | ^\d{3}Â matches 3 numeric digits at the start of the searched string.   
**$** | The position at string end. | \d{3}$Â matches 3 numeric digits at the end of the searched string.  
**.** | Any single character except newline **\n**.  | a.cÂ matches "abc", "a1c", and "a-c".  
**|** | Choice between two or more items. | z|foodÂ matches "z" or "food".Â (z|f)oodÂ matches "zood" or "food".  
**\\*** | The ***** character; similar with **\\+** , **\?** , **\\.** , **\\[** , **\\(** , etc.  |   
**\b** | Word boundary. | er\bÂ matches the "er" in "never" but not the "er" in "verb".  
**\B** | Word non-boundary. | er\BÂ matches the "er" in "verb" but not the "er" in "never".  
**\d** | Digit character; equivalent toÂ **[0-9]**. | In "12 345",Â \d{2} matches "12" and "34".Â \dÂ matches "1", 2", "3", "4", "5".  
**\D** | Nondigit character; equivalent toÂ **[^0-9]**. | \D+Â matches "abc" and " def" in "abc123 def".  
**\s** | Any white-space character; like **[ \f\n\r\t\v]**. |   
**\S** | Any non-white-space character; like **[^ \f\n\r\t\v]**. |   
**\w** | **A-Z** , **a-z** , **0-9** , and underscore, likeÂ **[A-Za-z0-9_]**. | In "The quick brown foxâ¦",Â \w+Â matches "The", "quick", "brown", "fox".  
**\W** | Any character except **A-Z** , **a-z** , **0-9** , underscore. | In "The quick brown foxâ¦",Â \W+Â matches "â¦" and all of the spaces.  
**()** | Start and end of a subexpression. | A(\d)Â matches "A0" to "A9". (f|b)*oo matches foo, boo, and oo.   
**[abc]** | A character set; matches any specified character. | [abc]Â matches the "a" in "plain".  
**[^abc]** | Negative character set; any not specified character. | [^abc]Â matches the "p", "l", "i", and "n" in "plain".  
**[a-z]** | A range of characters; any character in the range. | [a-z]Â matches any lowercase character in the range "a" through "z".  
**[^a-z]** | Negative range; any character not in the range. | [^a-z]Â matches any character that is not in the range "a" through "z".  
**[a,b]** | All items separated with commas. | [a-z,-]Â matches all characters in "my-file".  
**{n}** | Preceding item exactlyÂ **n** Â times.Â  | o{2}Â does not match the "o" in "Bob", but the two "o"s in "food".  
**{n,}** | Preceding item at leastÂ **n** Â times.Â  | o{2,}Â does not match the "o" in "Bob" but all the "o"s in "foooood".  
**{n,m}** | Preceding item at leastÂ **n** Â and at mostÂ **m** Â times.Â  | In "1234567",Â \d{1,3} matches "123", "456", and "7".  
**\cx** | The control character indicated byÂ **x**.  | \cMÂ matches a CTRL+M or carriage return character.  
**\xnn** | A 2 digits hexadecimal character.  | \x41Â matches "A".Â (\x041Â is wrong!)  
**\unnnn** | A 4 digits Unicode character. | \u00A9Â matches the copyright symbol (Â©).  
  
### ANSI table (ISO 8859), for special characters in [ strings](aarray.htm)

32 | **[ ]** | 33 | **!** | 34 | **"** | 35 | **#** | 36 | **$** | 37 | **%** | 38 | **&** | 39 | **'**  
---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---  
40 | **(** | 41 | **)** | 42 | ***** | 43 | **+** | 44 | **,** | 45 | **-** | 46 | **.** | 47 | **/**  
48 | **0** | 49 | **1** | 50 | **2** | 51 | **3** | 52 | **4** | 53 | **5** | 54 | **6** | 55 | **7**  
56 | **8** | 57 | **9** | 58 | **:** | 59 | **;** | 60 | **<** | 61 | **=** | 62 | **>** | 63 | **?**  
64 | **@** | 65 | **A** | 66 | **B** | 67 | **C** | 68 | **D** | 69 | **E** | 70 | **F** | 71 | **G**  
72 | **H** | 73 | **I** | 74 | **J** | 75 | **K** | 76 | **L** | 77 | **M** | 78 | **N** | 79 | **O**  
80 | **P** | 81 | **Q** | 82 | **R** | 83 | **S** | 84 | **T** | 85 | **U** | 86 | **V** | 87 | **W**  
88 | **X** | 89 | **Y** | 90 | **Z** | 91 | **[** | 92 | **\** | 93 | **]** | 94 | **^** | 95 | **_**  
96 | **`** | 97 | **a** | 98 | **b** | 99 | **c** | 100 | **d** | 101 | **e** | 102 | **f** | 103 | **g**  
104 | **h** | 105 | **i** | 106 | **j** | 107 | **k** | 108 | **l** | 109 | **m** | 110 | **n** | 111 | **o**  
112 | **p** | 113 | **q** | 114 | **r** | 115 | **s** | 116 | **t** | 117 | **u** | 118 | **v** | 119 | **w**  
120 | **x** | 121 | **y** | 122 | **z** | 123 | **{** | 124 | **|** | 125 | **}** | 126 | **~** | 127 |   
128 | **â¬** | 129 | **â¢** | 130 | **â** | 131 | **Æ** | 132 | **â** | 133 | **â¦** | 134 | **â** | 135 | **â¡**  
136 | **Ë** | 137 | **â°** | 138 | **Å** | 139 | **â¹** | 140 | **Å** | 141 |  | 142 | **Å¢** | 143 |   
144 | **â¢** | 145 | **Å** | 146 | **Å** | 147 | **Å** | 148 | **Å** | 149 | **â¢** | 150 | **Å** | 151 | **Å**  
152 | **Ë** | 153 | **â¢** | 154 | **Å¡** | 155 | **âº** | 156 | **Å** | 157 | **â¢** | 158 | **Å£** | 159 | **Å¸**  
160 |  | 161 | **Â¡** | 162 | **Â¢** | 163 | **Â£** | 164 | **Â¤** | 165 | **Â¥** | 166 | **Â¦** | 167 | **Â§**  
168 | **Â¨** | 169 | **Â©** | 170 | **Âª** | 171 | **Â«** | 172 | **Â¬** | 173 |  | 174 | **Â®** | 175 | **Â¯**  
176 | **Â°** | 177 | **Â±** | 178 | **Â²** | 179 | **Â³** | 180 | **Â´** | 181 | **Âµ** | 182 | **Â¶** | 183 | **Â·**  
184 | **Â¸** | 185 | **Â¹** | 186 | **Âº** | 187 | **Â»** | 188 | **Â¼** | 189 | **Â½** | 190 | **Â¾** | 191 | **Â¿**  
192 | **Ã** | 193 |  | 194 | **Ã** | 195 | **Ã** | 196 | **Ã** | 197 | **Ã** | 198 | **Ã** | 199 | **Ã**  
200 | **Ã** | 201 | **Ã** | 202 | **Ã** | 203 | **Ã** | 204 | **Ã** | 205 |  | 206 | **Ã** | 207 |   
208 | **ï¿½?** | 209 | **Ã** | 210 | **Ã** | 211 | **Ã** | 212 | **Ã** | 213 | **Ã** | 214 | **Ã** | 215 | **Ã**  
216 | **Ã** | 217 | **Ã** | 218 | **Ã** | 219 | **Ã** | 220 | **Ã** | 221 |  | 222 | **Ã** | 223 | **Ã**  
224 | **Ã** | 225 | **Ã¡** | 226 | **Ã¢** | 227 | **Ã£** | 228 | **Ã¤** | 229 | **Ã¥** | 230 | **Ã¦** | 231 | **Ã§**  
232 | **Ã¨** | 233 | **Ã©** | 234 | **Ãª** | 235 | **Ã«** | 236 | **Ã¬** | 237 | **Ã­** | 238 | **Ã®** | 239 | **Ã¯**  
240 | **Ã°** | 241 | **Ã±** | 242 | **Ã²** | 243 | **Ã³** | 244 | **Ã´** | 245 | **Ãµ** | 246 | **Ã¶** | 247 | **Ã·**  
248 | **Ã¸** | 249 | **Ã¹** | 250 | **Ãº** | 251 | **Ã»** | 252 | **Ã¼** | 253 | **Ã½** | 254 | **Ã¾** | 255 | **Ã¿**  
  
### 

[Zorro Manual](content.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
