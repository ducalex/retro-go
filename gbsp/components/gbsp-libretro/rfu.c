/* gameplaySP
 *
 * Copyright (C) 2023 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"

// Debug print logic:
#ifdef RFU_DEBUG
  #define RFU_DEBUG_LOG(...) printf(__VA_ARGS__)
#else
  #define RFU_DEBUG_LOG(...)
#endif

// Config knobs, update with care.
#define BCST_ANNOUNCE_VB               30   // Send broadcast twice per second.
#define MAX_RFU_PEERS  MAX_RFU_NETPLAYERS   // Do not allow more than 32 peers.

#define RFU_DEF_TIMEOUT              32   // Expressed as frames (~533ms)
#define RFU_DEF_RTXMAX                4   // Up to 4 transmissions per send


// Unpacks big endian integers used for signaling
static inline u32 upack32(const u8 *ptr) {
  return ptr[3] | (ptr[2] << 8) | (ptr[1] << 16) | (ptr[0] << 24);
}

// Unpacks payload data, which is little-endian
static inline u32 leupack32(const u8 *ptr) {
  return ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
}

// The following commands, names and bit fields are not a 100% known.
// Most of them is guessed via reverse engineering the adapter and/or
// games that use it. Please take it with a grain of salt.
// You might wanna check:
// https://github.com/afska/gba-link-connection/
// https://blog.kuiper.dev/gba-wireless-adapter

#define RFU_CONN_INPROGRESS  0x01000000     // Connection ongoing
#define RFU_CONN_FAILED      0x02000000     // Connection failed

#define RFU_CONN_COMP_FAIL   0x01000000     // Failed to connect

#define RFU_CMD_INIT1        0x10   // Dummy after-init command
#define RFU_CMD_INIT2        0x3d   // Dummy after-init command
#define RFU_CMD_SYSCFG       0x17   // System configuration (comms. config)

#define RFU_CMD_LINKPWR      0x11   // Link/RF strength (0 to 0x16/0xFF)

// These are not really well documented.
#define RFU_CMD_SYSVER       0x12   // Return some 1 word with version info.
#define RFU_CMD_SYSSTAT      0x13   // System/Connection status.
#define RFU_CMD_SLOTSTAT     0x14   // Reads slot status information.
#define RFU_CMD_CFGSTAT      0x15   // Reads some sort of config state.

#define RFU_CMD_BCST_DATA    0x16   // Broadcast data setup

#define RFU_CMD_HOST_START   0x19   // Start broadcasting and accepting clients
#define RFU_CMD_HOST_ACCEPT  0x1a   // Poll for incoming connections
#define RFU_CMD_HOST_STOP    0x1b   // Stop accepting new connections

#define RFU_CMD_BCRD_START   0x1c   // Broadcast read session start
#define RFU_CMD_BCRD_FETCH   0x1d   // Broadcast read (actual data read)
#define RFU_CMD_BCRD_STOP    0x1e   // Broadcast read session end

#define RFU_CMD_CONNECT      0x1f   // Connect to host
#define RFU_CMD_ISCONNECTED  0x20   // Check for conection status
#define RFU_CMD_CONCOMPL     0x21   // Complete connection?

#define RFU_CMD_SEND_DATA    0x24   // Sends a data packet
#define RFU_CMD_SEND_DATAW   0x25   // Sends a data packet and waits
#define RFU_CMD_RECV_DATA    0x26   // Receive (poll) some data
#define RFU_CMD_WAIT         0x27   // Wait (for response or timeout)
#define RFU_CMD_RTX_WAIT     0x37   // Wait after some retransmit?

#define RFU_CMD_DISCONNECT   0x30   // Disconnects clients

// RFU commands for slave mode (~command responses)
#define RFU_CMD_RESP_TIMEO   0x27   // Timeout!
#define RFU_CMD_RESP_DATA    0x28   // There's data available
#define RFU_CMD_RESP_DISC    0x29   // Some clients disconnected


// These are internal FSM states for the communication steps.
#define RFU_COMSTATE_RESET       0    // Just out of reset
#define RFU_COMSTATE_HANDSHAKE   1    // Performing initial nintendo handshake
#define RFU_COMSTATE_WAITCMD     2    // Idle, waiting for a command
#define RFU_COMSTATE_WAITDAT     3    // Waiting for follow up data
#define RFU_COMSTATE_RESPCMD     4    // RFU to device response (cmd)
#define RFU_COMSTATE_RESPDAT     5    // RFU to device response (N words)
#define RFU_COMSTATE_RESPERR     6    // Send back the error command
#define RFU_COMSTATE_RESPERR2    7    // Send back the error code
#define RFU_COMSTATE_WAITEVENT   8    // Waiting for event or timeout
#define RFU_COMSTATE_WAITRESP    9    // Responding a wait command

// These FSM states describe the adapter wifi state.
#define RFU_STATE_IDLE            0
#define RFU_STATE_HOST            1    // Hosting (with broadcast)
#define RFU_STATE_CONNECTING      2    // Trying to connect to a parent
#define RFU_STATE_CLIENT          3    // Client, connected to a host


static u32 rfu_prev_data;
static u32 rfu_comstate, rfu_cnt, rfu_state;
static u32 rfu_buf[255];
static u8 rfu_cmd, rfu_plen;
static u32 rfu_timeout_cycles, rfu_resp_timeout;
static u8 rfu_timeout, rfu_rtx_max;

static struct {
  u32 buf[23];
  u8 blen;
} rfu_tx_buf;

static struct {
  u16 devid;         // Device ID assigned to the host
  u8 tx_ttl;         // Internal counter for broadcast transmission
  u32 bdata[6];      // Data to broadcast other devices
  struct {
    u16 client_id;   // libretro client ID
    u16 devid;       // Host-assigned device ID
    u16 clttl;       // Client TTL (to check for disconnects)
    struct {
      u32 datalen;   // Byte count of data waiting to be polled.
      u8  data[16];  // Data received from client.
    } pkts[4];
  } clients[4];      // Connected clients IDs (zero means empty slot).
} rfu_host;

static struct {
  u16 devid;         // Device ID assigned to the client (by the host?)
  u16 clnum;         // Client number (0 to 3)
  u16 host_id;       // Client ID for the host device.
  // Store host recevied packets (up to 8!)
  struct {
    u16 hblen;       // Bytes received from the host.
    u8 hdata[128];   // Data received from the host (accumulated).
  } pkts[4];
} rfu_client;

typedef struct {
  u8 valid;
  u8 ttl;            // When it reaches zero, entry is invalidated
  u16 device_id;     // Random ID generated by the RFU for each new session
  u32 data[6];       // Broadcast data (game+user data)
} t_client_broadcast;

// The table is indexed by client_id
static t_client_broadcast rfu_peer_bcst[MAX_RFU_PEERS];

// Constants used for the network protocol.

#define NET_RFU_HEADER          0x52465531

#define NET_RFU_BROADCAST       0x00    // Host to everyone broadcast packet
#define NET_RFU_CONNECT_REQ     0x01    // Client connection request
#define NET_RFU_CONNECT_ACK     0x02    // Host connection response (accept)
#define NET_RFU_CONNECT_NACK    0x03    // Host connection response (reject)
#define NET_RFU_DISCONNECT      0x04    // Client/Host disconnect notice
#define NET_RFU_HOST_SEND       0x05    // Host to client data send
#define NET_RFU_CLIENT_SEND     0x06    // Client to host data send
#define NET_RFU_CLIENT_ACK      0x07    // Client ACKs host received data.

// Callbacks used to send and force-receive data.
void netpacket_send(uint16_t client_id, const void *buf, size_t len);
void netpacket_poll_receive();

static void rfu_net_send_cmd(int client_id, u32 ptype, u32 h) {
  u32 pkt[4] = {
    netorder32(NET_RFU_HEADER),  // RFU1 header
    netorder32(ptype),           // Message type
    netorder32(h),               // Header word
    0
  };
  netpacket_send(client_id, pkt, 16);
}

static void rfu_net_send_bcast(u32 ptype, u32 h, const u32 *pload) {
  u32 i;
  u32 pkt[9] = {
    netorder32(NET_RFU_HEADER),   // RFU1 header
    netorder32(ptype),            // Message type
    netorder32(h),                // Header word
  };
  for (i = 0; i < 6; i++)
    pkt[i + 3] = netorder32(pload[i]);

  // Broadcast to all clients (ID=0xffff)
  netpacket_send(0xffff, pkt, sizeof(pkt));
}

static void rfu_net_send_data(int client_id, u32 ptype, u32 h, const u32 *pload, unsigned plen) {
  u32 i;
  struct {
    u32 header[3];
    u8  data8[92];
  } pkt = {
    {
      netorder32(NET_RFU_HEADER),   // RFU1 header
      netorder32(ptype),            // Message type
      netorder32(h),                // Header word
    },
  };
  // Data is sent in little endian format over the RF link (presumably)
  for (i = 0; i < plen; i++)
    pkt.data8[i] = pload[i / 4] >> (8 * (i & 3));
  memset(&pkt.data8[plen], 0, 92 - plen);

  netpacket_send(client_id, &pkt, 104);
}

// This is called whenever the game uses the GPIO (pin D) to perform a reset
// pin flip in the external reset pin. We reset the device to a known state.
void rfu_reset() {
  RFU_DEBUG_LOG("RFU reset!\n");
  // Reset FSMs to a known state
  rfu_prev_data = 0;
  rfu_cnt = 0;
  rfu_state = RFU_STATE_IDLE;
  rfu_comstate = RFU_COMSTATE_RESET;
  rfu_timeout_cycles = 0;
  rfu_resp_timeout = 0;
  rfu_timeout = RFU_DEF_TIMEOUT;
  rfu_rtx_max = RFU_DEF_RTXMAX;
  memset(&rfu_host, 0, sizeof(rfu_host));

  // Clear all the received broadcasts.
  memset(&rfu_peer_bcst, 0, sizeof(rfu_peer_bcst));

  // Re-seed random gen.
  rand_seed(time(NULL));
  rand_seed(cpu_ticks);
}

static u16 new_devid() {
  while (1) {
    u16 n = rand_gen() ^ time(NULL);
    if (n)
      return n;
  }
}

// We have received a command in full (with a potential payload), process it
// and return the return command code (plus some payload too?).
static s32 rfu_process_command() {
  u32 i, j, cnt = 0;
  RFU_DEBUG_LOG("Processing command 0x%x (len %d)\n", rfu_cmd, rfu_plen);

  switch (rfu_cmd) {
  // These are not 100% supported, but they are OK as long as we ACK them.
  case RFU_CMD_INIT1:
  case RFU_CMD_INIT2:
    return 0;

  case RFU_CMD_SYSCFG:
    // Contains the slave timeout and retransmit count.
    rfu_timeout = rfu_buf[0];
    rfu_rtx_max = rfu_buf[0] >> 8;
    return 0;

  case RFU_CMD_SYSVER:
    rfu_buf[0] = 0x00830117;   // Likely some sort of firmware/hw version.
    return 1;

  case RFU_CMD_SYSSTAT:
    // Lower bits contain the DEVID, along with slot bits (if any) and some status code.
    if (rfu_state == RFU_STATE_HOST)
      rfu_buf[0] = (1 << 24) | rfu_host.devid;
    else if (rfu_state == RFU_STATE_CLIENT)
      rfu_buf[0] = (5 << 24) | ((1 << rfu_client.clnum) << 16) | rfu_client.devid;
    else
      rfu_buf[0] = 0;

    return 1;

  case RFU_CMD_SLOTSTAT:
    if (rfu_state == RFU_STATE_HOST) {
      // Just a list of connected devices it seems
      u32 cnt = 0;
      rfu_buf[cnt++] = 0;
      for (i = 0; i < 4; i++)
        if (rfu_host.clients[i].devid) {
          rfu_buf[0]++;
          rfu_buf[cnt++] = rfu_host.clients[i].devid | (i << 16);
        }
      return cnt;
    }
    return 0;

  case RFU_CMD_LINKPWR:
    // TODO: Return something better (ie. latency?)
    if (rfu_state == RFU_STATE_HOST)
      rfu_buf[0] = (rfu_host.clients[0].devid ? 0x000000ff : 0) |
                   (rfu_host.clients[1].devid ? 0x0000ff00 : 0) |
                   (rfu_host.clients[2].devid ? 0x00ff0000 : 0) |
                   (rfu_host.clients[3].devid ? 0xff000000 : 0);
    else if (rfu_state == RFU_STATE_CLIENT)
      rfu_buf[0] = 0xffffffff;
    else
      rfu_buf[0] = 0;
    return 1;

  // Process broadcast read sessions.
  // TODO return errors if outside of a session.
  case RFU_CMD_BCRD_START:
    return 0;   // Return an ACK immediately.

  case RFU_CMD_BCRD_STOP:
  case RFU_CMD_BCRD_FETCH:
    // We pick up to four random broadcasting peers.
    // This is not randomly fair but whatever :D
    i = rand_gen() % MAX_RFU_PEERS;
    for (j = 0, cnt = 0; cnt < 4*7 && j < MAX_RFU_PEERS; j++) {
      u32 entry = (i + j) % MAX_RFU_PEERS;
      if (rfu_peer_bcst[entry].valid) {
        // Header is just the device ID
        rfu_buf[cnt++] = rfu_peer_bcst[entry].device_id;
        memcpy(&rfu_buf[cnt], rfu_peer_bcst[entry].data,
               sizeof(rfu_peer_bcst[entry].data));
        cnt += 6;
      }
    }
    return cnt;

  // Sets the broadcast data to use on Host mode
  case RFU_CMD_BCST_DATA:
    if (rfu_plen == 6)
      memcpy(rfu_host.bdata, rfu_buf, sizeof(rfu_host.bdata));
    return 0;

  case RFU_CMD_HOST_START:
    if (rfu_state == RFU_STATE_CLIENT) {
      RFU_DEBUG_LOG("RFU error: Cannot start host, we are a client already\n");
      return -1;   // Return error if we are connected (state client)
    }

    if (rfu_state == RFU_STATE_IDLE) {
      // Generate a new ID if we don't have one already.
      rfu_host.devid = new_devid();
      memset(rfu_host.clients, 0, sizeof(rfu_host.clients));
      rfu_state = RFU_STATE_HOST;    // Host mode active
      RFU_DEBUG_LOG("Start hosting with device ID %02x\n", rfu_host.devid);
    }
    // Start broadcasting immediately.
    rfu_host.tx_ttl = 0xff;
    return 0;

  case RFU_CMD_HOST_STOP:
    if (rfu_state == RFU_STATE_IDLE)
      return -1;  // Return error if host mode is not active

    // This just "stops" accepting new clients, however if the host has no
    // clients, it will return to idle state (I think!).
    if (rfu_state == RFU_STATE_HOST) {
      for (i = 0; i < 4; i++)
        if (rfu_host.clients[i].devid)
          return 0;     // We stay in the host mode.

      rfu_state = RFU_STATE_IDLE;
    }

    return 0;

  case RFU_CMD_HOST_ACCEPT:
    if (rfu_state == RFU_STATE_IDLE)
      return -1;  // Return error if host mode is not active

    // This is actually a "list connected devices", not actually accept().
    // Return a list of IDs and a client number (slot number).
    for (i = 0; i < 4; i++)
      if (rfu_host.clients[i].devid)
        rfu_buf[cnt++] = rfu_host.clients[i].devid | (i << 16);
    return cnt;

  case RFU_CMD_CONNECT:
    if (rfu_state == RFU_STATE_HOST)
      return -1;  // Return error if host mode is active

    // The game specified a device ID, find the host.
    {
      u16 reqid = rfu_buf[0] & 0xffff;
      for (i = 0, cnt = 0; i < MAX_RFU_PEERS; i++) {
        if (rfu_peer_bcst[i].valid &&
            rfu_peer_bcst[i].device_id == reqid) {

          // Send a request to the host to connect
          rfu_net_send_cmd(i, NET_RFU_CONNECT_REQ, reqid);
          rfu_state = RFU_STATE_CONNECTING;
          RFU_DEBUG_LOG("Requesting connection to client %d (%x)\n", i, reqid);
          return 0;
        }
      }
    }
    // If the ID cannot be found, just ACK for now, ISCONNECTED will fail.
    return 0;

  case RFU_CMD_ISCONNECTED:
    if (rfu_state == RFU_STATE_HOST)
      return -1;  // Return error if host mode is active

    if (rfu_state == RFU_STATE_CONNECTING)
      rfu_buf[0] = RFU_CONN_INPROGRESS;
    else if (rfu_state == RFU_STATE_IDLE)
      rfu_buf[0] = RFU_CONN_FAILED;
    else
      rfu_buf[0] = rfu_client.devid | (rfu_client.clnum << 16);

    return 1;

  case RFU_CMD_CONCOMPL:
    // Seems that this is also called even when no connection happened!
    if (rfu_state == RFU_STATE_HOST)
      return -1;

    // Returns ID, with slot number and can also indicate failure.
    if (rfu_state == RFU_STATE_CLIENT) {
      rfu_buf[0] = rfu_client.devid | (rfu_client.clnum << 16);
    } else {
      rfu_buf[0] = RFU_CONN_COMP_FAIL;
      rfu_state = RFU_STATE_IDLE;
    }

    return 1;

  case RFU_CMD_SEND_DATAW:
  case RFU_CMD_SEND_DATA:
    if (!rfu_plen)
      return 0;

    if (rfu_state == RFU_STATE_HOST) {
      // Read data to be sent into the TX buffer
      rfu_tx_buf.blen = rfu_buf[0] & 0x7F;
      memcpy(rfu_tx_buf.buf, &rfu_buf[1], (rfu_plen - 1)*sizeof(u32));
    }
    else if (rfu_state == RFU_STATE_CLIENT) {
      // Same as above, but the header encoding is funny
      rfu_tx_buf.blen = (rfu_buf[0] >> (8 + rfu_client.clnum * 5)) & 0x1F;
      memcpy(rfu_tx_buf.buf, &rfu_buf[1], (rfu_plen - 1)*sizeof(u32));
    }

    /* fallthrough */
  case RFU_CMD_RTX_WAIT:
    if (rfu_state == RFU_STATE_HOST) {
      // Host sends a package to all clients.
      RFU_DEBUG_LOG("Host sending %d bytes / %d words to clients\n",
                    rfu_tx_buf.blen, rfu_plen - 1);
      if (rfu_tx_buf.blen <= 90) {
        for (i = 0; i < 4; i++)
          if (rfu_host.clients[i].devid)
            rfu_net_send_data(rfu_host.clients[i].client_id,
              NET_RFU_HOST_SEND, rfu_tx_buf.blen, rfu_tx_buf.buf, rfu_tx_buf.blen);
      }
    }
    else if (rfu_state == RFU_STATE_CLIENT) {
      // Schedule data to be sent
      RFU_DEBUG_LOG("Client sending %d bytes / %d words to host\n",
                    rfu_tx_buf.blen, rfu_plen - 1);
      if (rfu_tx_buf.blen <= 16) {
        // Send it immediately! This is not really accurate.
        rfu_net_send_data(rfu_client.host_id, NET_RFU_CLIENT_SEND,
          (rfu_tx_buf.blen << 24) | (rfu_client.clnum << 16) | rfu_client.devid,
          rfu_tx_buf.buf, rfu_tx_buf.blen);
      }
    }
    else
      return -1;   // We are not connected nor a host
    break;

  case RFU_CMD_RECV_DATA:
    if (rfu_state == RFU_STATE_HOST) {
      // Receive data from clients
      u32 cnt = 0, bufbytes = 0;
      u8 tmpbuf[16*4] = {0};
      rfu_buf[cnt++] = 0;   // Header contains byte counts as a bitfield.
      for (i = 0; i < 4; i++) {
        u32 dlen = MIN(16, rfu_host.clients[i].pkts[0].datalen);
        if (rfu_host.clients[i].devid && dlen != 0) {
          RFU_DEBUG_LOG("Host reads data from client buffer (%d bytes)\n", dlen);
          // Accumulate into temp buffer
          memcpy(&tmpbuf[bufbytes], &rfu_host.clients[i].pkts[0].data[0], dlen);
          bufbytes += dlen;
          // Update byte count header for this client
          rfu_buf[0] |= dlen << (8 + i * 5);
          // Discard front packet
          memmove(&rfu_host.clients[i].pkts[0], &rfu_host.clients[i].pkts[1],
                  3 * sizeof(rfu_host.clients[i].pkts[0]));
          rfu_host.clients[i].pkts[3].datalen = 0;
        }
      }
      // Copy data into words into the RFU buffer.
      for (i = 0; i < (bufbytes + 3) / 4; i++)
        rfu_buf[cnt++] = leupack32(&tmpbuf[i*4]);
      return cnt;
    }
    else if (rfu_state == RFU_STATE_CLIENT) {
      u32 cnt = 0;
      u32 dlen = rfu_client.pkts[0].hblen;
      RFU_DEBUG_LOG("Client reads data from host buffer (%d bytes)\n", dlen);
      rfu_buf[cnt++] = dlen;   // Header contains byte count.
      for (j = 0; j < (dlen + 3) / 4; j++)
        rfu_buf[cnt++] = leupack32(&rfu_client.pkts[0].hdata[j*4]);

      // Move to the next packet
      memmove(&rfu_client.pkts[0], &rfu_client.pkts[1], sizeof(rfu_client.pkts[0]) * 3);
      rfu_client.pkts[3].hblen = 0;
      return cnt;
    }
    break;

  case RFU_CMD_WAIT:
    // Do nothing, return no data (just ack).
    // The handler will deal with clock reversing.
    return 0;

  case RFU_CMD_DISCONNECT:
    if (rfu_state == RFU_STATE_CLIENT) {
      // Assuming self-disconnect?
      rfu_net_send_cmd(rfu_client.host_id, NET_RFU_DISCONNECT,
                       rfu_client.devid | (rfu_client.clnum << 16));
      rfu_state = RFU_STATE_IDLE;
    } else if (rfu_state == RFU_STATE_HOST) {
      // We are disconnecing some client(s).
      for (i = 0; i < 4; i++)
        if (rfu_buf[0] & (1 << i)) {
          // Send disconnect notice, clear slot!
          rfu_net_send_cmd(rfu_host.clients[i].client_id, NET_RFU_DISCONNECT,
                   rfu_host.clients[i].devid | (i << 16));
          memset(&rfu_host.clients[i], 0, sizeof(rfu_host.clients[i]));
        }
    }
    return 0;

  default:
    RFU_DEBUG_LOG("Unknown RFU command %02x\n", rfu_cmd);
  };

  return 0;
}

