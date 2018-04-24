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

        std::unique_ptr<SequentialFile>
        OpenSequentialFile(const std::string & fname) override {
            int fd;
            int flags = O_RDONLY;

            do {
                fd = open(fname.c_str(), flags, kPermission);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);

            FILE * file;
            do {
                file = fdopen(fd, "r");
            } while (file == nullptr && errno == EINTR);
            if (file == nullptr) {
                close(fd);
                throw IO_EXCEPTION(fname);
            }
            return std::make_unique<PosixSequentialFile>(fname, file);
        }

        std::unique_ptr<RandomAccessFile>
        OpenRandomAccessFie(const std::string & fname) override {
            int fd;
            int flags = O_RDONLY;

            do {
                fd = open(fname.c_str(), flags, kPermission);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);
            return std::make_unique<PosixRandomAccessFile>(fname, fd);
        }

        static std::unique_ptr<WritableFile>
        OpenWritableFile(const std::string & fname, bool reopen) {
            int fd;
            int flags;
            size_t filesize;
            if (reopen) {
                flags = O_CREAT | O_WRONLY | O_APPEND;
                filesize = Default()->GetFileSize(fname);
            } else {
                flags = O_CREAT | O_WRONLY | O_TRUNC;
                filesize = 0;
            }

            do {
                fd = open(fname.c_str(), flags, kPermission);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);
            return std::make_unique<PosixWritableFile>(fname, filesize, fd);
        }

        std::unique_ptr<WritableFile>
        OpenWritableFile(const std::string & fname) override {
            return OpenWritableFile(fname, false);
        }

        std::unique_ptr<WritableFile>
        ReopenWritableFile(const std::string & fname) override {
            return OpenWritableFile(fname, true);
        }

        static std::unique_ptr<MmapFile>
        OpenMmapFile(const std::string & fname, bool reopen) {
            int fd;
            int flags;
            size_t len;
            if (reopen) {
                flags = O_CREAT | O_RDWR | O_APPEND;
                len = Default()->GetFileSize(fname);
            } else {
                flags = O_CREAT | O_RDWR | O_TRUNC;
                len = MmapFile::kMinSize;
            }

            do {
                fd = open(fname.c_str(), flags, kPermission);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) {
                throw IO_EXCEPTION(fname);
            }
            SetCLOEXEC(fd);

            if (!reopen) {
                int r = ftruncate(fd, MmapFile::kMinSize);
                if (r != 0) {
                    throw IO_EXCEPTION(fname);
                }
            }

            void * base = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (base == MAP_FAILED) {
                throw IO_EXCEPTION(fname);
            }
            return std::make_unique<PosixMmapFile>(fname, base, len, fd);
        }

        std::unique_ptr<MmapFile>
        OpenMmapFile(const std::string & fname) override {
            return OpenMmapFile(fname, false);
        }

        std::unique_ptr<MmapFile>
        ReopenMmapFile(const std::string & fname) override {
            return OpenMmapFile(fname, true);
        }
    };

    Env * Env::Default() {
        static PosixEnv impl;
        return &impl;
    }
}