/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _NETPLAY_H
#define _NETPLAY_H

#include "huexpressd.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern const int PACKET_IDENTIFIER;

  extern const int PACKET_IDENTIFICATION_ID;
  extern const int PACKET_IDENTIFICATION_ACKNOWLEDGE_ID;
  extern const int PACKET_STATUS_ID;
  extern const int PACKET_DIGEST_ID;
  extern const int PACKET_INTERNET_DIGEST_ID;

  extern const int PACKET_IDENTIFICATION_NUMBER_REQUESTED_SLOTS;
  extern const int PACKET_IDENTIFICATION_INPUT_DEVICE_INDEX;
  extern const int PACKET_IDENTIFICATION_INPUT_DEVICE_NUMBER;
  extern const int PACKET_IDENTIFICATION_CHECKSUM;
  extern const int PACKET_IDENTIFICATION_LENGTH;

  extern const int PACKET_IDENTIFICATION_ACKNOWLEDGE_NUMBER_ALLOCATED_SLOTS;
  extern const int PACKET_IDENTIFICATION_ACKNOWLEDGE_CHECKSUM;
  extern const int PACKET_IDENTIFICATION_ACKNOWLEDGE_LENGTH;

  extern const int PACKET_STATUS_FRAME_NUMBER;
  extern const int PACKET_STATUS_INPUT_DEVICE_INDEX;
  extern const int PACKET_STATUS_INPUT_DEVICE_NUMBER;
  extern const int PACKET_STATUS_CHECKSUM;
  extern const int PACKET_STATUS_LENGTH;

  extern const int PACKET_INTERNET_DIGEST_FRAME_NUMBER;
  extern const int PACKET_INTERNET_DIGEST_NUMBER_DIGEST;
  extern const int PACKET_INTERNET_DIGEST_DIGEST_INDEX;
  extern const int PACKET_INTERNET_DIGEST_DIGEST_NUMBER;
  extern const int PACKET_INTERNET_DIGEST_BASE_LENGTH;
  extern const int PACKET_INTERNET_DIGEST_INCREMENT_LENGTH;

  extern const int MAX_NUMBER_PLAYER;

  extern const int MIN_NUMBER_PLAYER;

  extern const int SERVER_SOCKET_TIMEOUT;
  extern const int CLIENT_SOCKET_TIMEOUT;
  extern const int CLIENT_SOCKET_INTERNET_TIMEOUT;


  extern const int CLIENT_PACKET_SIZE;
  extern const int SERVER_PACKET_SIZE;

  extern const int DEFAULT_SERVER_PORT;

  extern const int DIGEST_SIZE;

  //! Initialize the network
  /*!
   * @return 0 in case of success
   */
  int init_network ();

  //! Releases ressources allocated for the network communication
  void shutdown_network ();

  //! Compute the checksum for an array of byte
  uchar compute_checksum (uchar *data, int index_min,
				  int index_max);

  //! Return the number of available slots
  int count_remaining_slot (global_status_type * global_status,
			    global_option_type * global_option);

  //! Send a packet to acknowledge identification
  void send_identification_acknowledge_packet (global_status_type *
					       global_status,
					       int number_allocated_slots);

  //! Compute the packet as if it was an identification one and eventually send a reply
  void identify_client (global_status_type * global_status,
			global_option_type * global_option);

  //! Stores an incoming packet for local processing
  int read_incoming_server_packet (global_status_type * global_status);

  //! Handles client requests using the accurate but greedy way
  void serve_clients_lan_protocol (global_status_type * global_status,
				   global_option_type * global_option);

  //! Handles client request using the inaccrute but fast way
  void serve_clients_internet_protocol (global_status_type * global_status,
					global_option_type * global_option);

#ifdef __cplusplus
}
#endif

#endif				/* _NETPLAY_H */
