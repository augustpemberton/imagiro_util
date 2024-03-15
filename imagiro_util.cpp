
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


#include "src/util.cpp"
#include "src/filewatcher/gin_filewatcher.cpp"
#include "src/zstr/miniz.cpp"

// Re-enable warnings for MSVC
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Re-enable warnings for GCC/Clang
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
