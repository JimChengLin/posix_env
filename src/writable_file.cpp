#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "writable_file.h"

#define IO_EXCEPTION(f) std::runtime_error("IO:" + PENV_EXCEPTION_INFO + " | " + strerror(errno) + " | " + (f))

namespace penv {
    PosixWritableFile::~PosixWritableFile() {
        if (last_preallocated_block_ > 0) {
            ftruncate(fd_, static_cast<off_t>(filesize_));
#if defined(PENV_OS_LINUX)
            struct stat sbuf;
            int r = fstat(fd_, &sbuf);
            if (r == 0 &&
                (sbuf.st_size + sbuf.st_blksize - 1) / sbuf.st_blksize != sbuf.st_blocks / (sbuf.st_blksize / 512)) {
                fallocate(fd_, FALLOC_FL_KEEP_SIZE | FALLOC_FL_PUNCH_HOLE, filesize_,
                          kPreallocationBlockSize * last_preallocated_block_ - filesize_);
            }
#endif
        }
        close(fd_);
    }

    void PosixWritableFile::Write(const Slice & data) {
        size_t left = data.size();
        const char * src = data.data();
        while (left != 0) {
            ssize_t done = write(fd_, src, left);
            if (done < 0) {
                if (errno == EINTR) {
                    continue;
                }
                throw IO_EXCEPTION(fname_);
            }
            left -= done;
            src += done;
        }
        filesize_ += data.size();
    }

    void PosixWritableFile::Truncate(size_t n) {
        int r = ftruncate(fd_, static_cast<off_t>(n));
        if (r != 0) {
            throw IO_EXCEPTION(fname_);
        } else {
            filesize_ = n;
        }
    }

    void PosixWritableFile::Sync() {
        if (fsync(fd_) != 0) {
            throw IO_EXCEPTION(fname_);
        }
    }

    void PosixWritableFile::Hint(WriteLifeTimeHint hint) {
#if defined(PENV_OS_LINUX) && defined(F_SET_RW_HINT)
        fcntl(fd_, F_SET_RW_HINT, &hint);
#endif
    }

    void PosixWritableFile::RangeSync(size_t offset, size_t n) {
#if defined(PENV_OS_LINUX)
        int r = sync_file_range(fd_, static_cast<off_t>(offset), static_cast<off_t>(n), SYNC_FILE_RANGE_WRITE);
        if (r != 0) {
            throw IO_EXCEPTION(fname_);
        }
#endif
    }

    void PosixWritableFile::PrepareWrite(size_t offset, size_t n) {
        size_t new_last_preallocated_block = (offset + n + kPreallocationBlockSize - 1) / kPreallocationBlockSize;
        if (new_last_preallocated_block > last_preallocated_block_) {
            Allocate(kPreallocationBlockSize * last_preallocated_block_,
                     kPreallocationBlockSize * (new_last_preallocated_block - last_preallocated_block_));
            last_preallocated_block_ = new_last_preallocated_block;
        }
    }

    void PosixWritableFile::Allocate(size_t offset, size_t n) {
#if defined(PENV_OS_LINUX)
        int r = fallocate(fd_, 0, static_cast<off_t>(offset), static_cast<off_t>(n));
        if (r != 0) {
            throw IO_EXCEPTION(fname_);
        }
#endif
    }
}