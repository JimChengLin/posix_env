#pragma once
#ifndef POSIX_ENV_WRITABLE_FILE_H
#define POSIX_ENV_WRITABLE_FILE_H

#include "env.h"

namespace penv {
    class PosixWritableFile : public WritableFile {
    public:
        ~PosixWritableFile() override = default;

    public:
        void Write(const Slice & data) override;

        void WriteAt(size_t offset, const Slice & data) override;

        void Truncate(size_t n) override;

        void Flush() override;

        void Sync() override;

        size_t GetFileSize() const override;

        void Hint(WriteLifeTimeHint hint) override;

        void RangeSync(size_t offset, size_t n) override;

        void PrepareWrite(size_t offset, size_t n) override;

        void Allocate(size_t offset, size_t n) override;
    };
}

#endif //POSIX_ENV_WRITABLE_FILE_H
