#pragma once
#ifndef POSIX_ENV_ENV_H
#define POSIX_ENV_ENV_H

// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// All Env implementations are safe for concurrent access from
// multiple threads without any external synchronization.

/*
 * 作者: 左吉吉
 * 时间: 2018夏初
 * 发布协议: AGPL
 *
 * 注意: 全组件使用 **异常** 替代 **状态码**
 */

#include <memory>

#include "slice.h"

namespace penv {
    class MmapFile;

    class RandomAccessFile;

    class SequentialFile;

    class WritableFile;

    class Env {
    public:
        Env() = default;

        virtual ~Env() = default;

    public:
        static Env * Default();

        virtual size_t GetFileSize(const std::string & fname) = 0;

    public:
        enum {
            kPermission = 0644
        };

        virtual void OpenSequentialFile(const std::string & fname,
                                        std::unique_ptr<SequentialFile> * result) = 0;

        virtual void OpenRandomAccessFie(const std::string & fname,
                                         std::unique_ptr<RandomAccessFile> * result) = 0;

        virtual void OpenWritableFile(const std::string & fname,
                                      std::unique_ptr<WritableFile> * result) = 0;

        virtual void ReopenWritableFile(const std::string & fname,
                                        std::unique_ptr<WritableFile> * result) = 0;

        virtual void OpenMmapFile(const std::string & fname,
                                  std::unique_ptr<MmapFile> * result) = 0;

        virtual void ReopenMmapFile(const std::string & fname,
                                    std::unique_ptr<MmapFile> * result) = 0;
    };

    class SequentialFile {
    public:
        SequentialFile() = default;

        virtual ~SequentialFile() = default;

    public:
        virtual void Read(size_t n, char * scratch) = 0;

        virtual void Skip(size_t n) = 0;
    };

    class RandomAccessFile {
    public:
        RandomAccessFile() = default;

        virtual ~RandomAccessFile() = default;

    public:
        virtual void ReadAt(size_t offset, size_t n, char * scratch) const = 0;

        virtual void Prefetch(size_t offset, size_t n) = 0;

        enum AccessPattern {
            NORMAL, SEQUENTIAL, RANDOM, NOREUSE, WILLNEED, DONTNEED
        };

        virtual void Hint(AccessPattern hint) = 0;
    };

    class WritableFile {
    public:
        WritableFile() = default;

        virtual ~WritableFile() = default;

    public:
        virtual void Write(const Slice & data) = 0;

        virtual void Truncate(size_t n) = 0;

        virtual void Sync() = 0;

        virtual size_t GetFileSize() const = 0;

        // These values match Linux definition
        enum WriteLifeTimeHint {
            WLTH_NOT_SET = 0,
            WLTH_NONE,
            WLTH_SHORT,
            WLTH_MEDIUM,
            WLTH_LONG,
            WLTH_EXTREME
        };

        virtual void Hint(WriteLifeTimeHint hint) = 0;

        virtual void RangeSync(size_t offset, size_t n) = 0;

        virtual void PrepareWrite(size_t offset, size_t n) = 0;

        virtual void Allocate(size_t offset, size_t n) = 0;
    };

    class MmapFile {
    public:
        MmapFile() = default;

        virtual ~MmapFile() = default;

    public:
        virtual void * Base() = 0;

        virtual const void * Base() const = 0;

        enum {
            kMinSize = 4096
        };

        virtual size_t GetFileSize() const = 0;

        virtual void Resize(size_t n) = 0;

        enum AccessPattern {
            NORMAL, SEQUENTIAL, RANDOM, WILLNEED, DONTNEED
        };

        virtual void Hint(AccessPattern hint) = 0;
    };
}

#endif //POSIX_ENV_ENV_H
