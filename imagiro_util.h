#pragma once

// Disable warnings for MSVC
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

// Disable warnings for GCC/Clang
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Weverything" // For Clang
#endif

#include "src/util.h"
#include "src/Note.h"
#include "src/ConditionLock.h"
#include "src/filewatcher/gin_filewatcher.h"
#include "src/readerwriterqueue/readerwriterqueue.h"
#include "src/readerwriterqueue/concurrentqueue.h"
#include "src/readerwriterqueue/readerwritercircularbuffer.h"
#include "src/miniz/miniz.h"
#include "src/miniz/compress_string.h"
#include "src/BackgroundTaskRunner.h"
#include "src/structures/FixedHashSet.h"
#include "src/structures/beman/inplace_vector.h"


// Re-enable warnings for MSVC
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Re-enable warnings for GCC/Clang
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif