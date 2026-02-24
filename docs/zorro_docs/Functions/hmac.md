---
title: hmac
url: https://zorro-project.com/manual/en/hmac.htm
category: Functions
subcategory: None
related_pages:
- [brokerCommand](brokercommand.htm)
- [HTTP functions](http.htm)
- [FTP transfer](ftp.htm)
---

# hmac

## hmac(string Content, int Size, string Secret, int Mode): string

Generates a cryptographic hash from the string **Content** with an optional **Secret** for authentication purposes. 

### Parameters:

**Content** \- Message or data to be hashed.  
**Size** \- **Content** size in bytes, or **0** for using the string length of **Content**.  
**Secret** \- Secret string for generating a cryptographic hash, or **0** for a simple SHA512 or SHA256 hash.  
**Mode** \- **512** for a HMAC512 hash, **256** for a HMAC256 hash. **+64** when **Secret** is base64 encoded. 

###  Returns:

Hash string of the given **Content** , or **0** when something was wrong.

### Remarks:

  * This function is often used for signing REST API headers. If can be accessed in a broker plugin either by adding Zorro functions with the [ZorroDll.cpp](dlls.htm#project), or with the [SET_FUNCTIONS](brokercommand.htm) command.  

### Example:

```c
char Header[1024]M
strcpy(Header,"Content-Type:application/json");
strcat(Header,"\nAccept:application/json");
strcat(Header,"\nApi-Key: ");
strcat(Header,MyKey);
strcat(Header,"\nApi-Signature: ");
strcat(Header,hmac(MyContent,0,MySecret,512));
```

### See also:

[HTTP functions](http.htm), [FTP functions](ftp.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
