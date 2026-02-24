---
title: http functions
url: https://zorro-project.com/manual/en/http.htm
category: Functions
subcategory: None
related_pages:
- [Broker API](brokerplugin.htm)
- [String handling](str_.htm)
- [FTP transfer](ftp.htm)
- [File access](file_.htm)
- [email](email.htm)
---

# http functions

# HTTP functions

The HTTP functions can retreive or scrape web content - prices, signals, news, or indicators - and use them in the trading strategy. They can also execute PHP scripts and can be used by a [broker plugin](brokerplugin.htm) for communicating with a REST or FIX API. 

## http_transfer (string URL, string Data): string

Transfers data to or from the web address given by **URL**. The function waits until the web page content or the PHP script response is transferred back to Zorro. The **data** string can contain arguments to be passed to a PHP script (such as the content of an email). The function returns a temporary string containing the web page content for further evaluation; the content remains valid until the next **http_transfer** call. The number of bytes received can be retrieved with **http_status(0)**. 

## http_request (string Url, string Data, string Header, string Method): int 

Sends a HTTP request to the given **URL** , and returns a identifier number (**id**) for controlling the data transfer with **http_status**. The **Data** string can contain arguments to be passed to a PHP script, such as the content of an email. The request type, such as **"GET"** , **"POST"** , **"PATCH"** etc. is given in **Method**. If **Data** and **Method** are **0** , the request type defaults to **GET**. The optional **Header** string can contain custom headers (f.i. for authorization) separated by line feeds (**"\n"**). After the transfer is finished, the identifier number must be freed with **http_free**. 

## http_proxy (string proxy, int port) 

Sets up a HTTP proxy server for the connection to the Internet. If no proxy is used, or if you don't know what a HTTP proxy is, you won't need this function. 

## http_status (int id): long 

Returns the number of bytes received with the HTTP data transfer with the identifier **id** , or the number of bytes received with the last **http_transfer** call when **id == 0** , or the transfer status info: **0** \- transfer is still in progress; **-1** \- transfer failed; **-2** \- **id** is invalid; **-3** \- website did not respond; **-4** \- host could not be resolved.. 

## http_result (int id, string Content, long Size): long 

Stores the received data of a HTTP data transfer in the **content** string, up to **size** bytes, and returns the received number of bytes. A string terminator (0) is added at the end of the received data.  

## http_free (int id)

FFrees the identifier number of a HTTP data transfer and stops the transfer. Must be called at the end of each HTTP data transfer. For stopping the most recent transfer, set **id** to **0**. 

### Parameters:

**proxy** \- proxy server name (example: **"proxy-host.com"**).   
**port** \- port of the proxy server (example: **8080**)).  
**Url** \- URL of the web page (examples: **"<http://opserver.de/scratch/ip.php>"** or **"https://www.google.com/finance"**)).   
**Data** \- **string** (or **char** array) containing the data to be sent, for instance a JSON record.   
**Content** \- **string** (or **char** array) to receive the web page content or the result data of a HTTP POST operation.  
**Header** \- one or several header strings separated by line feeds (**'\n'**):  
**Size** \- maximum number of characters to be received, or **0** for using the actual **content** string length.   
**id** \- identifier number, returned by **http_post()**.  

### RRemarks:

  * Up to 4 HTTP data transfers can be run at the same time using **http_request**. If only a single transfer is running at any time, the **id** remains the same and needs not be freed between transfers. 
  * All functions return **0** if an error occured, and nonzero if everything went ok (if not otherwise mentioned in the function description). 
  * The **data** string can be accessed in a PHP script through the $_POST string array (see example). String variables can be named with **'='** and different variables can be separated with **' &'**, as in **"user=John &password=Secret"**. 
  * The [strvar](str_.htm) function can be used to parse numbers on certain locations of web pages.

### Examples:

```c
// access the CFTC website, read the current Commitment Of Traders Report,
// and store it in a file.
function main(){
  file_delete("cot.txt"); // delete previous report
  file_append("cot.txt",
    http_transfer("http://www.cftc.gov/dea/futures/deacmesf.htm",0));
}
```

```c
// get current date and time from the Internet
function main(){
  string Response = http_transfer("http://worldtimeapi.org/api/ip",0);
  string Date = strtext(Response,"\"datetime\":","0");
  printf("\n%s",Date);
}
```

```c
// Download historical EOD price data from Google
function main()
{
  string Code = "SPY";
  string URL = strf("https://www.google.com/finance/historical?q=%s&startdate=01-Jan-2000&output=csv",Code);
  string Content = http_transfer(URL,0);
  if(!Content) continue;
  file_write("History\\history.csv",Content,0);
  dataNew(1,0,7);
  if(dataParse(1,"%d-%b-%y,f3,f1,f2,f4,f6","History\\history.csv"))
    dataSave(1,strf("History\\%s.t6",Code));
}
```

```c
// Download earnings data from AlphaVantage 
// and store earnings surprises in a dataset
int loadEarnings()
{
  string URL = strf("https://www.alphavantage.co/query?function=EARNINGS&symbol=%s&apikey=%s",
    Asset,report(33));  // 33 = AlphaVantage API key
  string Content = http_transfer(URL,0); // JSON format
  if(!Content) return 0; 
  string Pos = strstr(Content,"reportedDate");  
  if(!Pos) return 0;
  dataNew(1,0,0);
  while(Pos) {
    var Date = wdatef("%Y-%m-%d",Pos+12+4);
    if(Date == 0) break;
    Pos = strstr(Pos,"surprisePercentage");
    if(!Pos) break;
    int Row = dataRow(1,dataAppendRow(1,2));
    dataSet(1,Row,0,Date); // earnings date in field 0
    var Surprise = atof(Pos+18+4);
    dataSet(1,Row,1,Surprise); // surprise in fields 1
    printf("\n%.4f %.4f",dataVar(1,Row,0),dataVar(1,Row,1));
    Pos = strstr(Pos,"reportedDate");
  }
  return 1;
}
```

```c
// start the script "ip.php" on a remote server, and // print dots until Zorro's IP address is returned 
function main()
{
  char ip_str[100]; // just a long empty string
   int id = http_request("http://myserver.de/php/ip.php",0,0,0);
  if(!id) return;
  while(!http_status(id)) {
    if(!wait(100)) return; // wait for the server to reply
    printf(".");
  }  if(http_status(id) > 0) { //transfer successful?
    http_result(id,ip_str,100);   //get the replied IP
    printf("\n%s",ip_str);
  } else
    printf("\nError during transfer!");
  http_free(id); //always clean up the id!
}
ip.php:
<?php  echo "Your IP address: " . $_SERVER['REMOTE_ADDR'];?>
```

```c
// send an email when a trade is entered
function sendEmailAboutTrade()
{
// compose the message
  string Content = strf("content=Zorro has entered a trade!\n%d Lots of %s",
    Lots,Asset);
  http_transfer("http://www.myserver.com/zorromail.php",Content);
}

zorromail.php:
<?php $to = "me@myself.com"; $subject = "Message from Zorro!"; $body = $_POST['content']; mail($to,$subject,$body);?>
```

```c
// write text to a file on a server
function sendEmailAboutTrade()
{
// compose the message
  string Content = "content=This is my file content");
  string echo = http_transfer("http://www.myserver.com/store.php",Content);
  printf("\nResponse: $s",echo);
}

store.php:
<?php$file = 'file.txt';
$data = $_POST['content'];
file_put_contents($file,$data);
echo "Stored in " . $file;
?>
```

### See also:

[ftp functions](ftp.htm), [file functions](file_.htm), [email](email.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
