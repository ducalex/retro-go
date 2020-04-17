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

#include "netplay.h"

#define DEBUG

const int PACKET_IDENTIFIER = 0;

const int PACKET_IDENTIFICATION_ID = 0x55;
const int PACKET_IDENTIFICATION_ACKNOWLEDGE_ID = 0x96;
const int PACKET_STATUS_ID = 0xC2;
const int PACKET_DIGEST_ID = 0x3B;
const int PACKET_INTERNET_DIGEST_ID = 0x1F;

const int PACKET_IDENTIFICATION_NUMBER_REQUESTED_SLOTS = 1;
const int PACKET_IDENTIFICATION_INPUT_DEVICE_INDEX = 2;
const int PACKET_IDENTIFICATION_INPUT_DEVICE_NUMBER = 5;
const int PACKET_IDENTIFICATION_CHECKSUM = 7;
const int PACKET_IDENTIFICATION_LENGTH = 8;

const int PACKET_IDENTIFICATION_ACKNOWLEDGE_NUMBER_ALLOCATED_SLOTS = 1;
const int PACKET_IDENTIFICATION_ACKNOWLEDGE_CHECKSUM = 2;
const int PACKET_IDENTIFICATION_ACKNOWLEDGE_LENGTH = 3;

const int PACKET_STATUS_FRAME_NUMBER = 1;
const int PACKET_STATUS_INPUT_DEVICE_INDEX = 5;
const int PACKET_STATUS_INPUT_DEVICE_NUMBER = 5;
const int PACKET_STATUS_CHECKSUM = 10;
const int PACKET_STATUS_LENGTH = 11;

const int PACKET_INTERNET_DIGEST_FRAME_NUMBER = 1;
const int PACKET_INTERNET_DIGEST_NUMBER_DIGEST = 5;
const int PACKET_INTERNET_DIGEST_DIGEST_INDEX = 6;
const int PACKET_INTERNET_DIGEST_DIGEST_NUMBER = 5;
const int PACKET_INTERNET_DIGEST_BASE_LENGTH = 7;
const int PACKET_INTERNET_DIGEST_INCREMENT_LENGTH = 5;

const int MAX_NUMBER_PLAYER = 5;

const int MIN_NUMBER_PLAYER = 1;

const int SERVER_SOCKET_TIMEOUT = 1000;	/* In millisecond */
const int CLIENT_SOCKET_TIMEOUT = 10;	/* In millisecond */
const int CLIENT_SOCKET_INTERNET_TIMEOUT = 20;	/* In millisecond */

const int CLIENT_PACKET_SIZE = 11;
const int SERVER_PACKET_SIZE = 7 + 5 * 5;

const int DEFAULT_SERVER_PORT = 25679;

const int FRAME_PER_SECOND = 60;
const int DIGEST_SIZE = 5;
const int BUFFER_SIZE_SECONDS = 60 * 2;	/* two minuts */
const int BUFFER_SIZE_BYTES = 60 * 2 * 60 * 5;	/* size in seconds * frame per seconds * digest size */
const int AUTO_OUTDATE = 60 * 2 * 60;	/* size in seconds * frame per seconds */

const double DOUBLE_FRAME_DURATION = 1.0 / 60.0;	/* In second */
const int SDL_TICKS_PER_SECOND = 1000;

/*!
 * In the lan protocol implementation, this array holds the input for the previous frame
 */
static uchar previous_input_value[5];

/*!
 * In the internet protocol implementation, this pointer holds the history */
static uchar *history_digest = NULL;

//! Send packet for the previous frame
void send_previous_digest_packet (global_status_type * global_status,
				  global_option_type * global_option,
				  IPaddress address);

/*!
 * Initializes the network for netplay. SDL has to be initialized first.
 */
int
init_network ()
{

  if (SDLNet_Init () == -1)
    {
      printf ("SDLNet_Init: Error\n");
      return 2;
    }

  return 0;
}

/*!
 * Release the network for netplay. It doesn't change the state of SDL
 */
void
shutdown_network ()
{
  SDLNet_Quit ();
}

