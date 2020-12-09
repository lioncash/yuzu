// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "common/common_types.h"
#ifdef _MSC_VER
#include "common/string_util.h"
#endif

namespace Common::FS {

// User paths for GetUserPath
enum class UserPath {
    CacheDir,
    ConfigDir,
    KeysDir,
    LogDir,
    NANDDir,
    RootDir,
    SDMCDir,
    LoadDir,
    DumpDir,
    ScreenshotsDir,
    ShaderDir,
    SysDataDir,
    UserDir,
};

// Returns true if the path exists
[[nodiscard]] bool Exists(const std::filesystem::path& path);

// Returns true if path is a directory
[[nodiscard]] bool IsDirectory(const std::filesystem::path& path);

// Returns the size of filename (64bit)
[[nodiscard]] u64 GetSize(const std::filesystem::path& path);

// Overloaded GetSize, accepts FILE*
[[nodiscard]] u64 GetSize(FILE* f);

// Returns true if successful, or path already exists.
bool CreateDir(const std::filesystem::path& path);

// Create all directories in path
// Returns true if successful, or path already exists.
[[nodiscard("Directory creation can fail and must be tested")]] bool CreateDirs(
    const std::filesystem::path& path);

// Creates directories in path. Returns true on success.
[[deprecated("This function is deprecated, use CreateDirs")]] bool CreateFullPath(
    const std::filesystem::path& path);

// Deletes a given file at the path.
// This will also delete empty directories.
// Return true on success
bool Delete(const std::filesystem::path& path);

// Renames file src to dst, returns true on success
bool Rename(const std::filesystem::path& src, const std::filesystem::path& dst);

// copies file src to dst, returns true on success
bool Copy(const std::filesystem::path& src, const std::filesystem::path& dst);

// creates an empty file filename, returns true on success
bool CreateEmptyFile(const std::string& filename);

/**
 * @param num_entries_out to be assigned by the callable with the number of iterated directory
 * entries, never null
 * @param directory the path to the enclosing directory
 * @param virtual_name the entry name, without any preceding directory info
 * @return whether handling the entry succeeded
 */
using DirectoryEntryCallable =
    std::function<bool(u64* num_entries_out, const std::filesystem::path& directory,
                       const std::filesystem::path& virtual_name)>;

/**
 * Scans a directory, calling the callback for each file/directory contained within.
 * If the callback returns failure, scanning halts and this function returns failure as well
 *
 * @param num_entries_out Assigned by the function with the number of iterated directory entries,
 *                        can be null
 * @param directory       The directory to scan
 * @param callback        The callback which will be called for each entry
 *
 * @return whether scanning the directory succeeded
 */
bool ForeachDirectoryEntry(u64* num_entries_out, const std::filesystem::path& directory,
                           const DirectoryEntryCallable& callback);

// Deletes the given path and anything under it. Returns true on success.
bool DeleteDirRecursively(const std::filesystem::path& path);

// Returns the current directory
[[nodiscard]] std::optional<std::filesystem::path> GetCurrentDir();

// Create directory and copy contents (does not overwrite existing files)
void CopyDir(const std::filesystem::path& src, const std::filesystem::path& dst);

// Set the current directory to given path
bool SetCurrentDir(const std::filesystem::path& path);

// Returns a pointer to a string with a yuzu data dir in the user's home
// directory. To be used in "multi-user" mode (that is, installed).
const std::string& GetUserPath(UserPath path, const std::string& new_path = "");

[[nodiscard]] std::string GetHactoolConfigurationPath();

[[nodiscard]] std::string GetNANDRegistrationDir(bool system = false);

// Returns the path to where the sys file are
[[nodiscard]] std::string GetSysDirectory();

#ifdef __APPLE__
[[nodiscard]] std::string GetBundleDirectory();
#endif

#ifdef _WIN32
[[nodiscard]] const std::string& GetExeDirectory();
[[nodiscard]] std::string AppDataRoamingDirectory();
#endif

std::size_t WriteStringToFile(bool text_file, const std::string& filename, std::string_view str);

std::size_t ReadFileToString(bool text_file, const std::string& filename, std::string& str);

/**
 * Splits the filename into 8.3 format
 * Loosely implemented following https://en.wikipedia.org/wiki/8.3_filename
 * @param filename The normal filename to use
 * @param short_name A 9-char array in which the short name will be written
 * @param extension A 4-char array in which the extension will be written
 */
void SplitFilename83(const std::string& filename, std::array<char, 9>& short_name,
                     std::array<char, 4>& extension);

// Splits the path on '/' or '\' and put the components into a vector
// i.e. "C:\Users\Yuzu\Documents\save.bin" becomes {"C:", "Users", "Yuzu", "Documents", "save.bin" }
[[nodiscard]] std::vector<std::string> SplitPathComponents(std::string_view filename);

// Gets all of the text up to the last '/' or '\' in the path.
[[nodiscard]] std::string_view GetParentPath(std::string_view path);

// Gets all of the text after the first '/' or '\' in the path.
[[nodiscard]] std::string_view GetPathWithoutTop(std::string_view path);

// Gets the filename of the path
[[nodiscard]] std::string_view GetFilename(std::string_view path);

// Gets the extension of the filename
[[nodiscard]] std::string_view GetExtensionFromFilename(std::string_view name);

// Removes the final '/' or '\' if one exists
[[nodiscard]] std::string_view RemoveTrailingSlash(std::string_view path);

// Creates a new vector containing indices [first, last) from the original.
template <typename T>
[[nodiscard]] std::vector<T> SliceVector(const std::vector<T>& vector, std::size_t first,
                                         std::size_t last) {
    if (first >= last) {
        return {};
    }
    last = std::min<std::size_t>(last, vector.size());
    return std::vector<T>(vector.begin() + first, vector.begin() + first + last);
}

enum class DirectorySeparator {
    ForwardSlash,
    BackwardSlash,
    PlatformDefault,
};

// Removes trailing slash, makes all '\\' into '/', and removes duplicate '/'. Makes '/' into '\\'
// depending if directory_separator is BackwardSlash or PlatformDefault and running on windows
[[nodiscard]] std::string SanitizePath(
    std::string_view path,
    DirectorySeparator directory_separator = DirectorySeparator::ForwardSlash);

// To deal with Windows being dumb at Unicode
template <typename T>
void OpenFStream(T& fstream, const std::string& filename, std::ios_base::openmode openmode) {
#ifdef _MSC_VER
    fstream.open(Common::UTF8ToUTF16W(filename), openmode);
#else
    fstream.open(filename, openmode);
#endif
}

// simple wrapper for cstdlib file functions to
// hopefully will make error checking easier
// and make forgetting an fclose() harder
class IOFile : public NonCopyable {
public:
    IOFile();
    // flags is used for windows specific file open mode flags, which
    // allows yuzu to open the logs in shared write mode, so that the file
    // isn't considered "locked" while yuzu is open and people can open the log file and view it
    IOFile(const std::string& filename, const char openmode[], int flags = 0);

