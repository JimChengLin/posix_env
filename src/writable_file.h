#pragma once
#ifndef POSIX_ENV_WRITABLE_FILE_H
#define POSIX_ENV_WRITABLE_FILE_H

#include "env.h"

namespace penv {
    class PosixWritableFile : public WritableFile {
    private:
        enum {
            kPreallocationBlockSize = 4 * 1024 * 1024
        };

        std::string fname_;
        size_t filesize_;
        size_t last_preallocated_block_;
        int fd_;

    public:
        PosixWritableFile(std::string fname, size_t filesize, int fd)
                : fname_(std::move(fname)),
                  filesize_(filesize),
                  last_preallocated_block_(0),
                  fd_(fd) {}

        ~PosixWritableFile() override;

    public:
        void Write(const Slice & data) override;

        void Truncate(size_t n) override;

        void Flush() override;

        void Sync() override;

        size_t GetFileSize() const override {
            return filesize_;
        }

        void Hint(WriteLifeTimeHint hint) override;

        void RangeSync(size_t offset, size_t n) override;

        void PrepareWrite(size_t offset, size_t n) override;

        void Allocate(size_t offset, size_t n) override;
    };
}

#endif //POSIX_ENV_WRITABLE_FILE_H