/*!
 * Compute the rotation of the value (equivalent of ROL in ia32 asm)
 * \param value the value to rotate
 * \return the rotated value
 */
uchar
rotate_left (uchar value)
{
  return (uchar) ((value << 1) + ((value & 0x80) ? 1 : 0));
}

/*!
 * Compute the check sum value according to the algorithm found in the header file
 * \param data the array on which perform the computation
 * \param index_min first index in the array to take into account
 * \param index_max last index (EXCLUSIVE) in the array to take into account
 */
uchar
compute_checksum (uchar *data, int index_min, int index_max)
{
  uchar checksum;
  int index;

  checksum = 0;

  for (index = index_min; index < index_max; index++)
    {
      checksum ^= data[index];
      checksum = rotate_left (checksum);
    }

  return checksum;
}

/*!
 * Side effect free minimum computation
 */
static int
min (int lhs, int rhs)
{
  return (lhs < rhs) ? lhs : rhs;
}

/*!
 * Returns the number of free (unidentified and under the limit requested) slots
 */
int
count_remaining_slot (global_status_type * global_status,
		      global_option_type * global_option)
{
  return global_option->number_player -
    global_status->number_identified_players;
}

/*!
 * Check if two SDLNet addresses are the same
 * \return non null value is addresses match, else 0
 */
int
equals_address (IPaddress lhs, IPaddress rhs)
{
  return (lhs.host == rhs.host) && (lhs.port == rhs.port);
}

/*!
 * Build and send a packet to acknowledge an identification. We use the digest
 * packet structure to prepare it.
 */
void
send_identification_acknowledge_packet (global_status_type * global_status,
					int number_allocated_slots)
{
  int number_destination;

  /* Set the identifier */
  global_status->digest_packet->data[PACKET_IDENTIFIER] =
    PACKET_IDENTIFICATION_ACKNOWLEDGE_ID;

  /* Set the number of allocated slots */
  global_status->digest_packet->
    data[PACKET_IDENTIFICATION_ACKNOWLEDGE_NUMBER_ALLOCATED_SLOTS] =
    number_allocated_slots;

  /* Compute the checksum */
  global_status->digest_packet->
    data[PACKET_IDENTIFICATION_ACKNOWLEDGE_CHECKSUM] =
    compute_checksum (global_status->digest_packet->data, PACKET_IDENTIFIER,
		      PACKET_IDENTIFICATION_ACKNOWLEDGE_CHECKSUM);


  global_status->digest_packet->len =
    PACKET_IDENTIFICATION_ACKNOWLEDGE_LENGTH;

  /* Back to sender */
  global_status->digest_packet->address =
    global_status->current_packet->address;

  number_destination = SDLNet_UDP_Send (global_status->server_socket,
					-1, global_status->digest_packet);

  if (number_destination != 1)
    {
      fprintf (stderr,
	       "Couldn't send the identification acknowledge packet to client\n");
    }

}

/*!
 * Compare given address with previous address from which slot allocation have been successfully processed
 * \param global_status the global status for the server
 * \parem address the address to check
 * \return -1 is the address was never seen
 * \return the number of slots we allocated for it back then, else
 */
int
compare_request_to_previous (global_status_type * global_status,
			     IPaddress address)
{
  int slot_index;

  for (slot_index = 0; slot_index < global_status->number_allocation_request;
       slot_index++)
    {
      if (equals_address
	  (address, global_status->allocation_request[slot_index].address))
	{
	  /* We found a previous request */
	  return global_status->allocation_request[slot_index].
	    allocated_slots;
	}
    }

  return -1;

}

/*!
 * Process an identification request from a client, filling the interesting
 * fields in the global_status structure to find later the client
 */
void
identify_client (global_status_type * global_status,
		 global_option_type * global_option)
{

  int requested_slots;
  int allocated_slots;
  int slot_index;
  int already_allocated_slot_by_client;

  if (global_status->current_packet->len != PACKET_IDENTIFICATION_LENGTH)
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in identification phase (received length: %d, expected length: %d)\n",
	       global_status->current_packet->len,
	       PACKET_IDENTIFICATION_LENGTH);
