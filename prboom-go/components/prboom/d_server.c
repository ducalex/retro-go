/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Network game server code
 *  New for LxDoom, but drawing ideas and code fragments from the
 *  earlier net code
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#ifdef USE_SDL_NET
 #include "SDL.h"
#endif

#include "doomtype.h"
#include "protocol.h"
#include "i_network.h"
#ifndef PRBOOM_SERVER
#include "m_fixed.h"
#endif
#include "i_system.h"
#include "m_swap.h"

#ifndef HAVE_NET

int main(void)
{
  fprintf(stderr,
      PACKAGE "-server: You must compile with networking enabled!\n");
  exit(1);
  return 1;
}

#else

#ifndef HAVE_GETOPT
/* The following code for getopt is from the libc-source of FreeBSD,
 * it might be changed a little bit.
 * Florian Schulze (florian.schulze@gmx.net)
 */

/*
 * Copyright (c) 1987, 1993, 1994
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)getopt.c  8.3 (Berkeley) 4/27/95";
#endif
static const char rcsid[] = "$FreeBSD$";
#endif /* LIBC_SCCS and not lint */

int opterr = 1,   /* if error message should be printed */
  optind = 1,   /* index into parent argv vector */
  optopt,     /* character checked for validity */
  optreset;   /* reset getopt */
char  *optarg;    /* argument associated with option */

#define BADCH (int)'?'
#define BADARG  (int)':'
#define EMSG  ""

char *__progname="prboom_server";

/*
 * getopt --
 *  Parse argc/argv argument vector.
 */
int
getopt(nargc, nargv, ostr)
  int nargc;
  char * const *nargv;
  const char *ostr;
{
  extern char *__progname;
  static char *place = EMSG;    /* option letter processing */
  char *oli;        /* option letter list index */
  int ret;

  if (optreset || !*place) {    /* update scanning pointer */
    optreset = 0;
    if (optind >= nargc || *(place = nargv[optind]) != '-') {
      place = EMSG;
      return (-1);
    }
    if (place[1] && *++place == '-') {  /* found "--" */
      ++optind;
      place = EMSG;
      return (-1);
    }
  }         /* option letter okay? */
  if ((optopt = (int)*place++) == (int)':' ||
      !(oli = strchr(ostr, optopt))) {
    /*
     * if the user didn't specify '-' as an option,
     * assume it means -1.
     */
    if (optopt == (int)'-')
      return (-1);
    if (!*place)
      ++optind;
    if (opterr && *ostr != ':')
      (void)fprintf(stderr,
          "%s: illegal option -- %c\n", __progname, optopt);
    return (BADCH);
  }
  if (*++oli != ':') {      /* don't need argument */
    optarg = NULL;
    if (!*place)
      ++optind;
  }
  else {          /* need an argument */
    if (*place)     /* no white space */
      optarg = place;
    else if (nargc <= ++optind) { /* no arg */
      place = EMSG;
      if (*ostr == ':')
        ret = BADARG;
      else
        ret = BADCH;
      if (opterr)
        (void)fprintf(stderr,
            "%s: option requires an argument -- %c\n",
            __progname, optopt);
      return (ret);
    }
    else        /* white space */
      optarg = nargv[optind];
    place = EMSG;
    ++optind;
  }
  return (optopt);      /* dump back option letter */
}
#else
#include <unistd.h>
#endif

#define MAXPLAYERS 4
#define BACKUPTICS 12

// Dummies to forfill l_udp.c unused client stuff
int M_CheckParm(const char* p) { p = NULL; return 1; }
int myargc;
char** myargv;

void NORETURN I_Error(const char *error, ...) // killough 3/20/98: add const
{
  va_list argptr;
  va_start(argptr,error);
  vfprintf(stderr,error,argptr);
  va_end(argptr);
  exit(-1);
}

int playerjoingame[MAXPLAYERS], playerleftgame[MAXPLAYERS];
UDP_CHANNEL remoteaddr[MAXPLAYERS];
enum { pc_unused, pc_connected, pc_ready, pc_confirmedready, pc_playing, pc_quit } playerstate[MAXPLAYERS];
int displaycounter;

boolean n_players_in_state(int n, int ps) {
	int i,j;
	for (i=j=0;i<MAXPLAYERS;i++)
		if (playerstate[i] == ps) j++;
	return (j == n);
}

void BroadcastPacket(packet_header_t *packet, size_t len)
{
  int i;
  for (i=0; i<MAXPLAYERS; i++)
    if (playerstate[i] != pc_unused && playerstate[i] != pc_quit)
      I_SendPacketTo(packet, len, &remoteaddr[i]);
}

