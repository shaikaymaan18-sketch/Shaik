# Hosting a Multiplayer Room

Use this guide for when you want to host a multiplayer lobby to play with others in Eden.  In order to have someone access the room from outside your local network, see [*Access Your Multiplayer Room Externally*](./MultiplayerExternalAccess.md)

**Click [Here](https://evilperson1337.notion.site/Hosting-a-Multiplayer-Room-2c357c2edaf6819481dbe8a99926cea2) for a version of this guide with images & visual elements.**

---

### Pre-Requisites

- Eden set up and Functioning
- Network Access
- Ability to allow programs through the firewall on your device.

---

## Steps

1. Open Eden and navigate to *Emulation → Multiplayer → Create Room.*    
2. Fill out the following information in the popup dialog box.
    
    
    | **Option** | **Acceptable Values** | **Default** | **Description** |
    | --- | --- | --- | --- |
    | Room Name | *Any string between 4 - 20 characters.* | *None* | Controls the name of the room and how it would appear in the Public Game Lobby/Room screen. |
    | Username | *Any string between 4 - 20 characters.* | *None* | Controls the name your user will appear as to the other users in the room. |
    | Preferred Game | *Any Game from your Game List.* | The first game on your game list | What game will the lobby be playing?  You are not forced to play the game you choose and can switch games without needing to recreate the room. |
    | Password | *None or any string* | *None* | What password do you want to secure the room with, if any. |
    | Max Players | 2 - 16 | 8 | How many players do you want to allow in the room at a time? |
    | Port | 1024 - 65535 | 24872 | What port do you want to run the lobby on?  Could technically be any port number, but it’s best to choose an uncommon port to avoid potential conflicts.  See [*Well-Known Ports*](https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers#Well-known_ports) for more information on ports commonly used. |
    | Room Description | *None or any string* | *None* | An optional message that elaborates on what the room is for, or for a makeshift message of the day presented to users in the lobby. |
    | Load Previous Ban List | [Checked, Unchecked] | Checked | Tells Eden to load the list containing users you have banned before. |
    | Room Type | [Public, Unlisted] | Public | Specifies whether you want the server to appear in the public game lobby browser |
3. Click **Host Room** to start the room server.  You may get a notice to allow the program through the firewall from your operating system.   Allow it and then users can attempt to connect to your room.