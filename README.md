# MT4 Manager
MT4 Manager is .NET wrapper for meta trader 4 (MT4) manager API built using C++/Cli.

# Copyright
MetaQuotes Software Corp. is the developer of the MetaTrader 4 online trading platform and MQL4/MQL5 software languages legal owner.

This solution was created to help users with .net experience use the latest API provide by MetaQuotes' Manager API. It was created using the same pattern provided in MT4ManagerAPI.h

# Structure

1) Definitions.h:
     * MetaQuotes: Wrapper over "MT4ManagerAPI.h"
     * Helpers: Includes Helper class used as a setter/getter for mails,new topics and plugin parameters.
     * Manager: Includes all data types defined in MT4ManagerAPI.h

2) MT4ManAPI.h:

    Includes methods and functions defined in MT4ManagerAPI.h
    
# Usage

The solution creates a class library (MT4Manager.dll) that can be included in .net projects.

# Simple Example
```
// Include namespace
using Manager;

// Create MeTatraderManager instance 
MeTatraderManager mt4=new MeTatraderManager();

// Use connect function similar to how it is used in MT4ManagerAPI.h
mt4.Connect("127.0.0.1:443");
mt4.Login(1, "manager");

// Use Extended Pumping Switch
mt4.PumpingSwitchEx(CBFunc,(int)PumpingFlags.CLIENT_FLAGS_HIDETICKS|(int)PumpingFlags.CLIENT_FLAGS_HIDEONLINE,0);
```
