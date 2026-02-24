---
title: Installation, VPS
url: https://zorro-project.com/manual/en/vps.htm
category: Brokers & Data Feeds
subcategory: None
related_pages:
- [FXCM](fxcm.htm)
- [Strategy Coding, 1-8](tutorial_var.htm)
- [Visual Studio](dlls.htm)
- [Zorro.ini Setup](ini.htm)
- [Command Line Options](command.htm)
- [R Bridge](rbridge.htm)
- [Python Bridge](python.htm)
- [Licenses](restrictions.htm)
- [Asset and Account Lists](account.htm)
- [set, is](mode.htm)
- [System strings](script.htm)
- [Multiple cycles](numtotalcycles.htm)
- [Integrating Zorro](engine.htm)
- [putvar, getvar](putvar.htm)
- [loadStatus, saveStatus](loadstatus.htm)
- [email](email.htm)
- [HTTP functions](http.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Brokers & Data Feeds](brokers.htm)
- [Getting Started](started.htm)
---

# Installation, VPS

# Installation and deployment. Cloud services. 

Some tips for running Zorro on your home PC or on a web server:

  * While it is recommended to use a recent Windows version for trading, Zorro runs on any version from XP or Server 2012 on. The 64-bit version Zorro64 runs on 64-bit Windows 7 or above. Some broker APIs (f.i. [FXCM](fxcm.htm)) also require Windows 7 or above. For installing Zorro under Linux or OS X, see below.
  * You can install Zorro in any folder with full read/write access. Don't use a path with blanks or special characters, such as cyrillic, chinese, accents, or diacriticals. Zorro itself has no problem with that, but external modules and some broker APIs have. By default, Zorro is installed in your user directory (**C:\Users\YourName\Zorro**). But if **YourName** contains non-letter characters, such as blanks, dots, or foreign language characters, install it elsewhere. **C:\Zorro** will always work.
  * It is not recommended to install Zorro in a restricted access directory, such as the **Program Files** or **Programs(x86)** directory. If you still want to do it, switch off the Windows **UAC** (User Access Control). Otherwise the **UAC** will create 'shadow copies' of your strategy scripts in the **ProgramData** folder, and redirects file access to the shadow copy. This can be confusing - not only for scripts, but also for users . 
  * Zorro runs fine from a USB stick. You'll need about 40 MB for a plain installation with no historical data files.
  * Zorro is portable and needs no installer. You can directly copy a Zorro installation from your PC to a VPS or to a different folder.
  * For savng hard disk space, you can compress the **History** folder (right click / Properties / Advanced / Compress content). When you work on strategies, this folder will likely contain many huge historical data files. Compression saves disk space.

### Zorro installation folders

  * **Main folder:** Zorro executables and DLLs.
  * **Strategy:** Scripts (**.c** , **.cpp** , **.h**) and compiled scripts (**.x** , **.dll**).
  * **include:** Common include files for the scripts.
  * **History:** Historical data (.t1, .t2, .t6, .t8) and asset lists (.csv).
  * **Data:** Parameters, rules, machine learning models
  * **Cache:** Temporary files
  * **Log:** Log files (.txt) and charts (.png).
  * **Source:** Templates and plugin sources.
  * **Plugin:** 32-bit Broker, exchange, and data plugins.
  * **Zorro64:** Zorro 64-bit executable and DLLs.
  * **Plugin64:** 64-bit Broker and data source plugins.

### Zorro on old computers

For operating systems prior to Windows 8, or when you'll get a **'Dll not found'** or similar error message on Zorro start, you may need to install the **VC++ 2022 32-bit redistributable** package from Microsoft. You can download it either from Microsoft or from [our server](http://opserver.de/down/VC_redist.x86.exe). It will unpack and install itself.

### Zorro 64-bit and C++

The Zorro installation includes a 32 bit version (**Zorro.exe**) and a 64 bit version (**Zorro64\Zorro64.exe**). The 32-bit Zorro is the everyday version for all normal tasks, and runs even on old PCs with Windows XP. **Zorro64** is about 15% faster and can access all physical memory on the PC, which offers advantages for backtesting large asset portfolios with high resolution historical data. Since most broker plugins are 32 bit, the 32 bit Zorro is normally used for live trading. Unless you're high frequency trading, speed and memory footprint matter only for backtests and optimizations.

