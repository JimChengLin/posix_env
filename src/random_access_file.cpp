#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "defs.h"
#include "random_access_file.h"

// For non linux platform, the following macros are used only as place holder.
#if !defined(PENV_OS_LINUX)
#define POSIX_FADV_NORMAL 0     /* [MC1] no further special treatment */
#define POSIX_FADV_RANDOM 1     /* [MC1] expect random page refs */
#define POSIX_FADV_SEQUENTIAL 2 /* [MC1] expect sequential page refs */
#define POSIX_FADV_WILLNEED 3   /* [MC1] will need these pages */
#define POSIX_FADV_DONTNEED 4   /* [MC1] don't need these pages */
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
            throw std::runtime_error("IO: While pread offset " + std::to_string(offset)
                                     + " len " + std::to_string(n) + ' '
                                     + fname_ + ' '
                                     + std::to_string(errno));
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
            throw std::runtime_error("IO: While prefetch offset " + std::to_string(offset)
                                     + " len " + std::to_string(n) + ' '
                                     + fname_ + ' '
                                     + std::to_string(errno));
        }
    }

    // A wrapper for fadvise, if the platform doesn't support fadvise,
    // it will simply return 0.
    inline static int Fadvise(int fd, off_t offset, size_t len, int advice) {
#if defined(PENV_OS_LINUX)
        return posix_fadvise(fd, offset, len, advice);
#else
        return 0;
#endif
    }

    void PosixRandomAccessFile::Hint(AccessPattern hint) {
        switch (hint) {
            case NORMAL:
                Fadvise(fd_, 0, 0, POSIX_FADV_NORMAL);
                break;
            case RANDOM:
                Fadvise(fd_, 0, 0, POSIX_FADV_RANDOM);
                break;
            case SEQUENTIAL:
                Fadvise(fd_, 0, 0, POSIX_FADV_SEQUENTIAL);
                break;
            case WILLNEED:
                Fadvise(fd_, 0, 0, POSIX_FADV_WILLNEED);
                break;
            default:
                assert(hint == DONTNEED);
                Fadvise(fd_, 0, 0, POSIX_FADV_DONTNEED);
                break;
        }
    }
}