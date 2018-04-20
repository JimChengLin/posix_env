#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "env.h"
#include "random_access_file.h"
#include "sequential_file.h"

namespace penv {
    class PosixEnv : public Env {
    public:
        ~PosixEnv() override = default;

    public:
        size_t GetFileSize(const std::string & fname) override {
            struct stat sbuf;
            if (stat(fname.c_str(), &sbuf) != 0) {
                throw std::runtime_error("while stat a file for size");
            } else {
                return static_cast<size_t>(sbuf.st_size);
            }
        }

    public:
        void OpenSequentialFile(const std::string & fname,
                                std::unique_ptr<SequentialFile> * result) override {
            int fd;
            int flags = O_RDONLY;
            FILE * file = nullptr;

            do {
                fd = open(fname.c_str(), flags, 0644 /* 权限 */);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw std::runtime_error("IO: While open a file for sequentially reading "
                                         + fname + ' '
                                         + std::to_string(errno));
            }

            do {
                file = fdopen(fd, "r");
            } while (file == nullptr && errno == EINTR);
            if (file == nullptr) {
                close(fd);
                throw std::runtime_error("IO: While fdopen a file for sequentially reading "
                                         + fname + ' '
                                         + std::to_string(errno));
            }
            *result = std::make_unique<PosixSequentialFile>(fname, file);
        }

        void OpenRandomAccessFie(const std::string & fname,
                                 std::unique_ptr<RandomAccessFile> * result) override {
            int fd;
            int flags = O_RDONLY;

            do {
                fd = open(fname.c_str(), flags, 0644 /* 权限 */);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw std::runtime_error("IO: While open a file for randomly reading "
                                         + fname + ' '
                                         + std::to_string(errno));
            }
            *result = std::make_unique<PosixRandomAccessFile>(fname, fd);
        }

        static void OpenWritableFile(const std::string & fname,
                                     std::unique_ptr<WritableFile> * result,
                                     bool reopen) {

        }

        void OpenWritableFile(const std::string & fname,
                              std::unique_ptr<WritableFile> * result) override {
            return OpenWritableFile(fname, result, false);
        }

        void ReopenWritableFile(const std::string & fname,
                                std::unique_ptr<WritableFile> * result) override {
            return OpenWritableFile(fname, result, true);
        }

        static void OpenMmapFile(const std::string & fname,
                                 std::unique_ptr<MmapFile> * result,
                                 size_t size,
                                 bool reopen) {
            int fd;
            int flags = (reopen) ? (O_CREAT | O_APPEND) : (O_CREAT | O_TRUNC);
            flags |= O_RDWR;

            do {
                fd = open(fname.c_str(), flags, 0644 /* 权限 */);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw std::runtime_error("IO: While open a file for appending "
                                         + fname + ' '
                                         + std::to_string(errno));
            }

            void * base = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            *result = std::make_unique<MmapFile>(fname, base, size, fd);
        }

        void OpenMmapFile(const std::string & fname,
                          std::unique_ptr<MmapFile> * result) override {
            return OpenMmapFile(fname, result, GetFileSize(fname), false);
        }

        void ReopenMmapFile(const std::string & fname,
                            std::unique_ptr<MmapFile> * result) override {
            return OpenMmapFile(fname, result, GetFileSize(fname), true);
        }
    };

    Env * Env::Default() {
        static PosixEnv impl;
        return &impl;
    }
}