// Returns true if a Wait event can finish due to new data being available.
static bool rfu_data_avail() {
  if (rfu_state == RFU_STATE_CLIENT) {
    if (rfu_client.pkts[0].hblen != 0)
      return true;
  }
  else if (rfu_state == RFU_STATE_HOST) {
    // Returns true if any data from any client is available.
    u32 i;
    for (i = 0; i < 4; i++)
      if (rfu_host.clients[i].devid && rfu_host.clients[i].pkts[0].datalen)
        return true;
  }
  return false;
}

// Called whenever the game flips the SIOCNT:Start bit to transmit a value.
// We simulate the reception and response of said value (over the SPI bus).
u32 rfu_transfer(u32 sent_value) {
  u32 retval = 0x80000000;

  switch (rfu_comstate) {
  case RFU_COMSTATE_RESET:
    // Start of sequence (check the low 16 bits)
    retval = 0;
    if ((sent_value & 0xFFFF) == 0x494E)
      rfu_comstate = RFU_COMSTATE_HANDSHAKE;

    break;

  case RFU_COMSTATE_HANDSHAKE:
    // Check for the last step of the sequence
    if (sent_value == 0xB0BB8001)
      rfu_comstate = RFU_COMSTATE_WAITCMD;

    retval = (sent_value << 16) | ((~rfu_prev_data) & 0xFFFF);
    break;

  case RFU_COMSTATE_WAITCMD:
    // Wait for a new command, verify its header.
    if ((sent_value >> 16) == 0x9966) {
      rfu_plen = (u8)(sent_value >> 8);
      rfu_cmd  = (u8)(sent_value);
      rfu_cnt = 0;
      if (!rfu_plen) {
        // Returns error code or response length
        s32 ret = rfu_process_command();
        if (ret < 0) {
          rfu_comstate = RFU_COMSTATE_RESPERR;
          rfu_cmd = (u32)(-ret); // Err code
          rfu_plen = 1;
        }
        else {
          rfu_comstate = RFU_COMSTATE_RESPCMD;
          rfu_plen = (u32)ret;
        }
      }
      else
        rfu_comstate = RFU_COMSTATE_WAITDAT;
    }
    break;

  case RFU_COMSTATE_WAITDAT:
    rfu_buf[rfu_cnt++] = sent_value;
    if (rfu_cnt == rfu_plen) {
      s32 ret = rfu_process_command();
      if (ret < 0) {
        rfu_comstate = RFU_COMSTATE_RESPERR;
        rfu_cmd = (u32)(-ret); // Err code
        rfu_plen = 1;
      }
      else {
        rfu_comstate = RFU_COMSTATE_RESPCMD;
        rfu_plen = (u32)ret;
      }
      rfu_cnt = 0;
    }
    break;

  case RFU_COMSTATE_RESPCMD:
    // Disregard the input value and respond with the ack'ed command
    retval = 0x99660080 | rfu_cmd | (rfu_plen << 8);

    // These commands are special: they do not have any response data
    // and reverse the clock roles (the RFU becomes master).
    if (rfu_cmd == RFU_CMD_WAIT || rfu_cmd == RFU_CMD_RTX_WAIT || rfu_cmd == RFU_CMD_SEND_DATAW) {
      rfu_comstate = RFU_COMSTATE_WAITEVENT;
      rfu_timeout_cycles = rfu_timeout * (16777216 / 60);  // Frames to cycles
      rfu_resp_timeout = rfu_rtx_max * (16777216 / 60 / 6);  // RFU "frame" to cycles
    } else {
      rfu_comstate = rfu_plen ? RFU_COMSTATE_RESPDAT : RFU_COMSTATE_WAITCMD;
    }
    break;

  case RFU_COMSTATE_RESPDAT:
    // Keep pushing data to the GBA
    retval = rfu_buf[rfu_cnt++];
    if (rfu_cnt == rfu_plen)
      rfu_comstate = RFU_COMSTATE_WAITCMD;
    break;

  case RFU_COMSTATE_WAITEVENT:
  case RFU_COMSTATE_WAITRESP:
    // TODO: this should not happen? Since we are master?
    break;

  case RFU_COMSTATE_RESPERR:
    // Return the error code command. Includes one error value
    retval = 0x996601ee;
    rfu_comstate = RFU_COMSTATE_RESPERR2;
    break;

  case RFU_COMSTATE_RESPERR2:
    retval = rfu_cmd;   // Some error code, not understood yet.
    rfu_comstate = RFU_COMSTATE_WAITCMD;
    break;
  };

  rfu_prev_data = sent_value;
  return retval;
}