#endif
      return;
    }

  if (global_status->current_packet->data[PACKET_IDENTIFIER] !=
      PACKET_IDENTIFICATION_ID)
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in identification phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	       global_status->current_packet->data[PACKET_IDENTIFIER],
	       PACKET_IDENTIFICATION_ID);
#endif
      return;
    }

  if (global_status->current_packet->data[PACKET_IDENTIFICATION_CHECKSUM] !=
      compute_checksum (global_status->current_packet->data,
			PACKET_IDENTIFIER, PACKET_IDENTIFICATION_CHECKSUM))
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Packet checksum received when in identification phase (received checksum: 0x%02x, expected checksum: 0x%02x)\n",
	       global_status->current_packet->
	       data[PACKET_IDENTIFICATION_CHECKSUM],
	       compute_checksum (global_status->current_packet->data,
				 PACKET_IDENTIFIER,
				 PACKET_IDENTIFICATION_CHECKSUM));
#endif
      return;
    }

  requested_slots =
    global_status->current_packet->
    data[PACKET_IDENTIFICATION_NUMBER_REQUESTED_SLOTS];

  allocated_slots =
    min (requested_slots,
	 count_remaining_slot (global_status, global_option));

  /* Check if it's a new request or a re-request (happens when the client didn't get the acknowledgement) */
  already_allocated_slot_by_client =
    compare_request_to_previous (global_status,
				 global_status->current_packet->address);

  if (already_allocated_slot_by_client != -1)
    {
      /* We already answered this client, we'll reacknowledge */
      send_identification_acknowledge_packet (global_status,
					      already_allocated_slot_by_client);

      return;
    }

  for (slot_index = global_status->number_identified_players;
       slot_index <
       global_status->number_identified_players + allocated_slots;
       slot_index++)
    {
      /* Set the address for identified player */
      global_status->input_mapping[slot_index].address.port =
	global_status->current_packet->address.port;

      global_status->input_mapping[slot_index].address.host =
	global_status->current_packet->address.host;


      /* Set the remote input device number */
      global_status->input_mapping[slot_index].remote_input_device =
	global_status->current_packet->
	data[PACKET_IDENTIFICATION_INPUT_DEVICE_INDEX + slot_index -
	     global_status->number_identified_players];

      /* Change status from UNIDENTIFIED */
      global_status->player_status[slot_index] = WAITING;
    }

  global_status->allocation_request[global_status->number_allocation_request].
    address.port = global_status->current_packet->address.port;

  global_status->allocation_request[global_status->number_allocation_request].
    address.host = global_status->current_packet->address.host;

  global_status->allocation_request[global_status->number_allocation_request].
    allocated_slots = allocated_slots;

  global_status->number_allocation_request++;

  global_status->number_identified_players += allocated_slots;

  send_identification_acknowledge_packet (global_status, allocated_slots);

}


/*!
 * Read an incoming packet and stores it for future computations
 * \return non null if a packet was read
 * \return 0 if no packet is available
 */
int
read_incoming_server_packet (global_status_type * global_status)
{
  switch (SDLNet_UDP_Recv
	  (global_status->server_socket, global_status->current_packet))
    {
    case 0:
      return 0;
    case 1:
#if defined(DEBUG)
      fprintf (stderr, "Stored a packet from client.\n");
#endif
      return 1;
    default:
      fprintf (stderr, "Internal warning: impossible case (%s:%s)\n",
	       __FILE__, __LINE__);
      return 0;
    }
  return 0;
}

/*!
 * Process a status packet and updates global input status and client status
 */
void
store_remote_status_lan (global_status_type * global_status,
			 global_option_type * global_option)
{
  int input_index;

  if (global_status->current_packet->len != PACKET_STATUS_LENGTH)
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in status receiving phase (received length: %d, expected length: %d)\n",
	       global_status->current_packet->len, PACKET_STATUS_LENGTH);
#endif

      /* If may be an identification packet which was lost at some point */
      identify_client (global_status, global_option);

      return;
    }

  if (global_status->current_packet->data[PACKET_IDENTIFIER] !=
      PACKET_STATUS_ID)
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in status receiving phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	       global_status->current_packet->data[PACKET_IDENTIFIER],
	       PACKET_STATUS_ID);
