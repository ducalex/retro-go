/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "pce.h"
#include "utils.h"

uchar pce_cd_cmdcnt;

uint32 pce_cd_sectoraddy;

uchar pce_cd_sectoraddress[3];

uchar pce_cd_temp_dirinfo[4];

uchar pce_cd_temp_play[4];

uchar pce_cd_temp_stop[4];

uchar pce_cd_dirinfo[4];

extern uchar pce_cd_adpcm_trans_done;

uchar cd_port_180b = 0;

uchar cd_fade;
// the byte set by the fade function

static void
pce_cd_set_sector_address(void)
{
	pce_cd_sectoraddy = pce_cd_sectoraddress[0] << 16;
	pce_cd_sectoraddy += pce_cd_sectoraddress[1] << 8;
	pce_cd_sectoraddy += pce_cd_sectoraddress[2];
}


static void
pce_cd_handle_command(void)
{
	if (pce_cd_cmdcnt) {
#if ENABLE_TRACING_CD_2
		TRACE("CDRom2: Command received: 0x%02x.\n", io.cd_port_1801);
#endif

		if (--pce_cd_cmdcnt)
			io.cd_port_1800 = 0xd0;
		else
			io.cd_port_1800 = 0xc8;

		switch (pce_cd_curcmd) {
		case 0x08:
			// SCSI CD READ
			if (!pce_cd_cmdcnt) {
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: Read command: %d sectors.\n",
					  io.cd_port_1801);
				TRACE("CDRom2: Starting at %02x:%02x:%02x.\n",
					  pce_cd_sectoraddress[0], pce_cd_sectoraddress[1],
					  pce_cd_sectoraddress[2]);
				TRACE("CDRom2: MODE: %x\n", Rd6502(0x20FF));
#endif
				cd_sectorcnt = io.cd_port_1801;
				if (cd_sectorcnt == 0) {
					MESSAGE_ERROR("%s, cd_sectorcnt == 0 !!!\n", __func__);
				}

				pce_cd_set_sector_address();
				pce_cd_read_sector();


				/* TEST */
				// cd_port_1800 = 0xD0; // Xanadu 2 doesn't block but still crash
				/* TEST */

#if ENABLE_TRACING_CD_2
				TRACE("Result of reading : $1800 = 0X%02X\n",
					  io.cd_port_1800);
#endif

				/* TEST ZEO
				   if (Rd6502(0x20ff)==0xfe)
				   cd_port_1800 = 0x98;
				   else
				   cd_port_1800 = 0xc8;
				   * ******** */
			} else
				pce_cd_sectoraddress[3 - pce_cd_cmdcnt] = io.cd_port_1801;

			break;
		case 0xd8:
			// PLAY AUDIO CD TRACK
			pce_cd_temp_play[pce_cd_cmdcnt] = io.cd_port_1801;

			if (!pce_cd_cmdcnt) {
				io.cd_port_1800 = 0xd8;
			}
			break;
		case 0xd9:
			// PLAY AUDO CD TRACK SECTION
			pce_cd_temp_stop[pce_cd_cmdcnt] = io.cd_port_1801;

			if (!pce_cd_cmdcnt) {
				io.cd_port_1800 = 0xd8;
				/*
				   if (pce_cd_temp_stop[3] == 1)
				   osd_cd_play_audio_track(bcdbin[pce_cd_temp_play[2]]);
				   else
				 */
				if ((pce_cd_temp_play[0] |
					 pce_cd_temp_play[1] |
					 pce_cd_temp_stop[0] | pce_cd_temp_stop[1]) == 0) {
					if (CD_emulation == 5)
						HCD_play_track(bcdbin[pce_cd_temp_play[2]], 1);
					else
						osd_cd_play_audio_track(bcdbin
												[pce_cd_temp_play[2]]);
				} else {
					if (CD_emulation == 5) {
						HCD_play_sectors(Time2Frame
										 (bcdbin[pce_cd_temp_play[2]],
										  bcdbin[pce_cd_temp_play[1]],
										  bcdbin[pce_cd_temp_play[0]]),
										 Time2Frame(bcdbin
													[pce_cd_temp_stop[2]],
													bcdbin[pce_cd_temp_stop
														   [1]],
													bcdbin[pce_cd_temp_stop
														   [0]]),
										 pce_cd_temp_stop[3] == 1);
					} else {
						osd_cd_play_audio_range(bcdbin
												[pce_cd_temp_play[2]],
												bcdbin[pce_cd_temp_play
													   [1]],
												bcdbin[pce_cd_temp_play
													   [0]],
												bcdbin[pce_cd_temp_stop
													   [2]],
												bcdbin[pce_cd_temp_stop
													   [1]],
												bcdbin[pce_cd_temp_stop
													   [0]]);
					}
				}

#if ENABLE_TRACING_CD
				Log("Play from %d:%d:%d:(%d) to %d:%d:%d:(%d)\nloop = %d\n", bcdbin[pce_cd_temp_play[2]], bcdbin[pce_cd_temp_play[1]], bcdbin[pce_cd_temp_play[0]], pce_cd_temp_play[3], bcdbin[pce_cd_temp_stop[2]], bcdbin[pce_cd_temp_stop[1]], bcdbin[pce_cd_temp_stop[0]], pce_cd_temp_stop[3], pce_cd_temp_stop[3] == 1);
#endif

			}
			break;
		case 0xde:
			// GET CD INFORMATION
#if ENABLE_TRACING_CD_2
			TRACE
				("CDRom2: Arg for 0xde command is %X, command count is %d\n",
				 io.cd_port_1801, pce_cd_cmdcnt);
#endif

			if (pce_cd_cmdcnt)
				pce_cd_temp_dirinfo[pce_cd_cmdcnt] = io.cd_port_1801;
			else {
				// We have received two arguments in pce_cd_temp_dirinfo
				// We can use only one
				// There's an argument indicating the kind of info we want
				// and an optional argument for track number
				pce_cd_temp_dirinfo[0] = io.cd_port_1801;

#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: I'll answer to 0xde command request\n");
				TRACE("CDRom2:   Arguments are:  %x, %x, %x, %x\n",
					  pce_cd_temp_dirinfo[0], pce_cd_temp_dirinfo[1],
					  pce_cd_temp_dirinfo[2], pce_cd_temp_dirinfo[3]);
#endif

				switch (pce_cd_temp_dirinfo[1]) {
				case 0:
					// We want info on number of first and last track
					switch (CD_emulation) {
					case 2:
					case 3:
					case 4:
						pce_cd_dirinfo[0] = binbcd[01];
						// Number of first track  (BCD)
						pce_cd_dirinfo[1] = binbcd[nb_max_track];
						// Number of last track (BCD)
						break;
					case 1:
						{
							int first_track, last_track;
							osd_cd_nb_tracks(&first_track, &last_track);
							TRACE
								("CDRom2: CD: first track %d, last track %d\n",
								 first_track, last_track);
							pce_cd_dirinfo[0] = binbcd[first_track];
							pce_cd_dirinfo[1] = binbcd[last_track];
						}
						break;
					case 5:
						pce_cd_dirinfo[0] = binbcd[HCD_first_track];
						pce_cd_dirinfo[1] = binbcd[HCD_last_track];
						TRACE
							("CDRom2: HCD: first track %d, last track %d\n",
							 HCD_first_track, HCD_last_track);
						break;
					}			// switch CD emulation
					cd_read_buffer = pce_cd_dirinfo;
					pce_cd_read_datacnt = 2;

#if ENABLE_TRACING_CD_2
					TRACE
						("CDRom2: Data resulting of 0xde request is %x and %x\n",
						 cd_read_buffer[0], cd_read_buffer[1]);
#endif
					break;
				case 2:
					// We want info on the track whose number is pce_cd_temp_dirinfo[0]
					switch (CD_emulation) {
					case 2:
					case 3:
					case 4:
					case 5:
						pce_cd_dirinfo[0]
							= CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].
							beg_min;
						pce_cd_dirinfo[1]
							= CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].
							beg_sec;
						pce_cd_dirinfo[2]
							= CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].
							beg_fra;
						pce_cd_dirinfo[3]
							= CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].
							type;
#if ENABLE_TRACING_CD
						TRACE("CDRom2: Type of track %d is %d\n",
							  bcdbin[pce_cd_temp_dirinfo[0]],
							  CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].
							  type);
