#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "defs.h"
#include "random_access_file.h"

#define IO_EXCEPTION(f) std::runtime_error("IO:" + PENV_EXCEPTION_INFO + " | " + strerror(errno) + " | " + (f))

#if !defined(POSIX_FADV_NORMAL)
#define POSIX_FADV_NORMAL
#define POSIX_FADV_SEQUENTIAL
#define POSIX_FADV_RANDOM
#define POSIX_FADV_NOREUSE
#define POSIX_FADV_WILLNEED
#define POSIX_FADV_DONTNEED

#define posix_fadvise(fd, offset, len, advice) 0
#endif

namespace penv {
    PosixRandomAccessFile::~PosixRandomAccessFile() {
        close(fd_);
    }

    void PosixRandomAccessFile::ReadAt(size_t offset, size_t n, char * scratch) const {
        ssize_t r = -1;
        size_t left = n;
        char * ptr = scratch;
        while (left > 0) {
            r = pread(fd_, ptr, left, static_cast<off_t>(offset));
            if (r <= 0) {
                if (r == -1 && errno == EINTR) {
                    continue;
                }
                break;
            }
            ptr += r;
            offset += r;
            left -= r;
        }
        if (r < 0) {
            throw IO_EXCEPTION(fname_);
        }
    }

    void PosixRandomAccessFile::Prefetch(size_t offset, size_t n) {
        ssize_t r = 0;
#if defined(PENV_OS_LINUX)
        r = readahead(fd_, offset, n);
#endif
#if defined(PENV_OS_MACOSX)
        radvisory advice = {static_cast<off_t>(offset),
                            static_cast<int>(n)};
        r = fcntl(fd_, F_RDADVISE, &advice);
#endif
        if (r == -1) {
            throw IO_EXCEPTION(fname_);
        }
    }

    void PosixRandomAccessFile::Hint(AccessPattern hint) {
        switch (hint) {
            case NORMAL:
                posix_fadvise(fd_, 0, 0, POSIX_FADV_NORMAL);
                break;
            case SEQUENTIAL:
                posix_fadvise(fd_, 0, 0, POSIX_FADV_SEQUENTIAL);
                break;
            case RANDOM:
                posix_fadvise(fd_, 0, 0, POSIX_FADV_RANDOM);
                break;
            case NOREUSE:
                posix_fadvise(fd_, 0, 0, POSIX_FADV_NOREUSE);
                break;
            case WILLNEED:
                posix_fadvise(fd_, 0, 0, POSIX_FADV_WILLNEED);
                break;
            default:
                assert(hint == DONTNEED);
                posix_fadvise(fd_, 0, 0, POSIX_FADV_DONTNEED);
                break;
        }
    }
}