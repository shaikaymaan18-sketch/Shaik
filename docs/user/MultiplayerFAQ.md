# Multiplayer FAQ

This FAQ will serve as a general quick question and answer simple questions.

**Click [Here](https://evilperson1337.notion.site/Multiplayer-FAQ-2c357c2edaf680fca2e9ce59969a220f) for a version of this guide with images & visual elements.**

---

## Can Eden Play Games with a Switch Console?

No - The only emulator that has this kind of functionality is *Ryujinx* and it’s forks.  This solution requires loading a custom module on a modded switch console to work.

## Can I Play Online Games?

No - This would require hijacking requests to Nintendo’s official servers to a custom server infrastructure built to emulate that functionality.  This is how services like [*Pretendo*](https://pretendo.network/) operate.  As such, you would not be able to play “Online”, you can however play multiplayer games.

## What’s the Difference Between Online and Multiplayer?

I have chosen the wording carefully here for a reason.  

- Online: Games that connect to Nintendo’s Official servers and allow games/functionality using that communication method are unsupported on any emulator currently (for obvious reasons).
- Multiplayer:  The ability to play with multiple players on separate systems.  This is supported for games that support LDN Local Wireless.

The rule of thumb here is simple: If a game supports the ability to communicate without a server (Local Wireless, LAN, etc.) you will be able to play with other users.  If it requires a server to function - it will not.  You will need to look up if your title support Local Wireless/LAN play as an option.

## How Does Multiplayer Work on Eden Exactly?

Eden’s multiplayer works by emulating the Switch’s local wireless (LDN) system, then tunneling that traffic over the internet through “rooms” that act like lobbies. Each player runs their own instance of the emulator, and as long as everyone joins the same room and the game supports local wireless multiplayer, the emulated consoles see each other as if they were on the same local network. This design avoids typical one‑save netplay issues because every user keeps an independent save and console state while only the in‑game wireless packets are forwarded through the room server.  In practice, you pick or host a room, configure your network interface/port forwarding if needed, then launch any LDN‑capable game; from the game’s perspective it is just doing standard local wireless, while the emulator handles discovery and communication over the internet or LAN.

## What Do I Need to Do?

That depends entirely on what your goal is and your level of technical ability, you have a 2 options on how to proceed.  

1. Join a Public Lobby.
    1. If you just want to play *Mario Kart 8 Deluxe* with other people and don’t care how it works or about latency - this is the option for you. See [*Joining a Multiplayer Room*](./MultiplayerJoiningGuide.md) for instructions on how to do this.
2. Host Your Own Room.
    1. This option will require you to be comfortable with accessing your router’s configuration, altering firewall rules, and troubleshooting when things (inevitably) don’t work out perfectly on the first try.  Use this option if you want to control the room entirely, are concerned about latency issues, or just want to run something for your friends. See [*Hosting a Multiplayer Room*](./MultiplayerHostingGuide.md) for next steps*.*

## Can Other Platforms Play Together?

Yes - the platform you choose to run the emulator on does not matter.  Steam Deck users can play with Windows users, Android users can play with MacOS users, etc.  Furthermore different emulators can play together as well (Eden/Citron/Ryubing, etc.) - but be you may want to all go to the same one if you are having issues.

## What Pitfalls Should I Look Out For?

While it would be nice if everything always worked perfectly - that is not reality.  Here are some things you should watch out for when attempting to play multiplayer.

1. Emulator Version Mismatches
    1. Occasionally updates to the emulator of choice alter how the LDN functionality is handled.  In these situations, unexpected behavior can occur when trying to establish LDN connections.  This is a good first step to check if you are having issues playing a game together, but can join the same lobby without issue.
2. Game Version Mismatches
    1. It is best practice to have the game version be identical to each other in order to ensure that there is no difference in how the programs are handling the LDN logic.  Games are black boxes that the dev team cannot see into to ensure the logic handling operates the same way.  For this reason, it is highly advised that the game versions match across all the players.  This would be a good 2nd step to check if you are having issues playing a game together, but can join the same lobby without issue.
3. Latency
    1. Because this implementation is emulating a LAN/Local Wireless connection - it is extremely sensitive to network latency and drops.  Eden has done a good job of trying to account for this and not immediately drop users out - but it is not infallible.  If latency is a concern or becomes an issue - consider hosting a room.