#endif
      return;
    }

  if (global_status->frame_number !=
      SDLNet_Read32 (global_status->current_packet->data +
		     PACKET_STATUS_FRAME_NUMBER))
    {

      if (global_status->frame_number ==
	  SDLNet_Read32 (global_status->current_packet->data +
			 PACKET_STATUS_FRAME_NUMBER) + 1)
	{
	  /*
	   * This a request for the previous frame, i.e. a client lost his digest, we resend it to him
	   */

	  send_previous_digest_packet (global_status, global_option,
				       global_status->current_packet->
				       address);

	  return;
	}

      /* Discarding status not corresponding to this frame */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in status receiving phase (status for frame %d, awaiting for frame %d)\n",
	       SDLNet_Read32 (global_status->current_packet->data +
			      PACKET_STATUS_FRAME_NUMBER),
	       global_status->frame_number);
#endif
      return;
    }

  for (input_index = 0; input_index < MAX_NUMBER_PLAYER; input_index++)
    {
      if (equals_address
	  (global_status->current_packet->address,
	   global_status->input_mapping[input_index].address))
	{

	  if (global_status->player_status[input_index] == READY)
	    {
#if defined(DEBUG)
	      fprintf (stderr,
		       "We have another status for the same frame, same origin.");
#endif
	      /* TODO : check if it's the same status we already have stored */
	    }

	  /* Global input number input_index will be filled by this packet status */
	  global_status->input_value[input_index] =
	    global_status->current_packet->
	    data[PACKET_STATUS_INPUT_DEVICE_INDEX +
		 global_status->input_mapping[input_index].
		 remote_input_device];

#if defined(DEBUG)
	  fprintf (stderr, "input value[%d] = 0x%02x (data[%d])\n",
		   input_index,
		   global_status->current_packet->
		   data[PACKET_STATUS_INPUT_DEVICE_INDEX +
			global_status->input_mapping[input_index].
			remote_input_device],
		   PACKET_STATUS_INPUT_DEVICE_INDEX +
		   global_status->input_mapping[input_index].
		   remote_input_device);
#endif

	  /* Slot is now filled */
	  global_status->player_status[input_index] = READY;

	}
    }

}

/*!
 * Reset the input value to begin with a new frame
 */
void
init_frame_status (global_status_type * global_status,
		   global_option_type * global_option)
{
  int player_index;

  for (player_index = 0; player_index < global_option->number_player;
       player_index++)
    {
      previous_input_value[player_index] =
	global_status->input_value[player_index];
      global_status->input_value[player_index] = 0;
      global_status->player_status[player_index] = WAITING;
    }
}


/*!
 * Send a digest packet to a single client
 */
void
send_individual_digest_packet (IPaddress client_address,
			       global_status_type * global_status)
{
  int number_destination;

  global_status->digest_packet->address = client_address;

  number_destination = SDLNet_UDP_Send (global_status->server_socket,
					-1, global_status->digest_packet);

  if (number_destination != 1)
    {
      fprintf (stderr, "Couldn't send the status packet to client\n");
    }

}

/*!
 * Send a digest packet to clients if they reeeeeeeeeally deserve it
 */
void
send_digest_lan_packets (global_status_type * global_status,
			 global_option_type * global_option)
{

  int player_index;

  for (player_index = 0; player_index < global_option->number_player;
       player_index++)
    {
      if (global_status->player_status[player_index] != READY)
	{
#if defined(DEBUG)
	  fprintf (stderr,
		   "Player number %d not being READY prevent the sending of the digest packet\n",
		   player_index);
#endif
	  return;
	}
    }

  /* If we're here, all concerned clients are READY (== we got their status) */

  /* Set the identifier */
  global_status->digest_packet->data[PACKET_IDENTIFIER] = PACKET_DIGEST_ID;

  /* Set the current frame */
  SDLNet_Write32 (global_status->frame_number,
		  global_status->digest_packet->
		  data + PACKET_STATUS_FRAME_NUMBER);

  /* Set the digest input values */
  for (player_index = 0; player_index < global_option->number_player;
       player_index++)
    {
      global_status->digest_packet->data[PACKET_STATUS_INPUT_DEVICE_NUMBER +
					 player_index] =
	global_status->input_value[player_index];
    }

  /* Compute the checksum */
  global_status->digest_packet->
    data[PACKET_STATUS_CHECKSUM] =
    compute_checksum (global_status->digest_packet->data, PACKET_IDENTIFIER,
		      PACKET_STATUS_CHECKSUM);


  global_status->digest_packet->len = PACKET_STATUS_LENGTH;

  /* Now, global_status->digest_packet is THE digest packet we can send to all clients */

  for (player_index = 0; player_index < global_option->number_player;
       player_index++)
    {

      /* TODO : find a much more elegant and efficient way to find unique client addresses */

      int duplicate_index;
      int already_sent;

      already_sent = 0;

      for (duplicate_index = 0; duplicate_index < player_index;
	   duplicate_index++)
	{
	  if (equals_address
	      (global_status->input_mapping[player_index].address,
	       global_status->input_mapping[duplicate_index].address))
	    {
	      already_sent = 1;
	      break;
	    }
	}

      if (!already_sent)
	{
	  send_individual_digest_packet (global_status->
					 input_mapping[player_index].address,
					 global_status);
	}

    }

  /* We're done with this frame, skip to the next one */
  global_status->frame_number++;

  /* Get ready for receiving status for next (now current) frame */
  init_frame_status (global_status, global_option);

}


/*!
 * Send the digest packet of the previous frame to a single client
 */
void
send_previous_digest_packet (global_status_type * global_status,
			     global_option_type * global_option,
			     IPaddress address)
{

  int player_index;

  /* Set the identifier */
  global_status->digest_packet->data[PACKET_IDENTIFIER] = PACKET_DIGEST_ID;

  /* Set the previous frame */
  SDLNet_Write32 (global_status->frame_number - 1,
		  global_status->digest_packet->
		  data + PACKET_STATUS_FRAME_NUMBER);

  /* Set the digest input values */
  for (player_index = 0; player_index < global_option->number_player;
       player_index++)
    {
      global_status->digest_packet->data[PACKET_STATUS_INPUT_DEVICE_NUMBER +
					 player_index] =
	previous_input_value[player_index];
    }

  /* Compute the checksum */
  global_status->digest_packet->
    data[PACKET_STATUS_CHECKSUM] =
    compute_checksum (global_status->digest_packet->data, PACKET_IDENTIFIER,
		      PACKET_STATUS_CHECKSUM);


  global_status->digest_packet->len = PACKET_STATUS_LENGTH;

  /* Now, global_status->digest_packet is the previous digest packet we can send to this given client */

  send_individual_digest_packet (global_status->input_mapping[player_index].
				 address, global_status);
}

/*!
 * Main loop of the lan protocol service
 */
void
serve_clients_lan_protocol (global_status_type * global_status,
			    global_option_type * global_option)
{

  init_frame_status (global_status, global_option);

  for (;;)
    {
      int number_ready_socket;

#if defined(DEBUG)
      fprintf (stderr, "Waiting for status packets for frame %u\n",
	       global_status->frame_number);
#endif

      number_ready_socket =
        SDLNet_CheckSockets (global_status->server_socket_set,
			     SERVER_SOCKET_TIMEOUT);

      if (number_ready_socket == -1)
	{
	  fprintf (stderr, "Error in socket waiting (disconnection ?).\n");
	  break;
	}

      if (number_ready_socket == 1)
	{

#if defined(DEBUG)
	  fprintf (stderr, "Got a packet\n");
#endif

	  /* We're awaiting a packet in the server socket */
	  if (read_incoming_server_packet (global_status))
	    {
	      /* Stores the incoming status and updates clients status accordingly */
	      store_remote_status_lan (global_status, global_option);

	      /* Sends packet (if needed) to client accordingly to their status */
	      send_digest_lan_packets (global_status, global_option);
	    }

	}

    }
}

/*!
 * Return the time remaining before the next timeout to which issue a digest to all clients
 * \param global_status the global status of the server
 */
