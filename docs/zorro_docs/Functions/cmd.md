---
title: Command, Define
url: https://zorro-project.com/manual/en/cmd.htm
category: Functions
subcategory: None
related_pages:
- [Command Line Options](command.htm)
- [Retraining](retraining.htm)
---

# Command, Define

## Command[0] .. Command[3] 

Contains the numbers passed with the **-i** option on the [command line](command.htm) (Zorro S only).   

### Type:

**int**

## Define

The name given with the **-d** option on the [command line](command.htm) (Zorro S only). 

### Type:

**string**

### Remarks:

  * The **Command** variables are used for passing the live trading start date and bar offset to a Zorro instance in [Retest mode](retraining.htm). 

### Example:

```c
function run()
{
  switch(Command[0]) {
    case 1: doThis(); break;
    case 2: doThat(); break;
  }
  ...
}
```

### See also:

[Command line](command.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
