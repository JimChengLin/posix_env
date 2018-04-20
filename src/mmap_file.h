#pragma once
#ifndef POSIX_ENV_MMAP_FILE_H
#define POSIX_ENV_MMAP_FILE_H

#include "env.h"

namespace penv {
    class PosixMmapFile : public MmapFile {
    private:
        std::string fname_;
        void * base_;
        size_t len_;
        int fd_;

    public:
        PosixMmapFile(std::string fname, void * base, size_t len, int fd)
                : fname_(std::move(fname)),
                  base_(base),
                  len_(len),
                  fd_(fd) {}

        ~PosixMmapFile() override = default;

    public:
        void * Base() override {
            return base_;
        }

        const void * Base() const override {
            return base_;
        }

        size_t GetFileSize() const override {
            return len_;
        }

        void Resize(size_t n) override;

        void Hint(AccessPattern hint) override;
    };
}

#endif //POSIX_ENV_MMAP_FILE_H
