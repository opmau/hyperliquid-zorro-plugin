---
title: sound
url: https://zorro-project.com/manual/en/sound.htm
category: Functions
subcategory: None
related_pages:
- [Zorro.ini Setup](ini.htm)
- [printf, print, msg](printf.htm)
- [progress](progress.htm)
---

# sound

## sound(string name)

Plays a **.wav** sound file. 

### Parameters:

**name** | Name of the sound file, f.i. **"test.wav"**. If the file is not found in the Zorro folder, a standard chime is played.   
---|---  
  
###  Remarks:

  * Sounds are not played when [Mute](ini.htm) is set. 

### Example:

```c
sound("win.wav");
```

### See also:

[printf](printf.htm), [progress](progress.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
