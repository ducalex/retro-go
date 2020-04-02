This document describes the network protocol used for netplay in Retro-Go.

# Connection process
- Host starts a wifi access point (at the moment the SSID isn't hidden, to help with development).
- Guests connect to host's access point.
- After acquiring its IP, the guest broadcasts a NETPLAY_PACKET_INFO.
- All other players respond to the new player with a NETPLAY_PACKET_INFO.
- When a player receives a NETPLAY_PACKET_INFO, it compares the protocol version and game ID with his own. If the protocol version doesn't match then the connection fails. If the game ID doesn't match then a warning is shown but connection may continue.
- Once the host determines that all players are connected (at the moment only 1), it sends a NETPLAY_PACKET_GAME_RESET
  which causes players to reset the emulator.
- Finally, the host starts its own emulation which broadcasts the first NETPLAY_PACKET_SYNC_REQ to all guests and waits until everybody is ready.
