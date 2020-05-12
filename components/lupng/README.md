LuPng
=====
A small PNG decoder/encoder lib extracted from my game engine, Lunacy. It was designed for minimal state overhead and to exclusively use read/write callbacks to make it easier to use PNG data from various sources.

Usage
-----
See example.c

Building
--------
LuPng uses the zlib API for compression and decompression. For convenience this repo also contains miniz, a public domain implementation of the deflate algorithm that exposes the zlib API. However if your project already uses zlib you can tell LuPng to use that instead.

With zlib:
  - add lupng.c to your project
  - define LUPNG_USE_ZLIB
  - link against zlib
  - Example: `cc example.c lupng.c -DLUPNG_USE_ZLIB -lz`

With miniz:
  - add lupng.c and miniz.c to your project
  - add a miniz header to your include path (there is one provided in the miniz/ directory)
  - Example: `cc example.c lupng.c miniz/miniz.c -Iminiz/`

License
-------
LuPng is licensed under the MIT license. See the file LICENSE for details.
Miniz is placed in the public domain by its author, Rich Geldreich.