int
get_remaining_time (global_status_type * global_status)
{
  double next_time;
  uint32 current_time;

  current_time = SDL_GetTicks ();

  next_time =
    global_status->start_time +
    global_status->frame_number * DOUBLE_FRAME_DURATION;

#if defined(DEBUG)
  fprintf (stderr,
	   "Current time : %u ticks (%f second(s))\nTime up next frame (frame %d) : %u ticks (%f second(s))\n",
	   current_time,
	   (double) current_time / (double) SDL_TICKS_PER_SECOND,
	   global_status->frame_number,
	   (uint32) (next_time * 1000.0) - current_time,
	   next_time - current_time / (double) SDL_TICKS_PER_SECOND);

#endif

  if ((uint32) (next_time * 1000.0) < current_time)
    {
      fprintf (stderr, "The server is too slow, we're late for frame %u\n",
	       global_status->frame_number);
      return 0;
    }
  return (uint32) (next_time * 1000.0) - current_time;
}

/*!
 * Store an individual input value
 */
void
store_individual_status (global_status_type * global_status,
			 int input_index, uchar input_value)
{
  global_status->input_value[input_index] = input_value;
}

/*!
 * Store a packet information and updates the current status as well as the next frame asked by the client
 */
void
store_remote_status_internet (global_status_type * global_status,
			      global_option_type * global_option)
{
  int input_index;

  if (global_status->current_packet->len != PACKET_STATUS_LENGTH)
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in status receiving phase (received length: %d, expected length: %d)\n",
	       global_status->current_packet->len, PACKET_STATUS_LENGTH);
#endif

      /* If may be an identification packet which was lost at some point */
      identify_client (global_status, global_option);

      return;
    }

  if (global_status->current_packet->data[PACKET_IDENTIFIER] !=
      PACKET_STATUS_ID)
    {
      /* Discarding invalid packet */
#if defined(DEBUG)
      fprintf (stderr,
	       "Invalid packet received when in status receiving phase (received ID: 0x%02x, expected ID: 0x%02x)\n",
	       global_status->current_packet->data[PACKET_IDENTIFIER],
	       PACKET_STATUS_ID);
#endif
      return;
    }

  /* TODO : add checksum testing */

  for (input_index = 0; input_index < MAX_NUMBER_PLAYER; input_index++)
    {
      if (equals_address
	  (global_status->current_packet->address,
	   global_status->input_mapping[input_index].address))
	{

	  /* Update the status with the client status */
	  store_individual_status (global_status, input_index,
				   global_status->current_packet->
				   data[PACKET_STATUS_INPUT_DEVICE_INDEX +
					global_status->
					input_mapping[input_index].
					remote_input_device]);

	  if (SDLNet_Read32
	      (global_status->current_packet->data +
	       PACKET_STATUS_FRAME_NUMBER) != 0)
	    {

	      /* The client requested a given frame */

	      /* Update the next frame to send to this player */
	      global_status->next_frame_asked[input_index] =
		SDLNet_Read32 (global_status->current_packet->data +
			       PACKET_STATUS_FRAME_NUMBER);

#if defined(DEBUG)
	      if (global_status->next_frame_asked[input_index] ==
		  global_status->frame_number)
		{
		  fprintf (stderr,
			   "Great, player is waiting for the current frame, lag is less than a frame delay (player %d).\n",
			   input_index);
		}

	      if (global_status->next_frame_asked[input_index] ==
		  global_status->next_frame_to_send[input_index])
		{
		  fprintf (stderr,
			   "Half great, we don't have packets loss, the game should be smooth (player %d).\n",
			   input_index);
		}

#endif

	      if (global_status->next_frame_asked[input_index] >
		  global_status->frame_number)
		{
		  fprintf (stderr,
			   "Error, player asked a frame which hasn't been reached yet! (asked: %d, maximum: %d)\n",
			   global_status->next_frame_asked[input_index],
			   global_status->frame_number);
		  global_status->next_frame_asked[input_index] =
		    global_status->frame_number;

		}

	      if ((global_status->frame_number > AUTO_OUTDATE)
		  && (global_status->next_frame_asked[input_index] <=
		      global_status->frame_number - AUTO_OUTDATE))
		{
		  fprintf (stderr,
			   "FATAL ERROR !! Player requested an outdated frame, we can't provide it! (requested : %d, available range : %d - %d)\n",
			   global_status->next_frame_asked[input_index],
			   global_status->frame_number - AUTO_OUTDATE,
			   global_status->frame_number);

		  /* TODO: think about actions which should be performed in this case. */
		}

	      /* Update the counter for sending frames */
	      global_status->next_frame_to_send[input_index] =
		global_status->next_frame_asked[input_index];
	    }
	  else
	    {
#if defined(DEBUG)
	      fprintf (stderr,
		       "Client sends a status without requesting a specific frame.\n");
#endif
	    }
	}
    }
}