byte def_game_options[GAME_OPTIONS_SIZE] = \
{ // cf g_game.c:G_WriteOptions()
  1, // monsters remember
  1, // friction
  0, // weapon recoil
  1, // pushers
  0, // reserved/unused
  1, // player bobbing
  0, 0, 0, // respawn, fast, nomonsters
  1, // demo insurance
  0, 0, 0, 0, // 4 bytes of random number seed
  1, 0, 0, 0,
  0, 128, /* distfriend */
  0, 1, 1, 1, 1, 0,
  /* Zeroes for all compatibility stuff */
};

#define nosuch "NOSUCHCONFIGITEM"

const char* gameopt_config_names[] = {
"monsters_remember",nosuch,"weapon_recoil",nosuch,nosuch,"player_bobbing",nosuch,nosuch,nosuch,"demo_insurance",
nosuch,nosuch,nosuch,nosuch, // RNG seed
"monster_infighting","player_helpers",nosuch,nosuch,
nosuch,nosuch, // distfriend
"monster_backing","monster_avoid_hazards","monster_friction","help_friends","dog_jumping","monkeys",
"comp_telefrag","comp_dropoff","comp_vile","comp_pain","comp_skull","comp_blazing","comp_doorlight","comp_model","comp_god","comp_falloff","comp_floors","comp_skymap","comp_pursuit","comp_doorstuck","comp_staylift","comp_zombie","comp_stairs","comp_infcheat","comp_zerotags","comp_moveblock","comp_respawn","comp_sound" };

const int num_gameopts = sizeof gameopt_config_names / sizeof gameopt_config_names[0];

int verbose;

void NORETURN sig_handler(int signum)
{
  char buf[80];
  I_SigString(buf,80,signum);
  printf("Received signal: %s\n", buf);

  // Any signal is fatal
  exit(1);
}

void doexit(void)
{
  packet_header_t packet;

  // Send "downed" packet
  packet_set(&packet, PKT_DOWN, 0);
  BroadcastPacket(&packet, sizeof packet);
}

#ifndef USE_SDL_NET
static void I_InitSockets(int v4port)
{
  v4socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  I_SetupSocket(v4socket, v4port, AF_INET);
}
#else
static void I_InitSockets(Uint16 port)
{
  I_InitNetwork();
  udp_socket = I_Socket(port);
  if (!udp_socket) I_Error("I_InitSockets: failed to open UDP port %d\n",port);
}
#endif

long int ptic(packet_header_t* p)
{
    return doom_ntohl(p->tic);
}



void read_config_file(FILE* fp, struct setup_packet_s* sp)
{
  byte* gameopt = sp->game_options;

  while (!feof(fp)) {
    char  def[80];
    char  strparm[100];
    if (fscanf (fp, "%79s %99[^\n]\n", def, strparm) == 2) {
      int v = atoi(strparm);
      if (!strcmp(def,"default_skill")) {
        if (verbose) printf("config file sets default_skill to %d\n",v);
        sp->skill = v-1;
      } else if (!strcmp(def,"default_compatibility_level")) {
        if (verbose) printf("config file sets compatibility_level to %d\n",v);
	if (v == -1) v = MAX_COMPATIBILITY_LEVEL-1; //e6y: -1 => maxcompat
        sp->complevel = v;
      } else {
        int i;
        for (i=0; i<num_gameopts; i++) {
	  if (!!strcmp(gameopt_config_names[i],def)) continue;
	  if (verbose) printf("config file sets %s to %d\n",def,v);
	  gameopt[i] = v;
	}
      }
    }
  }
}

static int badplayer(int n) { return (n < 0 || n >= MAXPLAYERS); }

