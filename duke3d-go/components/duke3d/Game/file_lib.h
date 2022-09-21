//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#ifndef _file_lib_public
#define _file_lib_public
#ifdef __cplusplus
extern "C" {
#endif

enum
   {
   filetype_binary,
   filetype_text
   };

enum
   {
   access_read,
   access_write,
   access_append
   };

//==========================================================================
//
// SafeOpenWrite - Opens a file for writing, returns handle
//
//==========================================================================
int32 SafeOpenWrite ( const char  * filename, int32 filetype );

//==========================================================================
//
// SafeOpenRead - Opens a file for reading, returns handle
//
//==========================================================================
int32 SafeOpenRead ( const char  * filename, int32 filetype );

//==========================================================================
//
// SafeOpenAppend - Opens a file for appending, returns handle
//
//==========================================================================
int32 SafeOpenAppend ( const char  * filename, int32 filetype );

//==========================================================================
//
// SafeClose - Close a file denoted by the file handle
//
//==========================================================================
void SafeClose ( int32 handle );

//==========================================================================
//
// SafeFileExists - Checks for existence of file
//
//==========================================================================
boolean SafeFileExists ( const char  * filename );

//==========================================================================
//
// SafeFileLength - Get length of a file pointed to by handle
//
//==========================================================================
int32 SafeFileLength ( int32 handle );

//==========================================================================
//
// SafeRead - reads from a handle
//
//            handle - handle of file to read from
//
//            buffer - pointer of buffer to read into
//
//            count  - number of bytes to read
//
//==========================================================================
void SafeRead (int32 handle, void *buffer, int32 count);

//==========================================================================
//
// SafeWrite - writes to a handle
//
//             handle - handle of file to write to
//
//             buffer - pointer of buffer to write from
//
//             count  - number of bytes to write
//
//==========================================================================
void SafeWrite (int32 handle, void *buffer, int32 count);

//==========================================================================
//
// LoadFile - Load a file
//
//            filename - name of file
//
//            bufferptr - pointer to pointer of buffer to read into
//
//            returns number of bytes read
//
//==========================================================================
int32 LoadFile ( const uint8_t  * filename, void ** bufferptr );

//==========================================================================
//
// SaveFile - Save a file
//
//            filename - name of file
//
//            bufferptr - pointer to buffer to write from
//
//            count - number of bytes to write
//
//==========================================================================
void SaveFile ( const uint8_t  * filename, void * bufferptr, int32 count );

//==========================================================================
//
// GetPathFromEnvironment - Add a pathname described in an environment
//                          variable to a standard filename.
//
//                          fullname - final string containing entire path
//
//                          envname - string naming enivronment variable
//
//                          filename - standard filename
//
//==========================================================================
void GetPathFromEnvironment( uint8_t  *fullname, const uint8_t  *envname, const uint8_t  *filename );

//==========================================================================
//
// DefaultExtension - Add a default extension to a path
//
//                    path - a path
//
//                    extension - default extension should include '.'
//
//==========================================================================
void DefaultExtension (uint8_t  *path, const uint8_t  *extension);

//==========================================================================
//
// DefaultPath - Add the default path to a filename if it doesn't have one
//
//               path - filename
//
//               extension - default path
//
//==========================================================================
void DefaultPath (uint8_t  *path, const uint8_t  *basepath);

//==========================================================================
//
// ExtractFileBase - Extract the base filename from a path
//
//                   path - the path
//
//                   dest - where the file base name will be placed
//
//==========================================================================
void ExtractFileBase (uint8_t  *path, uint8_t  *dest);

//==========================================================================
//
// GetExtension - Extract the extension from a name
//                returns true if an extension is found
//                returns false otherwise
//
//==========================================================================
boolean GetExtension( uint8_t  *filename, uint8_t  *extension );

//==========================================================================
//
// SetExtension - Sets the extension from a name.  Assumes that enough
// 					space is left at the end of the string to hold an extension.
//
//==========================================================================
void SetExtension( uint8_t  *filename, const uint8_t  *extension );

#ifdef __MSDOS__
//******************************************************************************
//
// GetPath
//
// Purpose
//    To parse the directory entered by the user to make the directory.
//
// Parms
//    Path - the path to be parsed.
//
// Returns
//    Pointer to next path
//
//******************************************************************************
uint8_t  * GetPath (uint8_t  * path, uint8_t  *dir);

//******************************************************************************
//
// ChangeDirectory ()
//
// Purpose
//    To change to a directory.  Checks for drive changes.
//
// Parms
//    path - The path to change to.
//
// Returns
//    TRUE  - If successful.
//    FALSE - If unsuccessful.
//
//******************************************************************************
boolean ChangeDirectory (uint8_t  * path);

//******************************************************************************
//
// ChangeDrive ()
//
// Purpose
//    To change drives.
//
// Parms
//    drive - The drive to change to.
//
// Returns
//    TRUE  - If drive change successful.
//    FALSE - If drive change unsuccessful.
//
//******************************************************************************
boolean ChangeDrive (uint8_t  *drive);

#endif

#ifdef __cplusplus
};
#endif
#endif