/*!
 * Allocate memory for the history
 */
void
init_internet_history ()
{
  if (history_digest != NULL)
    {
      fprintf (stderr, "Attempting to reallocate the history.\n");
      return;
    }

  history_digest = (uchar *) malloc ((size_t) BUFFER_SIZE_BYTES);

  if (history_digest == NULL)
    {
      fprintf (stderr,
	       "FATAL ERROR ! Couldn't allocate memory for the history. Exiting.\n");
      exit (2);
    }
}

/*!
 * Free memory for the history
 */
void
release_internet_history ()
{
  if (history_digest == NULL)
    {
      fprintf (stderr, "Attempting to free non allocated history.\n");
      return;
    }

  free (history_digest);

}

/*!
 * Push the current frame status in the history
 */
void
store_current_status_in_history (global_status_type * global_status)
{
  memcpy (history_digest +
	  (global_status->frame_number % AUTO_OUTDATE) * DIGEST_SIZE,
	  global_status->input_value, DIGEST_SIZE);
}

/*!
 * Send a packet to a client to fit the need in term of frame number(s)
 * \param global_status global status for the application
 * \param client_index index of the global player to which send a packet
 */
void
send_individual_internet_digest_packet (global_status_type * global_status,
					int client_index)
{
  unsigned int frame_number;
  unsigned int frame_number_start;
  int number_digest;
  int digest_index;
  int number_destination;

  /* Set the identifier */
  global_status->digest_packet->data[PACKET_IDENTIFIER] =
    PACKET_INTERNET_DIGEST_ID;

  /* Set the number of the first frame digest */
  SDLNet_Write32 (global_status->next_frame_to_send[client_index],
		  global_status->digest_packet->data +
		  PACKET_INTERNET_DIGEST_FRAME_NUMBER);

  /* Compute the number of digest to send */
  number_digest =
    min (PACKET_INTERNET_DIGEST_DIGEST_NUMBER, global_status->frame_number);

  if (number_digest < PACKET_INTERNET_DIGEST_DIGEST_NUMBER)
    {
      /* We're at the beginning of the game */
      frame_number_start = 1;
    }
  else
    {
      if (global_status->next_frame_to_send[client_index] + number_digest >
	  global_status->frame_number)
	{
	  /* 
	   * If we feed the client with frames between what is expected and what we can provide at max, we won't fill all
	   * the space we have. Let's then feed the maximum of space up to what we can provide (thus giving frames which are
	   * thought to be older than what the client is waiting for [but it could be false and effectively waiting them])
	   */
	  frame_number_start =
	    global_status->frame_number -
	    PACKET_INTERNET_DIGEST_DIGEST_NUMBER + 1;
	}
      else
	{
	  /*
	   * We can fill the whole space with data from what was requested and we won't reach the last frame for which
	   * we can provide the status
	   */
	  frame_number_start =
	    global_status->next_frame_to_send[client_index];
	}
    }

#if defined(DEBUG)
  fprintf (stderr,
	   "number of digests to send = %d (%d-%d)\nnext_frame_to_send = %d, frame_number = %d\n",
	   number_digest,
	   frame_number_start,
	   frame_number_start + number_digest - 1,
	   global_status->next_frame_to_send[client_index],
	   global_status->frame_number);
#endif

  /* Set the number of digest to send */
  global_status->digest_packet->data[PACKET_INTERNET_DIGEST_NUMBER_DIGEST] =
    number_digest;

  global_status->digest_packet->len =
    PACKET_INTERNET_DIGEST_BASE_LENGTH +
    number_digest * PACKET_INTERNET_DIGEST_INCREMENT_LENGTH;

  /* Set destination address */
  global_status->digest_packet->address =
    global_status->input_mapping[client_index].address;

  for (digest_index = 0, frame_number = frame_number_start;
       digest_index < number_digest; digest_index++, frame_number++)
    {
      memcpy (global_status->digest_packet->data +
	      PACKET_INTERNET_DIGEST_DIGEST_INDEX +
	      digest_index * DIGEST_SIZE,
	      history_digest + (frame_number % AUTO_OUTDATE) * DIGEST_SIZE,
	      DIGEST_SIZE);
    }

  /* The checksum is located after the last digest data */
  global_status->digest_packet->data[PACKET_INTERNET_DIGEST_DIGEST_INDEX +
				     number_digest * DIGEST_SIZE] =
    compute_checksum (global_status->digest_packet->data, PACKET_IDENTIFIER,
		      PACKET_INTERNET_DIGEST_DIGEST_INDEX +
		      number_digest * DIGEST_SIZE);

  number_destination = SDLNet_UDP_Send (global_status->server_socket,
					-1, global_status->digest_packet);

  if (number_destination != 1)
    {
      fprintf (stderr, "Couldn't send the digest packet to client\n");
    }

  /* Next time, we'll send the next frame we haven't send yet */
  global_status->next_frame_to_send[client_index] = frame_number;

}

