#include <cerrno>

#include "defs.h"
#include "sequential_file.h"

#define IO_EXCEPTION(f) std::runtime_error("IO:" + PENV_EXCEPTION_INFO + " | " + strerror(errno) + " | " + (f))

namespace penv {
    PosixSequentialFile::~PosixSequentialFile() {
        fclose(file_);
    }

    void PosixSequentialFile::Read(size_t n, char * scratch) {
        size_t r = 0;
        do {
            r = fread_unlocked(scratch, 1, n, file_);
        } while (r == 0 && ferror(file_) && errno == EINTR);
        if (r < n) {
            if (feof(file_)) {
                clearerr(file_);
            } else {
                throw IO_EXCEPTION(fname_);
            }
        }
    }

    void PosixSequentialFile::Skip(size_t n) {
        if (fseek(file_, static_cast<long int>(n), SEEK_CUR)) {
            throw IO_EXCEPTION(fname_);
        }
    }
}