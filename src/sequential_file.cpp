#include <cerrno>

#include "defs.h"
#include "sequential_file.h"

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
                // We leave status as ok if we hit the end of the file
                // We also clear the error so that the reads can continue
                // if a new data is written to the file
                clearerr(file_);
            } else {
                // A partial read with an error
                throw std::runtime_error("IO: While read file sequentially "
                                         + fname_ + ' '
                                         + std::to_string(errno));
            }
        }
    }

    void PosixSequentialFile::Skip(size_t n) {
        if (fseek(file_, static_cast<long int>(n), SEEK_CUR)) {
            throw std::runtime_error("IO: While fseek to skip " + std::to_string(n) + " bytes "
                                     + fname_ + ' '
                                     + std::to_string(errno));
        }
    }
}