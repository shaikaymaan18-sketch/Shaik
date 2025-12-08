# Access Your Multiplayer Room Externally

Quite often the person with whom you want to play is located off of your internal network (LAN).  If you want to host a room and play with them you will need to get your devices to communicate with each other.  This guide will go over your options on how to do this so that you can play together.

**Click [Here](https://evilperson1337.notion.site/Access-Your-Multiplayer-Room-Externally-2c357c2edaf681c0ab2ce2ee624d809d) for a version of this guide with images & visual elements.**

---

### Pre-Requisites

- Eden set up and Functioning
- Network Access

---

## Options

### Port Forwarding

- **Difficulty Level**: High
    
    <aside>
    
    Use this option if you want the greatest performance/lowest latency, don’t want to install any software, and have the ability to modify your networking equipment’s configuration (most notably - your router).  Avoid this option if you cannot modify your router’s configuration or are uncomfortable with looking up things on your own.
    
    </aside>
    

Port forwarding is a networking technique that directs incoming traffic arriving at a specific port on a router or firewall to a designated device and port inside a private local network. When an external client contacts the public IP address of the router on that port, the router rewrites the packet’s destination information (IP address and sometimes port number) and forwards it to the internal host that is listening on the corresponding service. This allows services such as web servers, game servers, or remote desktop sessions hosted behind NAT (Network Address Translation) to be reachable from the wider Internet despite the devices themselves having non‑routable private addresses.

The process works by creating a static mapping—often called a “port‑forward rule”—in the router’s configuration. The rule specifies three pieces of data: the external (public) port, the internal (private) IP address of the target machine, and the internal port on which that machine expects the traffic. When a packet arrives, the router checks its NAT table, matches the external port to a rule, and then translates the packet’s destination to the internal address before sending it onward. Responses from the internal host are similarly rewritten so they appear to come from the router’s public IP, completing the bidirectional communication loop. This mechanism enables seamless access to services inside a protected LAN without exposing the entire network. 

For our purposes we would pick the port we want to expose (*e.g. 24872*) and we would access our router’s configuration and create a port-forward rule to send the traffic from an external connection to your local machine over our specified port (*24872)*.  The exact way to do so, varies greatly by router manufacturer - and sometimes require contacting your ISP to do so depending on your agreement.  You can look up your router on [*portforward.com](https://portforward.com/router.htm)* which may have instructions on how to do so for your specific equipment.  If it is not there, you will have to use Google/ChatGPT to determine the steps for your equipment.

---

### Use a Tunnelling Service

- **Difficulty Level**: Easy
    
    <aside>
    
    Use this option if you don’t want to have to worry about other users machine/configuration settings, but also cannot do port forwarding.  This will still require that you as the hoster install a program and sign up for an account - but will prevent you from having to deal with port forwards or networking equipment. Avoid this option if there is not a close relay and you are getting issues with latency.
    
    </aside>
    

Using a Tunnelling service may be the solution to avoid port forward, but also avoid worrying about your users setup. A tunnelling service works by having a lightweight client run on the machine that hosts the game server. That client immediately opens an **outbound** encrypted connection (typically over TLS/QUIC) to a relay node operated by the tunnel provider's cloud infrastructure. Because outbound traffic is almost always allowed through NAT routers and ISP firewalls, the tunnel can be established even when the host sits behind carrier‑grade NAT or a strict firewall. The tunnel provider then assigns a public address (e.g., `mygame.playit.gg:12345`). When a remote player connects to that address, the traffic reaches the the tunnel provider relay, which forwards it through the already‑established tunnel back to the client on the private network, and finally onto the local game server’s port. In effect, the server appears to the Internet as if it were listening on the public address, while the host never needs to configure port‑forwarding rules or expose its own IP directly.

For our purposes we would spawn the listener for the port that way chose when hosting our room.  The user would connect to our assigned public address/port combination, and it would be routed to our machine.  The tunnel must remain active for as long as you want the connection to remain open.  Closing the terminal will kill the tunnel and disconnect the users.

**Recommended Services:**

- [*Playit.GG*](https://playit.gg/)

---

### Use a VPN Service

- **Difficulty**: Easy

<aside>

Use this option if you don’t want to use a tunnelling service, or don’t want to manage the overhead of the playit gg solution, don’t want to send data through the relay, or any other reason.  Avoid this method if you do not want to have to manage VPN installation/configuration for your users.

</aside>

The VPN solution is a good compromise between the tunnelling solution and port forwarding.  You do not have to port forward or touch your networking equipment at all - but also don’t need to send all your data connections through a 3rd party relay.  The big downside is that you will have to ensure all of your users have your VPN solution installed *and* that they have a valid configuration.  When looking for a solution, it is advised to find one that uses the WireGuard protocol for speed, and does not require communication with a server beyond the initial handshake.

**Recommended Services:**

- [*Tailscale](https://tailscale.com/)*
- [*ZeroTier*](https://www.zerotier.com/)
- *Self-hosted VPN Solution*
    - This is so far out of the scope of this document it has a different postal code.

*Check with the provider you select on the sign up and installation process specific to that provider.*