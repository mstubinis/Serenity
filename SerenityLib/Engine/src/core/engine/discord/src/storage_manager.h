#pragma once

#include "types.h"

namespace discord {

class StorageManager final {
public:
    ~StorageManager() = default;

    Result Read(const char* name,
                std::uint8_t* data,
                uint32_t dataLength,
                uint32_t* read);
    void ReadAsync(const char* name,
                   std::function<void(Result, std::uint8_t*, uint32_t)> callback);
    void ReadAsyncPartial(const char* name,
                          std::uint64_t offset,
                          std::uint64_t length,
                          std::function<void(Result, std::uint8_t*, uint32_t)> callback);
    Result Write(const char* name, std::uint8_t* data, uint32_t dataLength);
    void WriteAsync(const char* name,
                    std::uint8_t* data,
                    uint32_t dataLength,
                    std::function<void(Result)> callback);
    Result Delete(const char* name);
    Result Exists(const char* name, bool* exists);
    void Count(std::int32_t* count);
    Result Stat(const char* name, FileStat* stat);
    Result StatAt(std::int32_t index, FileStat* stat);
    Result GetPath(char path[4096]);

private:
    friend class Core;

    StorageManager() = default;
    StorageManager(StorageManager const& rhs) = delete;
    StorageManager& operator=(StorageManager const& rhs) = delete;
    StorageManager(StorageManager&& rhs) = delete;
    StorageManager& operator=(StorageManager&& rhs) = delete;

    IDiscordStorageManager* internal_;
    static IDiscordStorageEvents events_;
};

} // namespace discord
