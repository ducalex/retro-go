#ifndef _HUGOD_H
#define _HUGOD_H

/*
 * -- Client -> Server packets types :
 *
 * - Identification :
 *
 * Byte 0 : Identifier = 0x55
 * Byte 1 : Number of requested slots (1-5)
 * Byte 2-6 : Remote (client) input device number (unused entries are ignored)
 * Byte 7 : Checksum on bytes 0-6 (algorithm described later)
 *
 * - In-game device status ( == request for others status for this frame )
 * Byte 0 : Identifier = 0xC2
 * Byte 1-4 : Frame number (network endianess encoded)
 *   (*) NOTE : in internet protocol version, the frame number is used to indicate a request.
 *   If null, there's no special request done.
 * Byte 5-9 : Remote (client) input device status
 *      Only devices which were declared as used has to be filled, remaining is ignored
 * Byte 10 : Checksum on bytes 0-9 (algorithm described soon)
 *
 * -- Server -> Client packets types :
 *
 * - Identification acknowledge
 *
 * Byte 0 : Identifier = 0x96
 * Byte 1 : Number of slots really allocated
 * Byte 2 : Checksum on bytes 0-1 (algorithm described very soon)
 *
 * - Digest (LAN PROTOCOL)
 *
 * Byte 0 : Identifier = 0x3B
 * Byte 1-4 : Frame number (network endianess encoded)
 * Byte 5-9 : Digest of the input device status
 * Byte 10 : Checksum on bytes 0-9 (algorithm described extremely soon)
 *
 * - Digest (INTERNET PROTOCOL)
 *
 * Byte 0 : Identifier = 0x1F
 * Byte 1-4 : Frame number of the first digest sent (network endianess encoded)
 * Byte 5 : Number of digest sent (>=1)
 * Byte 6-10 : 1st digest of the input device status
 * (Byte 11-15  : 2nd digest if any)
 * (...)
 * Last byte (6 + 5 * number of digest sent) : Checksum on previous bytes (algorithm described in an almost past future)
 *
 * -- Now, it's time to ... hmmm... let's see. Oh, yeah, checksum algorithm.
 *
 * It's mean to be quick and easy, not a real error checking
 *
 * byte checksum = 0;
 * for each byte to check, do checksum ^= current_byte; rotate left checksum
 * that's all folks
 */

#include <SDL_net.h>
#include "config.h"
#include "utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*!
   * Structure used to distinguish a remote (client) device. It uses the ip endpoint
   * designation and the number of the device on client's side
   */
  typedef struct
  {
    IPaddress address;
    char remote_input_device;
  } input_mapping_type;

  typedef enum { LAN_PROTOCOL_TYPE, INTERNET_PROTOCOL_TYPE } type_server_type;

  typedef struct
  {
    char number_player;
    int server_port;
    type_server_type type_server;
  } global_option_type;

  typedef struct
  {
    IPaddress address;
    char allocated_slots;
  } allocation_request_type;

  typedef enum
  { UNIDENTIFIED, WAITING, READY } client_status_type;

  typedef struct
  {
    uint32 frame_number;
    int number_identified_players;
    input_mapping_type input_mapping[5];	//!< 5 stands for MAX_NUMBER_PLAYER
    uchar input_value[5];                       //!< 5 stands for MAX_NUMBER_PLAYER
    client_status_type player_status[5];	//!< 5 stands for MAX_NUMBER_PLAYER
    UDPsocket server_socket;
    UDPpacket *current_packet;	//!< current packet computed from client
    UDPpacket *digest_packet;	//!< summary packet to send to clients
    SDLNet_SocketSet server_socket_set;	//!< A singleton set used to check activity passively
    int number_allocation_request;
    allocation_request_type allocation_request[5];  //!< 5 stands for MAX_NUMBER_PLAYER
    double start_time; //!< In seconds
    uint32 next_frame_to_send[5];  //!< 5 stands for MAX_NUMBER_PLAYER
    uint32 next_frame_asked[5];   //!< 5 stands for MAX_NUMBER_PLAYER
  } global_status_type;

#ifdef __cplusplus
}
#endif

#endif				/* _HUGO_SERVER_H */
