#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "env.h"
#include "mmap_file.h"
#include "random_access_file.h"
#include "sequential_file.h"
#include "writable_file.h"

#define IO_EXCEPTION(f) std::runtime_error("IO:" + PENV_EXCEPTION_INFO + " | " + strerror(errno) + " | " + (f))

namespace penv {
    class PosixEnv : public Env {
    public:
        ~PosixEnv() override = default;

    public:
        size_t GetFileSize(const std::string & fname) override {
            struct stat sbuf;
            if (stat(fname.c_str(), &sbuf) != 0) {
                throw IO_EXCEPTION(fname);
            } else {
                return static_cast<size_t>(sbuf.st_size);
            }
        }

    public:
        inline static void SetCLOEXEC(int fd) {
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
        }

        void OpenSequentialFile(const std::string & fname,
                                std::unique_ptr<SequentialFile> * result) override {
            int fd;
            int flags = O_RDONLY;
            FILE * file = nullptr;

            do {
                fd = open(fname.c_str(), flags, 0644 /* 权限 */);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);

            do {
                file = fdopen(fd, "r");
            } while (file == nullptr && errno == EINTR);
            if (file == nullptr) {
                close(fd);
                throw IO_EXCEPTION(fname);
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
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);
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
                                 bool reopen) {
            int fd;
            int flags = (reopen ? (O_CREAT | O_APPEND) : (O_CREAT | O_TRUNC)) | O_RDWR;

            do {
                fd = open(fname.c_str(), flags, 0644 /* 权限 */);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);

            size_t len = reopen ? Default()->GetFileSize(fname) : MmapFile::kMinSize;
            void * base = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (base == MAP_FAILED) {
                throw IO_EXCEPTION(fname);
            }
            *result = std::make_unique<PosixMmapFile>(fname, base, len, fd);
        }

        void OpenMmapFile(const std::string & fname,
                          std::unique_ptr<MmapFile> * result) override {
            return OpenMmapFile(fname, result, false);
        }

        void ReopenMmapFile(const std::string & fname,
                            std::unique_ptr<MmapFile> * result) override {
            return OpenMmapFile(fname, result, true);
        }
    };

    Env * Env::Default() {
        static PosixEnv impl;
        return &impl;
    }
}