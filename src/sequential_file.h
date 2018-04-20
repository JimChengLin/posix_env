#pragma once
#ifndef POSIX_ENV_SEQUENTIAL_FILE_H
#define POSIX_ENV_SEQUENTIAL_FILE_H

#include "env.h"

namespace penv {
    class PosixSequentialFile : public SequentialFile {
    private:
        std::string fname_;
        FILE * file_;

    public:
        PosixSequentialFile(std::string fname, FILE * file)
                : fname_(std::move(fname)),
                  file_(file) {}

        ~PosixSequentialFile() override;

    public:
        void Read(size_t n, char * scratch) override;

        void Skip(size_t n) override;
    };
}

#endif //POSIX_ENV_SEQUENTIAL_FILE_H
