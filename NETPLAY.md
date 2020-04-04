This document describes the network protocol used for netplay in Retro-Go.

Caution: This document is very much a work in progress that describes the currently untested netplay code. Design flaws and bugs are likely going to change the below procedures as they are discovered.

Espressif's documentation suggests a very low variation of the main esp32 clock (up to .1%). I did measure a bigger variation than that, but it is still below 1%. For all intents and purposes we can assume all devices run at the same speed.

The network stack will need to be tuned somewhat to reduce latency further, but in a 2-player setting it should be adequate for the time being.


# Connection process

- Host starts a wifi access point (at the moment the SSID isn't hidden, to help with development).
- Guests connect to host's access point.
- Once connected, the guest broadcasts their NETPLAY_PACKET_INFO.
- All other players respond to the new player with their own NETPLAY_PACKET_INFO.
- When a player receives a NETPLAY_PACKET_INFO from the *host*, it compares the protocol version and game ID with his own. If the protocol version doesn't match then the connection fails. If the game ID doesn't match then a warning is shown but connection may continue.
- Once the host determines that all players are connected (at the moment only 1), it sends a NETPLAY_PACKET_GAME_RESET
  which causes all players to zero reset the emulator. Zero reset means that we don't fill the memory with trash, instead we use a known value so that all players start with the exact same state.
- Finally, the host starts its own emulation which broadcasts the first NETPLAY_PACKET_SYNC_REQ to all guests and waits until everybody is ready.


# Emulation synchronization NES/SMS

- odroid_netplay_sync() is called immediately after reading the input (odroid_input_gamepad_read) in the emulation loop.
- If the player is the host:
  - The player broadcasts a NETPLAY_PACKET_SYNC_REQ (with data field set to its joystick).
  - It waits to receive a NETPLAY_PACKET_SYNC_ACK from each player.
  - It finally broadcasts a NETPLAY_PACKET_SYNC_DONE indicating emulation can resume.
  - odroid_netplay_sync() returns.
- If the player is a guest:
  - The player waits for a NETPLAY_PACKET_SYNC_REQ.
  - The player broadcasts a NETPLAY_PACKET_SYNC_ACK (with data field set to its joystick).
  - The player waits for a NETPLAY_PACKET_SYNC_DONE indicating emulation can resume.
  - odroid_netplay_sync() returns.
- The player emulates one frame.


# Emulation synchronization Game Boy/Game Gear

It will likely be the similar as above but instead of odroid_gamepad_state, serial registers will be exchanged through odroid_netplay_sync(). Though at the moment Game Gear is very low priority as it was never requested.


# State exchange

Description of how save states loading/saving will work in a netplay environment.


# ROM exchange

Description of how netplay will upload a ROM with a peer's RAM or SD Card in order to netplay.


# Forced synchronization

Description of how one player may trigger a forced sync that will cause all other players to mirror his current state. That would be a last resort in case emulation became out of sync.