#endif
						break;
					case 1:
						{
							int Min, Sec, Fra, Ctrl;
							osd_cd_track_info(bcdbin
											  [pce_cd_temp_dirinfo[0]],
											  &Min, &Sec, &Fra, &Ctrl);

							pce_cd_dirinfo[0] = binbcd[Min];
							pce_cd_dirinfo[1] = binbcd[Sec];
							pce_cd_dirinfo[2] = binbcd[Fra];
							pce_cd_dirinfo[3] = Ctrl;

#if ENABLE_TRACING_CD
							TRACE("CDRom2: Track #%d length - Min: %d,"
								  " Sec: %d, Frames: %d, Control: 0x%X\n",
								  bcdbin[pce_cd_temp_dirinfo[0]],
								  bcdbin[pce_cd_dirinfo[0]],
								  bcdbin[pce_cd_dirinfo[1]],
								  bcdbin[pce_cd_dirinfo[2]],
								  pce_cd_dirinfo[3]);
#endif
							break;

						}		// case CD emulation = 1

					}			// switch CD emulation

					pce_cd_read_datacnt = 3;
					cd_read_buffer = pce_cd_dirinfo;
					break;

				case 1:
					switch (CD_emulation) {
					case 1:
						{
							int min, sec, fra;

							osd_cd_length(&min, &sec, &fra);

#if ENABLE_TRACING_CD
							TRACE("CDRom2: CD Length - "
								  "Min: %d, Sec: %d, Frames: %d\n",
								  min, sec, fra);
#endif

							pce_cd_dirinfo[0] = binbcd[min];
							pce_cd_dirinfo[1] = binbcd[sec];
							pce_cd_dirinfo[2] = binbcd[fra];

							break;
						}		// case Cd emulation = 1
					default:
						pce_cd_dirinfo[0] = 0x25;
						pce_cd_dirinfo[1] = 0x06;
						pce_cd_dirinfo[2] = 0x00;
					}			// switch CD emulation

					pce_cd_read_datacnt = 3;
					cd_read_buffer = pce_cd_dirinfo;
					break;
				}				// switch command of request 0xde
			}					// end if of request 0xde (receiving command or executing them)
		}						// switch of request
	} else {					// end if of command arg or new request
		// it's a command ID we're receiving

#if ENABLE_TRACING_CD_2
		TRACE("CDRom2: Command byte received: 0x%02x.\n", io.cd_port_1801);
#endif

		switch (io.cd_port_1801) {
		case 0x00:
			// TEST UNIT READY
			io.cd_port_1800 = 0xD8;
			break;
		case 0x08:
			// READ
			pce_cd_curcmd = io.cd_port_1801;
			pce_cd_cmdcnt = 4;
			break;
		case 0xD8:
			// PLAY
			pce_cd_curcmd = io.cd_port_1801;
			pce_cd_cmdcnt = 4;
			break;
		case 0xD9:
			// PLAY
			pce_cd_curcmd = io.cd_port_1801;
			pce_cd_cmdcnt = 4;
			break;
		case 0xDA:
			// PAUSE
			pce_cd_curcmd = io.cd_port_1801;
			pce_cd_cmdcnt = 0;
			if (CD_emulation == 1)
				osd_cd_stop_audio();
			else if (CD_emulation == 5)
				HCD_pause_playing();
			break;
		case 0xDE:
			/* Get CD directory info */
			/* First arg is command? */
			/* Second arg is track? */
			io.cd_port_1800 = 0xd0;
			pce_cd_cmdcnt = 2;
			pce_cd_read_datacnt = 3;	/* 4 bytes */
			pce_cd_curcmd = io.cd_port_1801;
			break;
		}
	}
}


