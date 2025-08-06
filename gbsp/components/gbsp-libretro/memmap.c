
#include <stdint.h>
#include <stdbool.h>

#include "memmap.h"

// The JIT cache buffer is allocated via mmap (or win equivalent) so that it
// can be RWX. On top of that, we need the bufer to be "close" to the text
// segment, so that we can perform jumps between the two code blocks.
// Android and some other platforms discourage the usage of sections in the
// binary (ie. on-disk ELF) that are marked as executable and writtable for
// security reasons. Therefore we prefer to use mmap even though it can be
// tricky to map correctly.

// To map a block close to the code, we take the function address as a proxy
// of the text section address, and try to map the cache next to it. This is
// an iterative process of trial and error that is hopefully successful.

// Jump/Call offset requirements:
// x86-64 has a +/- 2GB offset requirement.
// ARM64 has a +/-128MB offset requirement.
// ARM32 has a +/- 32MB offset requirement (gpsp does not require this).
// MIPS requires blocks to be within the same 256MB boundary (identical 4 MSB)

#ifdef MMAP_JIT_CACHE

// JIT block requirements translated to allocation code.
#if defined(MIPS_ARCH)
  #define _MAP_ITERATIONS           1024   // Test -/+2GB in 4MB steps
  #define _MAP_STEP         (4*1024*1024)
  #define _VALIDATE_BLOCK_FN(ptr, size) \
          validate_addr_section_mips(ptr, size)
#elif defined(ARM64_ARCH)
  #define _MAP_ITERATIONS            256   // Test -/+128MB in 1MB steps
  #define _MAP_STEP           (1024*1024)
  #define _VALIDATE_BLOCK_FN(ptr, size) \
          validate_addr_offset(ptr, size, 128)
#else
  #define _MAP_ITERATIONS           1024   // Test -/+2GB in 4MB steps
  #define _MAP_STEP         (4*1024*1024)
  #define _VALIDATE_BLOCK_FN(ptr, size) \
          validate_addr_offset(ptr, size, 2048)
#endif

bool validate_addr_offset(void *ptr, unsigned size, unsigned max_offset_mb) {
	// Ensure that the start and the end of the block is not too far away.
	uintptr_t ref_addr = (uintptr_t)(map_jit_block);
	uintptr_t start_addr = (uintptr_t)ptr;
	uintptr_t end_addr = start_addr + size;
	uintptr_t dist1 = start_addr > ref_addr ? start_addr - ref_addr
	                                        : ref_addr - start_addr;
	uintptr_t dist2 = end_addr > ref_addr ? end_addr - ref_addr
	                                      : ref_addr - end_addr;

	return dist1 < max_offset_mb * 1024 * 1024 &&
	       dist2 < max_offset_mb * 1024 * 1024;
}

bool validate_addr_section_mips(void *ptr, unsigned size, unsigned max_offset_mb) {
	uintptr_t ref_addr = (uintptr_t)(map_jit_block);
	uintptr_t start_addr = (uintptr_t)ptr;
	uintptr_t end_addr = start_addr + size;
	const uintptr_t msk = ~0xFFFFFFFU;  // 256MB block

	return (ref_addr & msk) == (start_addr & msk) &&
	       (ref_addr & msk) == (end_addr & msk);
}

#ifdef WIN32

	#include <windows.h>
	#include <io.h>

	void *map_jit_block(unsigned size) {
		unsigned i;
		uintptr_t base = (uintptr_t)(map_jit_block) & (~(_MAP_STEP - 1ULL));
		for (i = 0; i < _MAP_ITERATIONS; i++) {
			int offset = ((i & 1) ? 1 : -1) * (i >> 1) * _MAP_STEP;
			uintptr_t baddr = base + (intptr_t)offset;
			if (!baddr)
				continue;    // Do not map NULL, bad things happen :)

			void *p = VirtualAlloc((void*)baddr, size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (p) {
				if (_VALIDATE_BLOCK_FN(p, size))
					return p;

				VirtualFree(p, 0, MEM_RELEASE);
			}

		}
		return 0;
	}

	void unmap_jit_block(void *bufptr, unsigned size) {
		VirtualFree(bufptr, 0, MEM_RELEASE);
	}

#else

	#include <sys/mman.h>

	// Posix implementation
	void *map_jit_block(unsigned size) {
		unsigned i;
		uintptr_t base = (uintptr_t)(map_jit_block) & (~(_MAP_STEP - 1ULL));
		for (i = 0; i < _MAP_ITERATIONS; i++) {
			int offset = ((i & 1) ? 1 : -1) * (i >> 1) * _MAP_STEP;
			uintptr_t baddr = base + (intptr_t)offset;
			if (!baddr)
				continue;    // Do not map NULL, bad things happen :)

			void *p = mmap((void*)baddr, size, PROT_READ|PROT_WRITE|PROT_EXEC,
			                                   MAP_ANON|MAP_PRIVATE, -1, 0);
			if (p) {
				if (_VALIDATE_BLOCK_FN(p, size))
					return p;

				munmap(p, size);
			}
		}
		return 0;
	}

	void unmap_jit_block(void *bufptr, unsigned size) {
		munmap(bufptr, size);
	}

#endif /* WIN32 */

#endif /* MMAP_JIT_CACHE */

