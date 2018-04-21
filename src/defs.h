#pragma once
#ifndef POSIX_ENV_DEFS_H
#define POSIX_ENV_DEFS_H

#if defined(__linux__)
#define PENV_OS_LINUX
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define PENV_OS_MACOSX
#endif

#if defined(PENV_OS_MACOSX)
#if !defined(fread_unlocked)
#define fread_unlocked fread
#endif
#if !defined(fwrite_unlocked)
#define fwrite_unlocked fwrite
#endif
#if !defined(fflush_unlocked)
#define fflush_unlocked fflush
#endif
#if !defined(fdatasync)
#define fdatasync fsync
#endif
#endif // defined(PENV_OS_MACOSX)

#define PENV_S1(x) #x
#define PENV_S2(x) PENV_S1(x)
#define PENV_EXCEPTION_INFO std::string(__PRETTY_FUNCTION__) + " at line " PENV_S2(__LINE__)

#endif //POSIX_ENV_DEFS_H
