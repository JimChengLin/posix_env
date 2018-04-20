#pragma once
#ifndef POSIX_ENV_RANDOM_ACCESS_FILE_H
#define POSIX_ENV_RANDOM_ACCESS_FILE_H

#include "env.h"

namespace penv {
    class PosixRandomAccessFile : public RandomAccessFile {
    private:
        std::string fname_;
        int fd_;

    public:
        PosixRandomAccessFile(std::string fname, int fd)
                : fname_(std::move(fname)),
                  fd_(fd) {}

        ~PosixRandomAccessFile() override;

    public:
        void ReadAt(size_t offset, size_t n, char * scratch) const override;

        void Prefetch(size_t offset, size_t n) override;

        void Hint(AccessPattern hint) override;
    };
}

#endif //POSIX_ENV_RANDOM_ACCESS_FILE_H
