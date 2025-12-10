# Finding the Server Information for a Multiplayer Room

Use this guide when you need to determine the connection information for the Public Multiplayer Lobby you are connected to.

---

### Pre-Requisites

- Eden set up and configured
- Internet Access

## Steps

### Method 1: Grabbing the Address from the Log File

1. Open Eden and Connect to the room you want to identify.
    1. See [*Joining a Multiplayer Room*](./MultiplayerJoiningGuide.md) for instructions on how to do so if you need them.
2. Go to *File → Open Eden Folder*, then open the **config** folder.
3. Open the the **qt-config.ini** file in a text editor.
4. Search for the following keys: 
    1. `Multiplayer\ip=` 
    2. `Multiplayer\port=`
5. Copy the Server Address and Port.

### Method 2: Using a Web Browser

1. Obtain the name of the room you want the information for.
2. Open a Web Browser.
3. Navigate to  [`https://api.ynet-fun.xyz/lobby`](https://api.ynet-fun.xyz/lobby)
4. Press *Ctrl + F* and search for the name of your room.
5. Look for and copy the Server Address and Port.

### Method 3: Using a Terminal (PowerShell or CURL)

1. Obtain the name of the room you want the information for.
2. Open the terminal supported by your operating system.
3. Run one of the following commands, replacing *<Name>* with the name of the server from step 1.
    
    ### PowerShell Command [Windows Users]
    
    ```powershell
    # Calls the API to get the address and port information
    (Invoke-RestMethod -Method Get -Uri "https://api.ynet-fun.xyz/lobby").rooms | Where-Object {$_.Name -eq '<NAME>'} | Select address,port | ConvertTo-Json
    
    # Example Output
    #{
    #  "address": "118.208.233.90",
    #  "port": 5001
    #}
    ```
    
    ### CURL Command [MacOS/Linux Users] **Requires jq*
    
    ```bash
    # Calls the API to get the address and port information
    curl -s "https://api.ynet-fun.xyz/lobby" | jq '.rooms[] | select(.name == "<NAME>") | {address, port}'
    
    # Example Output
    #{
    #  "address": "118.208.233.90",
    #  "port": 5001
    #}
    ```
    
4. Copy the Server Address and Port.