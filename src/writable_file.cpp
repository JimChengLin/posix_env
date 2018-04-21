#include "writable_file.h"

namespace penv {
    PosixWritableFile::~PosixWritableFile() {

    }

    void PosixWritableFile::Write(const Slice & data) {

    }

    void PosixWritableFile::WriteAt(size_t offset, const Slice & data) {

    }

    void PosixWritableFile::Truncate(size_t n) {

    }

    void PosixWritableFile::Flush() {

    }

    void PosixWritableFile::Sync() {

    }

    size_t PosixWritableFile::GetFileSize() const {

    }

    void PosixWritableFile::Hint(WriteLifeTimeHint hint) {

    }

    void PosixWritableFile::RangeSync(size_t offset, size_t n) {

    }

    void PosixWritableFile::PrepareWrite(size_t offset, size_t n) {

    }

    void PosixWritableFile::Allocate(size_t offset, size_t n) {

    }
}