/*!
 * Send a digest packet to all clients
 */
void
send_internet_digest (global_status_type * global_status,
		      global_option_type * global_option)
{

  int player_index;
  int client_index;

  /* Store the current status in the history */
  store_current_status_in_history (global_status);

  for (player_index = 0, client_index = 0;
       player_index < global_option->number_player; player_index++)
    {

      /* TODO : find a much more elegant and efficient way to find unique client addresses */

      int duplicate_index;
      int already_sent;

      already_sent = 0;

      for (duplicate_index = 0; duplicate_index < player_index;
	   duplicate_index++)
	{
	  if (equals_address
	      (global_status->input_mapping[player_index].address,
	       global_status->input_mapping[duplicate_index].address))
	    {
	      already_sent = 1;
	      break;
	    }
	}

      if (!already_sent)
	{
	  send_individual_internet_digest_packet (global_status,
						  client_index++);
	}

    }

  /* Advancing the frame number */
  global_status->frame_number++;

}

/*!
 * Main loop of the internet protocol service
 */
void
serve_clients_internet_protocol (global_status_type * global_status,
				 global_option_type * global_option)
{

  int player_index;

  for (player_index = 0; player_index < MAX_NUMBER_PLAYER; player_index++)
    {
      global_status->next_frame_to_send[player_index] = 1;
      global_status->input_value[player_index] = 0;
    }

  /* Allocate memory for the history */
  init_internet_history ();

  /* Prepare memory releasing at the end of the program */
  atexit (release_internet_history);

  init_frame_status (global_status, global_option);

  global_status->start_time =
    (double) SDL_GetTicks () / (double) SDL_TICKS_PER_SECOND;

  for (;;)
    {

#if defined(DEBUG)
      fprintf (stderr,
	       "Waiting for status packets while preparing frame %u\ntime = %u\n",
	       global_status->frame_number, SDL_GetTicks ());
#endif

      /* Let packets accumulate for a short amount of time */
      SDL_Delay (get_remaining_time (global_status));

#if defined(DEBUG)
      fprintf (stderr, "Finished sleeping, time = %u\n", SDL_GetTicks ());
#endif

      while (read_incoming_server_packet (global_status) != 0)
	{

#if defined(DEBUG)
	  fprintf (stderr, "Got a packet\n");
#endif

	  /* Stores the incoming status */
	  store_remote_status_internet (global_status, global_option);
	}

      send_internet_digest (global_status, global_option);

    }

  /* While the above loop is infinite, this won't be called */
  release_internet_history ();

}
