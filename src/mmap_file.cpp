#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "defs.h"
#include "mmap_file.h"

#define IO_EXCEPTION(f) std::runtime_error("IO:" + PENV_EXCEPTION_INFO + " | " + strerror(errno) + " | " + (f))

namespace penv {
    PosixMmapFile::~PosixMmapFile() {
        munmap(base_, len_);
        close(fd_);
    }

    void PosixMmapFile::Resize(size_t n) {
        int r;
#if defined(PENV_OS_MACOSX)
        r = ftruncate(fd_, static_cast<off_t>(n));
#else
        r = fallocate(fd_, 0, 0, static_cast<off_t>(n));
#endif
        if (r != 0) {
            throw IO_EXCEPTION(fname_);
        }

#if defined(PENV_OS_MACOSX)
        if (munmap(base_, len_) != 0) {
            throw IO_EXCEPTION(fname_);
        }
        base_ = mmap(nullptr, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
#else
        base_ = mremap(base_, len_, n, MREMAP_MAYMOVE);
#endif
        len_ = n;
        if (base_ == MAP_FAILED) {
            throw IO_EXCEPTION(fname_);
        }
    }

    void PosixMmapFile::Hint(AccessPattern hint) {
        switch (hint) {
            case NORMAL:
                posix_madvise(base_, len_, POSIX_MADV_NORMAL);
                break;
            case SEQUENTIAL:
                posix_madvise(base_, len_, POSIX_MADV_SEQUENTIAL);
                break;
            case RANDOM:
                posix_madvise(base_, len_, POSIX_MADV_RANDOM);
                break;
            case WILLNEED:
                posix_madvise(base_, len_, POSIX_MADV_WILLNEED);
                break;
            default:
                assert(hint == DONTNEED);
                posix_madvise(base_, len_, POSIX_MADV_DONTNEED);
                break;
        }
    }
}