uchar
pce_cd_handle_read_1800(uint16 A)
{

	switch (A & 15) {
	case 0:
		return io.cd_port_1800;
	case 1:
		{
			uchar retval;

			if (cd_read_buffer) {
				retval = *cd_read_buffer++;
				if (pce_cd_read_datacnt == 2048) {
					pce_cd_read_datacnt--;
#if ENABLE_TRACING_CD
					TRACE("CDRom2: Data count error\n");
#endif
				}
				if (!pce_cd_read_datacnt)
					cd_read_buffer = 0;
			} else
				retval = 0;

			return retval;
		}

	case 2:
		return io.cd_port_1802;

	case 3:
		{
			static uchar tmp_res = 0x02;

			tmp_res = 0x02 - tmp_res;

			io.backup = DISABLE;

			/* TEST */// return 0x20;

			return tmp_res | 0x20;
		}

		/* TEST */
	case 4:
		return io.cd_port_1804;

		/* TEST */
	case 5:
		return 0x50;

		/* TEST */
	case 6:
		return 0x05;

	case 0x0A:
#if ENABLE_TRACING_CD
		Log("HARD : Read %x from ADPCM[%04X] to VRAM : 0X%04X\n",
			PCM[io.adpcm_rptr], io.adpcm_rptr, IO_VDC_00_MAWR.W * 2);
#endif

		if (!io.adpcm_firstread)
			return PCM[io.adpcm_rptr++];
		else {
			io.adpcm_firstread--;
			return NODATA;
		}

	case 0x0B:					/* TEST */
		return 0x00;
	case 0x0C:					/* TEST */
		return 0x01;			// 0x89
	case 0x0D:					/* TEST */
		return 0x00;

	case 8:
		if (pce_cd_read_datacnt) {
			uchar retval;

			if (cd_read_buffer)
				retval = *cd_read_buffer++;
			else
				retval = 0;

			if (!--pce_cd_read_datacnt) {
				cd_read_buffer = 0;
				if (!--cd_sectorcnt) {
#if ENABLE_TRACING_CD
					TRACE("CDRom2: Sector data count over.\n");
#endif
					io.cd_port_1800 |= 0x10;
					pce_cd_curcmd = 0;
				} else {
#if ENABLE_TRACING_CD
					// not really needed unless troubleshooting sector reading
					TRACE("CDRom2: Sector data left count: %d\n",
						  cd_sectorcnt);
#endif
					pce_cd_read_sector();
				}
			}
			return retval;
		}
		break;
	}

	// FIXME: what to return here?
#if ENABLE_TRACING_CD
	TRACE("CDRom2: FIXME: %s: A is not known at 0x%X\n");
#endif
	return 0;
}