// Gets called every frame for basic device updates.
void rfu_frame_update() {
  // If device is in reset state, do nothing really.
  if (rfu_comstate != RFU_COMSTATE_RESET) {
    u32 i;
    // Account for peer expiration.
    for (i = 0; i < MAX_RFU_PEERS; i++) {
      if (rfu_peer_bcst[i].valid) {
        if (--rfu_peer_bcst[i].ttl == 0)
          rfu_peer_bcst[i].valid = 0;
      }
    }

    // Broadcast host session periodically
    if (rfu_state == RFU_STATE_HOST) {
      if (rfu_host.tx_ttl++ >= BCST_ANNOUNCE_VB) {
        rfu_host.tx_ttl = 0;
        rfu_net_send_bcast(NET_RFU_BROADCAST, rfu_host.devid, rfu_host.bdata);
      }
    }

    // Account client TTL (to timeout clients)
    if (rfu_state == RFU_STATE_HOST) {
      for (i = 0; i < 4; i++) {
        if (rfu_host.clients[i].devid) {
          if (++rfu_host.clients[i].clttl >= 240 /* 4s */) {
            // The client hasn't sent stuff for a while, disconnect.
            RFU_DEBUG_LOG("Disconnect client slot %d due to timeout\n", i);
            memset(&rfu_host.clients[i], 0, sizeof(rfu_host.clients[i]));
          }
        }
      }
    }
  }
}