int main(int argc, char** argv)
{
#ifndef USE_SDL_NET
  int localport = 5030;
#else
  Uint16 localport = 5030;
#endif
  int numplayers = 2, xtratics = 0, ticdup = 1;
  int exectics = 0; // gametics completed
  struct setup_packet_s setupinfo = { 2, 0, 1, 1, 1, 0, best_compatibility, 0, 0};
  char**wadname = NULL;
  char**wadget = NULL;
  int numwads = 0;
  {
    int opt;
    byte *gameopt = setupinfo.game_options;

    memcpy(gameopt, &def_game_options, sizeof (setupinfo.game_options));
    while ((opt = getopt(argc, argv, "c:t:x:p:e:l:adrfns:N:vw:")) != EOF)
      switch (opt) {
      case 'c':
        {
	  FILE *cf = fopen(optarg,"r");
	  if (!cf) { perror("fopen"); return -1; }
	  read_config_file(cf,&setupinfo);
	  fclose(cf);
	}
	break;
      case 't':
  if (optarg) ticdup = atoi(optarg);
  break;
      case 'x':
  if (optarg) xtratics = atoi(optarg);
  break;
      case 'p':
  if (optarg) localport = atoi(optarg);
  break;
      case 'e':
  if (optarg) setupinfo.episode = atoi(optarg);
  break;
      case 'l':
  if (optarg) setupinfo.level = atoi(optarg);
  break;
      case 'a':
  setupinfo.deathmatch = 2;
  break;
      case 'd':
  setupinfo.deathmatch = 1;
  break;
      case 'r':
  setupinfo.game_options[6] = 1;
  break;
      case 'f':
  setupinfo.game_options[7] = 1;
  break;
      case 'n':
  setupinfo.game_options[8] = 1;
  break;
      case 's':
  if (optarg) setupinfo.skill = atoi(optarg)-1;
  break;
      case 'N':
  if (optarg) setupinfo.players = numplayers = atoi(optarg);
  break;
      case 'v':
  verbose++;
  break;
      case 'w':
  if (optarg) {
    char *p;
    wadname = realloc(wadname, ++numwads * sizeof *wadname);
    wadget  = realloc(wadget ,   numwads * sizeof *wadget );
    wadname[numwads-1] = strdup(optarg);
    if ((p = strchr(wadname[numwads-1], ','))) {
      *p++ = 0; wadget[numwads-1] = p;
    } else wadget[numwads-1] = NULL;
  }
  break;
      }
  }

  setupinfo.ticdup = ticdup; setupinfo.extratic = xtratics;
  { /* Random number seed
     * Mirrors the corresponding code in G_ReadOptions */
    int rngseed = (int)time(NULL);
    setupinfo.game_options[13] = rngseed & 0xff;
    rngseed >>= 8;
    setupinfo.game_options[12] = rngseed & 0xff;
    rngseed >>= 8;
    setupinfo.game_options[11] = rngseed & 0xff;
    rngseed >>= 8;
    setupinfo.game_options[10] = rngseed & 0xff;
  }
  I_InitSockets(localport);

  printf("Listening on port %d, waiting for %d players\n", localport, numplayers);

  { // no players initially
    int i;
    for (i=0; i<MAXPLAYERS; i++) {
      playerjoingame[i] = INT_MAX;
      playerleftgame[i] = 0;
      playerstate[i] = pc_unused;
    }

    // Print wads
    for (i=0; i<numwads; i++)
      printf("Wad %s (%s)\n", wadname[i], wadget[i] ? wadget[i] : "");
  }

  // Exit and signal handling
  atexit(doexit); // heh
  signal(SIGTERM, sig_handler);
  signal(SIGINT , sig_handler);
#ifndef USE_SDL_NET
  signal(SIGQUIT, sig_handler);
  signal(SIGKILL, sig_handler);
  signal(SIGHUP , sig_handler);
#endif

  {
    int remoteticfrom[MAXPLAYERS] = { 0, 0, 0, 0 };
    int remoteticto[MAXPLAYERS] = { 0, 0, 0, 0 };
    int backoffcounter[MAXPLAYERS] = { 0, 0, 0, 0 };
    int curplayers = 0;
    int confirming = 0;
    boolean ingame = false;
    ticcmd_t netcmds[MAXPLAYERS][BACKUPTICS];

    while (1) {
  packet_header_t *packet = malloc(10000);
  size_t len;

  I_WaitForPacket(120*1000);
  while ((len = I_GetPacket(packet, 10000))) {
    if (verbose>2) printf("Received packet:");
    switch (packet->type) {
    case PKT_INIT:
      if (!ingame) {
        {
    int n;
    struct setup_packet_s *sinfo = (void*)(packet+1);

    /* Find player number and add to the game */
    n = *(short*)(packet+1);

    if (badplayer(n) || playerstate[n] != pc_unused)
     for (n=0; n<numplayers; n++)
      if (playerstate[n] == pc_unused) break;

    if (n == numplayers) break; // Full game
    playerstate[n] = pc_connected;
#ifndef USE_SDL_NET
    remoteaddr[n] = sentfrom;
#else
    if (sentfrom==-1)
      remoteaddr[n]=I_RegisterPlayer(&sentfrom_addr);
#endif

    printf("Join by ");
    I_PrintAddress(stdout, &remoteaddr[n]);
#ifdef USE_SDL_NET
    printf(" (channel %d)",remoteaddr[n]);
#endif
    printf(" as player %d\n",n);
    {
      int i;
      size_t extrabytes = 0;
      // Send setup packet
      packet_set(packet, PKT_SETUP, 0);
      memcpy(sinfo, &setupinfo, sizeof setupinfo);
      sinfo->yourplayer = n;
      sinfo->numwads = numwads;
      for (i=0; i<numwads; i++) {
        strcpy(sinfo->wadnames + extrabytes, wadname[i]);
        extrabytes += strlen(wadname[i]) + 1;
      }
      I_SendPacketTo(packet, sizeof *packet + sizeof setupinfo + extrabytes,
         remoteaddr+n);
      I_uSleep(10000);
      I_SendPacketTo(packet, sizeof *packet + sizeof setupinfo + extrabytes,
         remoteaddr+n);
    }
        }
      }
      break;
    case PKT_GO:
      if (!ingame) {
        int from = *(byte*)(packet+1);

	if (badplayer(from) || playerstate[from] == pc_unused) break;
	if (confirming) {
		if (playerstate[from] != pc_confirmedready) curplayers++;
		playerstate[from] = pc_confirmedready;
	} else
		playerstate[from] = pc_ready;
      }
      break;
    case PKT_TICC:
      {
        byte tics = *(byte*)(packet+1);
        int from = *(((byte*)(packet+1))+1);

	if (badplayer(from)) break;

        if (verbose>2)
            printf("tics %ld - %ld from %d\n", ptic(packet), ptic(packet) + tics - 1, from);
        if (ptic(packet) > remoteticfrom[from]) {
            // Missed tics, so request a resend
            packet_set(packet, PKT_RETRANS, remoteticfrom[from]);
            I_SendPacketTo(packet, sizeof *packet, remoteaddr+from);
        } else {
            ticcmd_t *newtic = (void*)(((byte*)(packet+1))+2);
            if (ptic(packet) + tics < remoteticfrom[from]) break; // Won't help
            remoteticfrom[from] = ptic(packet);
            while (tics--)
              netcmds[from][remoteticfrom[from]++%BACKUPTICS] =  *newtic++;
        }
      }
      break;
    case PKT_RETRANS:
      {
        int from = *(byte*)(packet+1);
	if (badplayer(from)) break;

        if (verbose>2) printf("%d requests resend from %d\n", from, ptic(packet));
        remoteticto[from] = ptic(packet);
      }
      break;
    case PKT_QUIT:
      {
        int from = *(byte*)(packet+1);
	if (badplayer(from)) break;

	if (!ingame && playerstate[from] != pc_unused) {
	  // If we already got a PKT_GO, we have to remove this player frmo the count of ready players. And we then flag this player slot as vacant.
	  printf("player %d pulls out\n", from);
	  if (playerstate[from] == pc_confirmedready) curplayers--;
	  playerstate[from] = pc_unused;
	} else
        if (playerleftgame[from] == INT_MAX) { // In the game
	  playerleftgame[from] = ptic(packet);
	  --curplayers;
	  if (verbose) printf("%d quits at %d (%d left)\n", from, ptic(packet),curplayers);
	  if (ingame && !curplayers) exit(0); // All players have exited
        }
      }
      // Fall through and broadcast it
    case PKT_EXTRA:
      BroadcastPacket(packet, len);
      if (packet->type == PKT_EXTRA) {
        if (verbose>2) printf("misc from %d\n", *(((byte*)(packet+1))+1));
      }
      break;
    case PKT_WAD:
      {
        int i;
        int from = *(byte*)(packet+1);
        char *name = 1 + (char*)(packet+1);
        size_t size = sizeof(packet_header_t);
        packet_header_t *reply;

	if (badplayer(from) || playerstate[from] != pc_unused) break;

        if (verbose) printf("Request for %s ", name);
        for (i=0; i<numwads; i++)
    if (!strcasecmp(name, wadname[i]))
      break;

        if ((i==numwads) || !wadget[i]) {
    if (verbose) printf("n/a\n");
    *(char*)(packet+1) = 0;
    I_SendPacketTo(packet, size+1, remoteaddr + from);
        } else {
    size += strlen(wadname[i]) + strlen(wadget[i]) + 2;
    reply = malloc(size);
    packet_set(reply, PKT_WAD, 0);
    strcpy((char*)(reply+1), wadname[i]);
    strcpy((char*)(reply+1) + strlen(wadname[i]) + 1, wadget[i]);
    printf("sending %s\n", wadget[i]);
    I_SendPacketTo(reply, size, remoteaddr + from);
    free(reply);
        }
      }
      break;
    default:
      printf("Unrecognised packet type %d\n", packet->type);
      break;
    }
  }
  free(packet);
  if (!ingame && n_players_in_state(numplayers,pc_confirmedready)) {
    int i;
    packet_header_t gopacket;
    packet = &gopacket;
    ingame=true;
    printf("All players joined, beginning game.\n");
    for (i=0; i<MAXPLAYERS; i++) {
      if (playerstate[i] == pc_confirmedready) {
	      playerjoingame[i] = 0;
	      playerleftgame[i] = INT_MAX;
	      playerstate[i] = pc_playing;
      }
    }
    packet_set(packet, PKT_GO, 0);
    BroadcastPacket(packet, sizeof *packet);
    I_uSleep(10000);
    BroadcastPacket(packet, sizeof *packet);
    I_uSleep(10000);
  }
  if (confirming && !--confirming && !ingame) {
    int i;
    curplayers = 0;
    for (i=0; i<MAXPLAYERS; i++) {
      if (playerstate[i] == pc_ready) {
	      playerstate[i] = pc_unused;
	      printf("Player %d dropped, no PKT_GO received in confirmation\n", i);
      }
      if (playerstate[i] == pc_confirmedready) playerstate[i] = pc_ready;
    }
  }
  if (!ingame && n_players_in_state(numplayers,pc_ready)) {
	  printf("All players ready, now confirming.\n");
	  confirming = 100;
  }

  if (ingame) { // Run some tics
  int lowtic = INT_MAX;
  int i;
  for (i=0; i<MAXPLAYERS; i++)
    if (playerstate[i] == pc_playing || playerstate[i] == pc_quit) {
      if (remoteticfrom[i] < playerleftgame[i]-1 && remoteticfrom[i]<lowtic)
        lowtic = remoteticfrom[i];
    }

  if (verbose>1) printf("%d new tics can be run\n", lowtic - exectics);

  if (lowtic > exectics)
    exectics = lowtic; // count exec'ed tics
  // Now send all tics up to lowtic
  for (i=0; i<MAXPLAYERS; i++)
    if (playerstate[i] == pc_playing) {
      int tics;
      if (lowtic <= remoteticto[i]) continue;
      if ((remoteticto[i] -= xtratics) < 0) remoteticto[i] = 0;
      tics = lowtic - remoteticto[i];
      {
        byte *p;
        packet = malloc(sizeof(packet_header_t) + 1 +
         tics * (1 + numplayers * (1 + sizeof(ticcmd_t))));
        p = (void*)(packet+1);
        packet_set(packet, PKT_TICS, remoteticto[i]);
        *p++ = tics;
        if (verbose>1) printf("sending %d tics to %d\n", tics, i);
        while (tics--) {
    int j, playersthistic = 0;
    byte *q = p++;
    for (j=0; j<MAXPLAYERS; j++)
      if ((playerjoingame[j] <= remoteticto[i]) &&
          (playerleftgame[j] > remoteticto[i])) {
        *p++ = j;
        memcpy(p, &netcmds[j][remoteticto[i]%BACKUPTICS], sizeof(ticcmd_t));
        p += sizeof(ticcmd_t);
        playersthistic++;
      }
    *q = playersthistic;
    remoteticto[i]++;
        }
        I_SendPacketTo(packet, p - ((byte*)packet), remoteaddr+i);
        free(packet);
      }
      {
        if (remoteticfrom[i] == remoteticto[i]) {
	  backoffcounter[i] = 0;
	} else if (remoteticfrom[i] > remoteticto[i]+1) {
	  if ((backoffcounter[i] += remoteticfrom[i] - remoteticto[i] - 1) > 35) {
	    packet_header_t *packet = malloc(sizeof(packet_header_t));
	    packet_set(packet, PKT_BACKOFF, remoteticto[i]);
	    I_SendPacketTo(packet,sizeof *packet,remoteaddr+i);
	    backoffcounter[i] = 0;
	    if (verbose) printf("telling client %d to back off\n",i);
	    free(packet);
	  }
	}
      }
    }
  }
      if (!((ingame ? 0xff : 0xf) & displaycounter++)) { 
        int i;
        fprintf(stderr,"Player states: [");
        for (i=0;i<MAXPLAYERS;i++) {
            switch (playerstate[i]) {
                case pc_unused: fputc(' ',stderr); break;
                case pc_connected: fputc('c',stderr); break;
                case pc_ready: fputc('r',stderr); break;
                case pc_confirmedready: fputc('g',stderr); break;
                case pc_playing: fputc('p',stderr); break;
                case pc_quit: fputc('x',stderr); break;
            }
        }
        fprintf(stderr,"]\n");
      }
    }
  }
}

#endif // HAVE_NET