void
pce_cd_handle_write_1800(uint16 A, uchar V)
{
	switch (A & 15) {
	case 0:
		// $1800 - CDC status
		if (V == 0x81)
			io.cd_port_1800 = 0xD0;
		return;
	case 1:
		// $1801 - CDC command / status / data
		io.cd_port_1801 = V;
		if (!pce_cd_cmdcnt)
			switch (V) {
			case 0:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: RESET? command at 1801\n");
#endif
				io.cd_port_1800 = 0x40;
				return;
			case 3:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: GET SYSTEM STATUS? command at 1801\n");
#endif
				return;
			case 8:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: READ SECTOR command at 1801\n");
#endif
				return;
			case 0x81:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: ANOTHER RESET? command at 1801\n");
#endif
				io.cd_port_1800 = 0x40;
				return;
			case 0xD8:
			case 0xD9:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: PLAY AUDIO? command at 1801\n");
#endif
				return;
			case 0xDA:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: PAUSE AUDIO PLAYING? command at 1801\n");
#endif
				return;
			case 0xDD:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: READ Q CHANNEL? command at 1801\n");
#endif
				return;
			case 0xDE:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: GET DIRECTORY INFO? command at 1801\n");
#endif
				return;
			default:
#if ENABLE_TRACING_CD_2
				TRACE("CDRom2: ERROR, unknown command %x at 1801\n", V);
#endif
				return;
			}
		return;
	case 2:
		// $1802 - ADPCM / CD control
		if ((!(io.cd_port_1802 & 0x80)) && (V & 0x80)) {
			io.cd_port_1800 &= ~0x40;
		} else if ((io.cd_port_1802 & 0x80) && (!(V & 0x80))) {
			io.cd_port_1800 |= 0x40;

#if ENABLE_TRACING_CD_2
			TRACE("CDRom2: ADPCM trans done = %d\n",
				  pce_cd_adpcm_trans_done);
#endif
			if (pce_cd_adpcm_trans_done) {
#if ENABLE_TRACING_CD
				TRACE("CDRom2: acknowledge DMA transfer for ADPCM data\n");
#endif
				io.cd_port_1800 |= 0x10;
				pce_cd_curcmd = 0x00;
				pce_cd_adpcm_trans_done = 0;
			}

			if (io.cd_port_1800 & 0x08) {
				if (io.cd_port_1800 & 0x20) {
					io.cd_port_1800 &= ~0x80;
				} else if (!pce_cd_read_datacnt) {
					if (pce_cd_curcmd == 0x08) {
						if (!--cd_sectorcnt) {
#if ENABLE_TRACING_CD
							TRACE("CDRom2: Sector data count over.\n");
#endif
							io.cd_port_1800 |= 0x10;	/* wrong */
							pce_cd_curcmd = 0x00;
						} else {
#if ENABLE_TRACING_CD
							TRACE("CDRom2: Sector data count: %d\n",
								  cd_sectorcnt);
#endif
							pce_cd_read_sector();
						}
					} else {
						if (io.cd_port_1800 & 0x10) {
							io.cd_port_1800 |= 0x20;
						} else {
							io.cd_port_1800 |= 0x10;
						}
					}
				} else {
					pce_cd_read_datacnt--;
				}
			} else {
				pce_cd_handle_command();
			}
		}

		io.cd_port_1802 = V;
		return;
	case 3:
		// TODO $1803 - BRAM lock / CD status
		return;
	case 4:
		// $1804 - CD reset
		if (V & 2) {
			// Reset asked, for now do nothing
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Reset mode for CD asked\n");
#endif

			switch (CD_emulation) {
			case 1:
				if (osd_cd_init(ISO_filename) != 0) {
					MESSAGE_ERROR("CDRom2: "
								  "CD Drive couldn't be initialised\n");
					exit(4);
				}
				break;
			case 2:
			case 3:
			case 4:
				fill_cd_info();
				break;
			case 5:
				fill_HCD_info(ISO_filename);
				break;
			}

			Wr6502(0x222D, 1);
			// This byte is set to 1 if a disc if present

			// cd_port_1800 &= ~0x40;
			io.cd_port_1804 = V;
		} else {
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Normal mode for CD asked\n");
#endif
			io.cd_port_1804 = V;
			// cd_port_1800 |= 0x40; // Maybe the previous reset is enough
			// cd_port_1800 |= 0xD0;
			// Indicates that the Hardware is ready after such a reset
		}
		return;

	case 5:
	case 6:
		// TODO $1805 - Convert PCM data / PCM data
		// TODO $1806 - PCM data
		return;

	case 7:
		// $1807 - BRAM unlock / CD status
		io.backup = ENABLE;
		return;

	case 8:
		// $1808 - ADPCM address (LSB) / CD data
		io.adpcm_ptr.B.l = V;
		return;

	case 9:
		// $1809 - ADPCM address (MSB)
		io.adpcm_ptr.B.h = V;
		return;

	case 0x0A:
		// $180A - ADPCM RAM data port
		PCM[io.adpcm_wptr++] = V;
#if ENABLE_TRACING_CD
		TRACE("CDRom2: Wrote %02X to ADPCM buffer[%04X]\n",
			  V, io.adpcm_wptr - 1);
#endif
		return;

	case 0x0B:
		// $180B - ADPCM DMA control
		if ((V & 2) && (!(cd_port_180b & 2))) {
			issue_ADPCM_dma();
			cd_port_180b = V;
			return;
		}

		/* TEST */
		if (!V) {
			io.cd_port_1800 &= ~0xF8;
			io.cd_port_1800 |= 0xD8;
		}
		cd_port_180b = V;
		return;

	case 0x0C:
		// TODO $180C - ADPCM status
		return;

	case 0x0D:
		// $180D - ADPCM address control
		if ((V & 0x03) == 0x03) {
			io.adpcm_dmaptr = io.adpcm_ptr.W;	// set DMA pointer
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Set DMA pointer to 0x%X\n", io.adpcm_dmaptr);
#endif
		}

		if (V & 0x04) {
			io.adpcm_wptr = io.adpcm_ptr.W;	// set write pointer
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Set write pointer to 0x%X\n", io.adpcm_wptr);
#endif
		}

		if (V & 0x08) {
			io.adpcm_rptr = io.adpcm_ptr.W;	// set read pointer
			io.adpcm_firstread = 2;
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Set read pointer to 0x%X\n", io.adpcm_rptr);
#endif
		}
		/* TEST
		   else { io.adpcm_rptr = io.adpcm_ptr.W; io.adpcm_firstread = 1; }
		 */
		/* TEST */
		// if (V & 0x08) io.

		if (V & 0x80) {
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Reset mode for ADPCM\n");
#endif
		} else {
#if ENABLE_TRACING_CD
			TRACE("CDRom2: Normal mode for ADPCM\n");
#endif
		}
		return;

	case 0xe:
		// $180E - ADPCM playback rate
		io.adpcm_rate = 32 / (16 - (V & 15));	// Set ADPCM playback rate
#if ENABLE_TRACING_CD
		TRACE("CDRom2: ADPCM rate set to %d kHz\n", io.adpcm_rate);
#endif
		return;

	case 0xf:
		// $180F - ADPCM and CD audio fade timer
		cd_fade = V;
#if ENABLE_TRACING_CD
		TRACE("CDRom2: Fade Setting to %d\n", V);
#endif
		return;
	}							// A&15 switch, i.e. CD ports
}