void rfu_net_receive(const void* buf, size_t len, uint16_t client_id) {
  // RFU1 header, just some minor sanity check really.
  if (len >= 12 && upack32((const u8*)buf) == NET_RFU_HEADER) {
    u32 i, j;
    const u8 *buf8 = (const u8*)buf;
    const u8 *payl = &buf8[12];
    u32 ptype = upack32(&buf8[4]);
    u32 hdata = upack32(&buf8[8]);

    switch (ptype) {
    case NET_RFU_BROADCAST:
      // Fill the broadcast slot for the peer
      RFU_DEBUG_LOG("Got broadcast (client: #%d devID: %02x)\n", client_id, hdata);
      if (client_id < MAX_RFU_PEERS) {
        rfu_peer_bcst[client_id].device_id = hdata;
        rfu_peer_bcst[client_id].valid = 1;
        rfu_peer_bcst[client_id].ttl = 0xff;
        for (j = 0; j < 6; j++)
          rfu_peer_bcst[client_id].data[j] = upack32(&payl[j*4]);
      }
      break;

    case NET_RFU_CONNECT_REQ:
      RFU_DEBUG_LOG("Received Conn Req (client ID: %d)\n", client_id);
      if (rfu_state == RFU_STATE_HOST) {
        // Ensure this client is not already connected!
        for (i = 0; i < 4; i++)
          if (rfu_host.clients[i].devid &&
              rfu_host.clients[i].client_id == client_id) {

            RFU_DEBUG_LOG("Connection request ignored: already connected!\n");
            return;
          }

        // Find an empty connection slot
        for (i = 0; i < 4; i++)
          if (!rfu_host.clients[i].devid) {
            u16 newid = new_devid();
            rfu_host.clients[i].devid = newid;
            rfu_host.clients[i].client_id = client_id;
            RFU_DEBUG_LOG("Connected client: assigned new devID %02x\n", newid);

            // Respond with ACK
            rfu_net_send_cmd(client_id, NET_RFU_CONNECT_ACK, newid | (i << 16));
            return;
          }
        
        // Not enough slots, NACK
        rfu_net_send_cmd(client_id, NET_RFU_CONNECT_NACK, 0);
      } else {
        RFU_DEBUG_LOG("Conn Req ignored, device is not in Host mode\n");
        rfu_net_send_cmd(client_id, NET_RFU_CONNECT_NACK, 0);
      }
      break;
      
    case NET_RFU_CONNECT_ACK:
      RFU_DEBUG_LOG("Received connection ACK from client ID: %d\n", client_id);
      // Only ok if we are not connected (not hosting)
      if (rfu_state == RFU_STATE_CONNECTING) {
        // Clear state and install device ID and slot number.
        memset(&rfu_client, 0, sizeof(rfu_client));
        rfu_client.devid = hdata & 0xffff;
        rfu_client.clnum = hdata >> 16;
        rfu_client.host_id = client_id;
        rfu_state = RFU_STATE_CLIENT;
        RFU_DEBUG_LOG("Client connected with slot ID %d\n", rfu_client.clnum);
      }
      break;

    case NET_RFU_CONNECT_NACK:
      // When receiving a NACK just return to Idle state.
      if (rfu_state == RFU_STATE_CONNECTING)
        rfu_state = RFU_STATE_IDLE;
      RFU_DEBUG_LOG("Received CONN NACK\n");
      break;

    case NET_RFU_DISCONNECT:
      if (rfu_state == RFU_STATE_HOST) {
        // Clear the client from the list
        u32 clnum = (hdata >> 16) & 0x3;
        u16 cldid = hdata & 0xffff;
        if (rfu_host.clients[clnum].devid == cldid)
          memset(&rfu_host.clients[clnum], 0, sizeof(rfu_host.clients[clnum]));
      }
      else if (rfu_state == RFU_STATE_CLIENT) {
        // Go back to idle state!
        memset(&rfu_client, 0, sizeof(rfu_client));
        rfu_state = RFU_STATE_IDLE;
      }
      break;

    case NET_RFU_HOST_SEND:
      // Only possible if we are a client
      if (rfu_state == RFU_STATE_CLIENT) {
        u32 blen = hdata & 0x7f;
        if (len >= blen + 12) {
          u32 i;
          // ACK the reception (so they know we are alive!)
          rfu_net_send_cmd(client_id, NET_RFU_CLIENT_ACK,
                           rfu_client.devid | (rfu_client.clnum << 16));
          // Receive data from the host. Queue que packet if possible
          for (i = 0; i < 4; i++) {
            if (!rfu_client.pkts[i].hblen) {
              memcpy(&rfu_client.pkts[i].hdata, payl, blen);
              rfu_client.pkts[i].hblen = blen;
              RFU_DEBUG_LOG("Recv host packet (%d bytes) Q[#%d]\n", blen, i);
              return;
            }
          }
          RFU_DEBUG_LOG("Client dropped a host packet\n");
        }
      }
      break;

    case NET_RFU_CLIENT_SEND:
      // Only available when we are hosting
      if (rfu_state == RFU_STATE_HOST) {
        u32 i;
        u16 cdevid = hdata & 0xffff;
        u32 clid = (hdata >> 16) & 0x3;
        u32 blen = hdata >> 24;

        // Validate the slot with device ID
        if (rfu_host.clients[clid].devid == cdevid) {
          rfu_host.clients[clid].clttl = 0;   // Account for packet reception
          for (i = 0; i < 4; i++) {
            if (!rfu_host.clients[clid].pkts[i].datalen) {
              memcpy(rfu_host.clients[clid].pkts[i].data, payl, blen);
              rfu_host.clients[clid].pkts[i].datalen = blen;
              RFU_DEBUG_LOG("Recv client packet (%d bytes) Q[#%d]\n", blen, i);
              return;
            }
          }
          RFU_DEBUG_LOG("Host dropped a client packet\n");
        }
      }

    case NET_RFU_CLIENT_ACK:
      // Should only happen when hosting
      if (rfu_state == RFU_STATE_HOST) {
        u32 devid = hdata & 0xffff;
        u32 clid = (hdata >> 16) & 0x3;

        if (rfu_host.clients[clid].devid == devid)
          rfu_host.clients[clid].clttl = 0;   // Account for packet reception
      }
    };
  } else {
    RFU_DEBUG_LOG("Drop malformed packet\n");
  }
}

