---
title: login
url: https://zorro-project.com/manual/en/login.htm
category: Functions
subcategory: None
related_pages:
- [Brokers & Data Feeds](brokers.htm)
- [Broker API](brokerplugin.htm)
---

# login

## login (int mode): int 

Logs in or out to the broker API.. 

### Parameters:

**mode** \- **0** for checking the login state, **1** for logging in, **2** for logging out, **4** for completely closing the broker API. **  
**

### Returns:

**1** when Zorro is logged in, **0** otherwise. 

### Remarks:

  * For recovering from a broker API crash, call **login(4)** , then **login(1)**.
  * Zorro normally logs in and out automatically, so this function is for special purposes only. 

### Example:

```c

```

### See also:

[Brokers](brokers.htm), [broker plugin](brokerplugin.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
