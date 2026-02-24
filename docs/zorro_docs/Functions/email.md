---
title: email
url: https://zorro-project.com/manual/en/email.htm
category: Functions
subcategory: None
related_pages:
- [HTTP functions](http.htm)
- [FTP transfer](ftp.htm)
---

# email

## email (string To, string From, string Subject, string Body, string Server, string User, string Password): int

Sends an email to a given recipient. Useful for receiving emails when Zorro opens or closes a trade, or for status reports. 

### Parameters:

**To** \- Recipient address as required by the server. Usually in angular brackets like **" <me@myself.com>"**.  
**From** \- Sender address, usually in angular brackets.  
**Subject** \- Subject line of the email, and optionally content types.  
**Body** \- Text content of the email, max 16k.  
**Server** \- SMTP/SMTPS server with optional port address, f.i. **"smtp://smtp.1und1.de"** or **"smtps://smtp.gmail.com:465"**.  
**User** \- Login name for the SMTP server, often identical to the sender address.  
**Passwort** \- Login password for the SMTP server. 

###  Returns:

**0** if the email could not be sent, otherwise nonzero. 

### Remarks:

  * Check with your email provider about any special requirements for sending emails from their SMTP server. 
  * The following instruction for sending from Gmail accounts was provided by a user. Log in to your google account. In Google Security turn off "Less Secure Apps". Enable 2-Step authentication. Search for "App Passwords", and under "Select app" select "Other", under "Device type" enter "Zorro", then press the [Generate] button. This produces a 16-digit passcode that you can then used in the email function in place of your Gmail account password. For the server, use "smtps://smtp.gmail.com:465".
  * For special content types, f.i. HTML, use a new line at the end of the **Subject** line, f.i. **"My subject \nContent-Type: text/html; charset=UTF-8\n";**.  

### Example:

```c
void main()
{
  string To = "<me@myself.org>";
  string From = "<zorro_alert@gmail.com>";
  string Subject = "Zorro Message";
  string Body = "Zorro has sent you a message!";
  string Server = "smtps://smtp.gmail.com:465";
  string User = "zorro_alert@gmail.com";
  string Password = "zorros_password";
  email(To,From,Subject,Body,Server,User,Password);
}
```

### See also:

[HTTP functions](http.htm), [FTP functions](ftp.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