// Account for consumed cycles and return if a serial IRQ should be raised.
bool rfu_update(unsigned cycles) {
  if (rfu_comstate == RFU_COMSTATE_WAITEVENT) {
    // Force receive packets so that we can perhaps abort the wait
    // This helps minimize latency (othweise we need to wait a full frame!)
    netpacket_poll_receive();

    // Check if we are running our of time to respond.
    rfu_timeout_cycles -= MIN(cycles, rfu_timeout_cycles);
    rfu_resp_timeout   -= MIN(cycles, rfu_resp_timeout);

    // Wait for GBA to go into slave mode before finishing the wait!
    if ((read_ioreg(REG_SIOCNT) & 0x1) == 0) {
      if (rfu_state == RFU_STATE_IDLE) {
        // We are disconnected (most likely)
        rfu_buf[0] = 0x99660000 | (1 << 8) | RFU_CMD_RESP_DISC;
        rfu_buf[1] = 0xF;  // Reason disconnect (0), all slots disconnected?
        rfu_buf[2] = 0x80000000;
        rfu_cnt = 0;
        rfu_plen = 3;
        rfu_comstate = RFU_COMSTATE_WAITRESP;
        RFU_DEBUG_LOG("Wait command resp: disconnect\n");
      }
      else if (rfu_data_avail()) {
        // Some event is available!
        rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_DATA;
        rfu_buf[1] = 0x80000000;
        rfu_cnt = 0;
        rfu_plen = 2;
        rfu_comstate = RFU_COMSTATE_WAITRESP;
      }
      else if (rfu_state == RFU_STATE_HOST && !rfu_resp_timeout) {
        // We "retransmitted" the message N times (not really, but the equivalent
        // time has elapsed) and we simulate a lack of client response.
        rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_DATA | (1 << 8);
        rfu_buf[1] = 0x00000F0F;  // TODO: just using 4 slots for now
        rfu_buf[2] = 0x80000000;
        rfu_cnt = 0;
        rfu_plen = 3;
        rfu_comstate = RFU_COMSTATE_WAITRESP;
      }
      else if (!rfu_timeout_cycles) {
        // We ran out of time, just return an "error" code
        rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_TIMEO;
        rfu_buf[1] = 0x80000000;
        rfu_cnt = 0;
        rfu_plen = 2;
        rfu_comstate = RFU_COMSTATE_WAITRESP;
        RFU_DEBUG_LOG("Wait command resp: timeout\n");
      }
    }
  }

  if (rfu_comstate == RFU_COMSTATE_WAITRESP) {
    // Pushes command and data as an RFU-master back to the GBA.
    // Only if SO/SI are low and the device is active to receive stuff
    if ((read_ioreg(REG_SIOCNT) & 0xC) == 0 && (read_ioreg(REG_SIOCNT) & 0x80)) {
      // Write data into the "received" register and clear active bit.
      write_ioreg(REG_SIODATA32_H, rfu_buf[rfu_cnt] >> 16);
      write_ioreg(REG_SIODATA32_L, rfu_buf[rfu_cnt] & 0xffff);
      write_ioreg(REG_SIOCNT, (read_ioreg(REG_SIOCNT) & ~0x80));
      rfu_cnt++;
      // Go back to slave mode
      if (rfu_cnt == rfu_plen)
        rfu_comstate = RFU_COMSTATE_WAITCMD;
      return read_ioreg(REG_SIOCNT) & 0x4000;
    }
  }

  return false;
}

