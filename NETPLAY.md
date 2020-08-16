This document describes the network protocol used for netplay in Retro-Go.

Caution: This document is very much a work in progress that describes the currently untested netplay code. Design flaws and bugs are likely going to change the following procedures as they are discovered.

Espressif's documentation suggests a low variation of the main esp32 oscillator (up to 240Mhz +/- .01%). In practice I've measured much higher variation between devices but it is still below 1%.For the time being we can safely assume that all devices run at the same speed.

The network stack will need to be tuned somewhat to reduce latency further, but in a 2-player setting it should be adequate for the time being.


# Connection process

- Host starts a wifi access point (at the moment the SSID isn't hidden, to help with development).
- Guests connect to host's access point.
- Upon connection the guest will receive a NETPLAY_PACKET_INFO from the host.
- The guest can now decide if the protocol and game ID match his or abandon the connection.
- Once the host determines that all players are connected (at the moment only 1), it broadcasts a NETPLAY_PACKET_READY that contains the list of players and instruct guests to zero reset their emulators. Zero reset means that we don't fill the memory with trash, instead we use a known value so that all players start with the exact same state.
- Finally, the host starts its own emulation which broadcasts the first NETPLAY_PACKET_SYNC_REQ to all guests and move to the next section.


# Emulation synchronization NES/SMS

- odroid_netplay_sync() is called immediately after reading the input (odroid_input_read_gamepad) in the emulation loop.
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

It will likely be the similar as above but, instead of odroid_gamepad_state_t, serial registers will be exchanged through odroid_netplay_sync(). Though at the moment Game Gear is very low priority and was never requested.


# State exchange

Description of how save states loading/saving will work in a netplay environment.


# ROM exchange

Description of how netplay will upload a ROM to a peer's RAM or SD Card in order to netplay.


# Forced synchronization

Description of how one player may trigger a forced sync that will cause all other players to mirror his current state. That would be a last resort in case emulation became out of sync.