The 32-bit Zorro version needs no further software, but for **Zorro64** you need a 64-bit system with the **VC++ 2022 64-bit redistributable** package installed. You can download it either from Microsoft or from [our server](http://opserver.de/down/VC_redist.x64.exe). It will unpack and install itself.

While the 32-bit Zorro has its own on-the-fly C compiler ([lite-C](tutorial_var.htm)), the 64-bit Zorro needs the free [Visual C++](dlls.htm) community edition for running C++ scripts. If you have Zorro S, enter the path to the VC++ compiler in [Zorro.ini](ini.htm). C++ files will then compile on the fly and start directly. You need not specifically learn C++, since it's (mostly) backwards compatible to C. The free Zorro version supports C++ also, but then you'll need to create a VC++ project for any .cpp script and manually compile it to a DLL.

###  Zorro on Linux or Mac

Although Zorro is a Windows program, it can also run under Linux and OS X with [Wine](https://www.winehq.org) or [ VMWare](https://customerconnect.vmware.com/en/downloads/details?downloadGroup=WKST-PLAYER-1624&productId=1039). You can find threads on the [ Zorro forum](https://opserver.de/ubb7/ubbthreads.php?ubb=cfrm&c=1) where Ubuntu and Mac installations are discussed. The basic steps for Wine:

  * Install 32-bit Wine from [ https://wiki.winehq.org/Download](https://wiki.winehq.org/Download). The installation steps for Ubuntu/Mint are described [ here](https://wiki.winehq.org/Ubuntu).
  * Install Zorro from [ https://zorro-project.com/download.php](https://zorro-project.com/download.php). By default it's installed in the virtual Windows drive under **\\\users\\\YourName\\\Zorro**. 
  * Alternatively to installing, copy a Zorro installation from the Windows disk system or a USB drive directly into a suited Linux directory.

For VMWare, install the [ VMWare Player](https://customerconnect.vmware.com/en/downloads/details?downloadGroup=WKST-PLAYER-1624&productId=1039&) and the [ Windows Virtual Machine](https://developer.microsoft.com/en-us/windows/downloads/virtual-machines/), then install Zorro within the VM. The Windows VM also contains [Visual Studio](dlls.htm) for supporting C++ scripts.

### Zorro on 4k TVs

Windows programs appear 'zoomed' on some very hi-res displays, in order to prevent a too small GUI. For disabling this, adjust the DPI (Dots Per Inch) scaling settings on your operating system. This is a common issue when applications do not scale properly on high-resolution displays. The general steps:

  * Right-click on the Zorro Application Shortcut on your desktop or in the folder where it is installed.
  * Select 'Properties'. Go to the 'Compatibility' tab.
  * Click on 'Change high DPI settings'. Check the box that says 'Override high DPI scaling behavior'.
  * Choose 'Application' from the drop-down menu under 'Scaling performed by'. This setting will instruct Windows to let Zorro handle its own DPI scaling, rather than the operating system scaling it up
  * Click 'Apply' and then 'OK'.
  * Restart Zorro to see if the changes take effect.

### Zorro in the cloud

Zorro can run on a Windows Server and can be [controlled](command.htm) by PHP scripts. In this way it can be used for live trading, but also for web based applications or for advisory services.

Virtual private servers (VPS) are offered by many commercial providers for a small fee, like $20 per month. You'll normally get a fast and reliable online connection to the broker or exchange, and can access Zorro via Internet from anywhere. It's recommended to select a large and reliable company, like 1&1, Google, Microsoft, or Amazon, for hosting a live trading VPS. Amazon offers its EC2™ servers even free during the first year. Installation with a broker or MT4 connection on a VPS is not totally easy to a beginner, so we're offering a **VPS installation service** on the [support page](https://zorro-project.com/docs.php). If you want to do it yourself, here are the basic steps for setting up an Amazon VPS:

  * Order a cloud server instance with Windows Server 2016 or above. You'll usually receive a server IP, a login name (normally "Administrator"), and a password from your provider. With Amazon you need a key pair for decrypting the password. It is stored in a **.pem** file.  
  

  * Launch the server instance from your provider's website. Wait a minute or so until it's up and running. On the Amazon console, select your instance and click [Connect]. Normally you connect with a RDP client. Amazon will displays the DNS and the user name (normally "Administrator"). For decrypting the password, navigate to the **.pem** file that you've stored in the previous step.  

  * Under Windows, start use the Remote Desktop Connection (under **Accessories**). Enter DNS, user name, and password. A window will open where you can see the VPS desktop. It normally looks like a very empty Windows desktop. Alternatively to the Windows Remote Desktop Connection, you can install TeamViewer™ on the VPS - it's free for private use and more convenient to handle, especially for uploading or downloading files. The Windows Remote Desktop, as well as TeamViewer are also available for iPhone and Android, so you can control your VPS from anywhere.   

  * You can now install Zorro on the VPS. We recommend not to run the installation program, but to copy the Zorro folder directly - it's a lot faster. Start the Windows Explorer both on your PC and on the VPS. On your PC, right click on the **Zorro** folder, and select **Copy**. On the VPS, open a folder on which you have full access rights - we recommend **Users\Administrator** \- and right click on an empty space. Select **Paste**. Your Zorro folder will now be copied over to the VPS. Make sure that the folder names are plain letters with no blanks, dots, or foreign language characters - see above. If you don't want to do backtests on the server and don't use the **PRELOAD** flag, historical data files (*.t1, *.t6, *.t8) need not be copied, which makes the process a lot faster. You also need no source code on the server. If you have compiled your scripts to **.x** or **.dll** files, copy only them.  

  * You can now start Zorro on the VPS, select your broker, enter your user name and password, and click [Trade]. Dependent on number of CPU cores and amount of RAM on your VPS, you can trade with several Zorro instances simultaneously. Note that backtesting large portfolio strategies - such as Z12 - will not work on a low-end VPS with little RAM. Live trading uses minimal memory and will normally always work. 

It is recommended to disable automatic Windows updates for preventing reboots that interrupt the trading session. Make also sure that the time and the time zone of the VPS is set up correctly. It does not matter which time zone - you can either use your PC's time zone, or the VPS time zone, or any other time zone - but the VPS time must match the zone. If it is wrong, Zorro will display a wrong UTC time in the server window, and strategies based on market open and close times - such as gap trading - won't work anymore. If in doubt, set the VPS to the same time and time zone of your PC at home.

###  Additional software

Zorro can utilize functions and libraries from several other free software packages: [R](rbridge.htm), [Python](python.htm), and [Visual Studio C++](dlls.htm). If you want to code in C++ or if you need additional statistical or machine learning functions, install the packages and enter their paths in [ZorroFix.ini](ini.htm). When your trading algorithm uses R or Python, it must be also installed on the trading server. Visual Studio is not needed for deployment, but you might need to install the redistributable package, **vc_redist.x86.exe** or **vc_redist.x64.exe**. They are freely available from Microsoft.

### Multiple Zorros

If one Zorro is good, many Zorros are even better. The free Zorro version can trade with one instance, one broker, and one account only. [Zorro S](restrictions.htm) allows to run many Zorro instances simultaneously with different scripts (for details about setting up multiple broker connections, see [Account List](account.htm)). How many Zorro instances can run at the same time depends on the broker API, the Internet bandwidth, and the PC speed and memory resources. As an example, a Amazon EC2 Micro Instance can support about 4 Zorro instances with a direct broker connection, or up to 2 instances with a MT4 connection. 

If the parallel trading Zorros use different broker APIs, set the [NOLOCK](mode.htm) flag for speeding up API access. Otherwise a synchronization mechanism lets any Zorro wait with accessing the broker API until other Zorros have finished their API access. 

When running multiple Zorros with the same script, make sure that they don't write into the same logs or temporary files. The [Script](script.htm) string and/or the [LogNumber](numtotalcycles.htm) variable can be used to let any Zorro generate different file names. For communication between different Zorro instances, see [Interprocess Communication](engine.htm).

### Unlocking Zorro S 

[Zorro S](restrictions.htm) licenses come either with a key file (for permanent licenses) or with a 20-digit token (for subscriptions). If you have a key, unzip it in the Zorro folder - that's all. If you have a token, enter it in [Zorro.ini](ini.htm). Permanent licenses include 2 years Zorro updates. For updating Zorro further after 2 years, the period can be [extended](https://zorro-project.com/download.php). Private traders can install Zorro S on multiple PCs or servers that are under their exclusive control. Corporations need one license per PC or server. 

### Disable live trading

If you let your apprentice develop trading strategies, you can cripple his Zorro so that live trading is disabled. For this, open the **Plugin** folder and remove all **.dll** files inside. Zorro can then still run scripts and backtests and has access to online data sources, but cannot connect to a broker or exchange.

### Surviving reboots

Rebooting a live trading server should be avoided. But if it still happens, the trading session can be automatically resumed when the server restarts. Here's the procedure - you'll need [Zorro S](restrictions.htm) since the free version does not fully support command lines:

  * Create a file **startup.bat** in the Zorro folder. Here's an example that starts Global Prime MT4, waits 25 seconds, then starts a Zorro trading session:

```c
START /D "C:\Program Files(x86)\Global Prime - MetaTrader 4\Terminal.exe"
TIMEOUT 25
START Zorro.exe -trade MyStrategy -stay
```

  * Right click on startup.bat and select Create Shortcut.
  * When the shortcut has been created, right-click it and select Cut.
  * Hit Windows-R to open the Run window.
  * In the Run window, type **shell:startup** to open the Startup folder. 
  * When the Startup folder has been opened, click the Home tab at the top of the folder and select Paste to paste the shortcut into the folder.

The **startup.bat** file will then be executed at any reboot. Depending on the Windows version, the Startup folder can be different - on older versions it's named **Autostart** and you can edit it with **msconfig**. On Windows Server, open Administrative Tools / Task Scheduler and enter the task with the properties "Start a program" / "When the computer starts". Please note that some server configurations are reported to start Zorro in minimized mode with no GUI when no user is logged in. In this case, use "At Log on" rather than "At startup", specify the user account in the task properties, and use the Microsoft Sysinternals Autologon program to establish automatic logon at start.

### Fallback server

A server breakdown due to a hardware failure can be fatal for some trading systems. In such a case, a fallback server should immediately take over the trading when the original server becomes unresponsive. This is the suggested setup of such a configuration:

  * The fallback server must contain a Zorro installation with a twin of the trading system and all necessary files. It should be accessible from the trading server either through an IP address or as a mapped network drive.
  * The trading system continuously counts up a 'heartbeat' variable and stores it on the fallback server with a [putvar](https://zorro-project.com/manual/en/putvar.htm) call. This variable serves as signal that the trading server is still running.
  * At any relevant change of the trading status, i.e. opening or closing a position, or change of an essential trade or algo variable, the trading system saves its status on the fallback server with a [saveStatus](https://zorro-project.com/manual/en/loadstatus.htm) call.
  * The system on the fallback server checks the heartbeat variable continuously for detecting a problem with the original server. If the heartbeat variable stops counting, the fallback system loads the trading status with a [loadStatus](https://zorro-project.com/manual/en/loadstatus.htm) call, sends an [email](https://zorro-project.com/manual/en/email.htm) to notify the user about the problem, and takes over the trading.
  * The fallback server can also send a similar heartbeat variable to the trading server. In this way the trading system can detect a breakdown of the fallback server, and notify the user by email about the problem.
  * When trading server and fallback server are not physically located on the same network, heartbeat and trading status can be exchanged via [ftp_upload](https://zorro-project.com/manual/en/ftpx.htm) or [http_transfer](https://zorro-project.com/manual/en/http.htm) commands.

### Time-triggered scripts

If you want to start a session at a specific time or date, such as every Monday at 15:30 pm, you can do that with the Zorro command line and the Windows Task Scheduler. Here's the procedure - you'll need [Zorro S](restrictions.htm) since the free version does not fully support command lines:

  * Open the Task Scheduler (on German Windows versions: Aufgabenplanung).
  * Under Action, open a new task. Use a weekly (or daily, or monthly) trigger with the desired start time.
  * For the task, select Start Program. In the program window, enter the Zorro task, for instance:

```c
C:\Users\MyName\Zorro\Zorro.exe -trade MyScript
```

  * Control the task's properties, then save and finish. It should now appear in the scheduler's task list.

### Publishing the trading status

It is recommended that you regularly observe Zorro's trade list, profit status, and messages on your PC or smartphone. For this purpose, Zorro displays the trade status and the message window permanently on a website that's updated once per minute. The tricky part is to make that page visible on the Internet. Here's the instruction for Amazon EC2 with Windows Server 2012 / 2016 (similar for Google Cloud instances):

  * Open your VPS desktop with Windows Remote Desktop or TeamViewer, and use the Server Manager to add a new role. From the selection of roles, select "**Web Server (IIS)** ". Windows will walk you through the setup - the default settings will do - and install the server.  
  

  * In the Amazon EC2 dashboard, select the "**security group** " of your instance and add a new inbound rule. Select "HTTP" (TCP protocol, port 80). This will make your website visible to the public. On a Google Cloud instance, make sure that HTTP and HTTPS traffic is enabled. When you now enter the public IP address of your server in a browser, you should see the IIS logo.  
  

  * In the **zorro.ini** file, edit **WebFolder** and set it to the web folder of your server. The default web folder of a Windows server is normally **C:\inetpub\wwwroot**.  
  

  * You can now visit Zorro's [trade status page](trading.htm#status) through the public IP address of your server - for instance, by entering **http:\\\123.456.78.90\Z12.htm** in your browser's address field. Note that anyone else who knows this address can observe your trade success! So you might want to add an authentication role to the IIS web server, or hide the status page in a cryptic subdirectory.

### Providing trade signals

Some trade copy services such as **ZuluTrade™** offer a free VPS for their signal providers. The VPS is already configured, completely with MT4, but it's a little tricky to copy Zorro onto it. Here's the procedure for ZuluTrade ([Zorro S](restrictions.htm) required):

  * Open a MT4 demo account with a broker of your choice (f.i. AAAFx, Zulutrade's own broker). When starting MT4, you'll get a form for adding an account; write down the login number and the password that you'll see in the MTR4 welcome message. You can deinstall MT4 afterwards, as you only needed the login data. The MT4 demo account will not expire unless the broker goes broke or the account is inactive for 30 days.  

  * Register as a trader on ZuluTrade. On the MT4 link settings, give the login number and password from the previous step. Zulutrade will now link your trader account to the MT4 version. If that was successful, you'll get a button under your [Settings] tab for connecting to your VPS. Note that sometimes you have to wait a couple of weeks until a free VPS slot is available and the button appears.  

  * Click the [Connect] button to open the VPS. The VPS desktop will appear in your browser. Compress a copy of your Zorro folder to a single zip archive; strip it before from files not needed for trading, such as the content of the **Log** folder, or the price data files in the **History** folder. A stripped down Zorro archive has only a size of about 10 MB. Upload the zip file to the VPS. With Zulutrade, you can do this with the upload manager: click on the half-transparent round icon at the top of the VPS desktop, then click [Select Files] and navigate to the Zorro zip file on your PC.  

  * Once the archive is uploaded, open the folder on the VPS that receives uploaded files - on the ZuluTrade VPS it's the **ZuluHDD** folder. You can now open the archive and drag the Zorro folder inside over to the MTR4 Experts folder. Zorro can run from anywhere, so it's no problem to have it in a MTR4 subfolder.   

  * Now install the [MT4 bridge](mt4plugin.htm) on the VPS. You can now start MT4 and Zorro on the VPS, select your strategy, click [Trade] and begin generating trade signals.

When you provide signals, please play fair in the interest of your followers. Do not use martingale or similar methods, even though this might attract more followers at first. In the end, only the few signal providers that survive more than one or two months will earn long-term trust and profits.

### See also:

[Brokers](brokers.htm), [Getting Started](started.htm), [engine API](engine.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