    ~IOFile();

    IOFile(IOFile&& other) noexcept;
    IOFile& operator=(IOFile&& other) noexcept;

    void Swap(IOFile& other) noexcept;

    bool Open(const std::string& filename, const char openmode[], int flags = 0);
    bool Close();

    template <typename T>
    std::size_t ReadArray(T* data, std::size_t length) const {
        static_assert(std::is_trivially_copyable_v<T>,
                      "Given array does not consist of trivially copyable objects");

        return ReadImpl(data, length, sizeof(T));
    }

    template <typename T>
    std::size_t WriteArray(const T* data, std::size_t length) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "Given array does not consist of trivially copyable objects");

        return WriteImpl(data, length, sizeof(T));
    }

    template <typename T>
    std::size_t ReadBytes(T* data, std::size_t length) const {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return ReadArray(reinterpret_cast<char*>(data), length);
    }

    template <typename T>
    std::size_t WriteBytes(const T* data, std::size_t length) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return WriteArray(reinterpret_cast<const char*>(data), length);
    }

    template <typename T>
    std::size_t WriteObject(const T& object) {
        static_assert(!std::is_pointer_v<T>, "WriteObject arguments must not be a pointer");
        return WriteArray(&object, 1);
    }

    std::size_t WriteString(std::string_view str) {
        return WriteArray(str.data(), str.length());
    }

    [[nodiscard]] bool IsOpen() const {
        return nullptr != m_file;
    }

    bool Seek(s64 off, int origin) const;
    [[nodiscard]] u64 Tell() const;
    [[nodiscard]] u64 GetSize() const;
    bool Resize(u64 size);
    bool Flush();

    // clear error state
    void Clear() {
        std::clearerr(m_file);
    }

private:
    std::size_t ReadImpl(void* data, std::size_t length, std::size_t data_size) const;
    std::size_t WriteImpl(const void* data, std::size_t length, std::size_t data_size);

    std::FILE* m_file = nullptr;
};

} // namespace Common::FS
