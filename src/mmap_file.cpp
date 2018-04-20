#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>

#include "defs.h"
#include "mmap_file.h"

// For non linux platform, the following macros are used only as place holder.
#if !defined(PENV_OS_LINUX)
#define POSIX_MADV_NORMAL 0     /* [MC1] no further special treatment */
#define POSIX_MADV_RANDOM 1     /* [MC1] expect random page refs */
#define POSIX_MADV_SEQUENTIAL 2 /* [MC1] expect sequential page refs */
#define POSIX_MADV_WILLNEED 3   /* [MC1] will need these pages */
#define POSIX_MADV_DONTNEED 4   /* [MC1] don't need these pages */
#endif

namespace penv {
    void PosixMmapFile::Resize(size_t n) {
        if (ftruncate(fd_, static_cast<off_t>(n)) != 0) {
            throw std::runtime_error("IO: While ftruncate to " + std::to_string(n) + " bytes "
                                     + fname_ + ' '
                                     + std::to_string(errno));
        }
#if defined(PENV_OS_LINUX)
        base_ = mremap(base_, len_, n, MREMAP_MAYMOVE);
#else
        if (munmap(base_, len_) != 0) {
            throw std::runtime_error("IO: While munmap "
                                     + fname_ + ' '
                                     + std::to_string(errno));
        }
        base_ = mmap(nullptr, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
#endif
        if (base_ == MAP_FAILED) {
            throw std::runtime_error("IO: While mmap "
                                     + fname_ + ' '
                                     + std::to_string(errno));
        }
        len_ = n;
    }

    inline static int Madvise(void * addr, size_t len, int advice) {
#if defined(PENV_OS_LINUX)
        return posix_madvise(addr, len, advice);
#else
        return 0;
#endif
    }

    void PosixMmapFile::Hint(AccessPattern hint) {
        switch (hint) {
            case NORMAL:
                Madvise(base_, len_, POSIX_MADV_NORMAL);
                break;
            case RANDOM:
                Madvise(base_, len_, POSIX_MADV_RANDOM);
                break;
            case SEQUENTIAL:
                Madvise(base_, len_, POSIX_MADV_SEQUENTIAL);
                break;
            case WILLNEED:
                Madvise(base_, len_, POSIX_MADV_WILLNEED);
                break;
            default:
                assert(hint == DONTNEED);
                Madvise(base_, len_, POSIX_MADV_DONTNEED);
                break;
        }
    }
}