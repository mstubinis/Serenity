//#if !defined(_CRT_SECURE_NO_WARNINGS)
//#define _CRT_SECURE_NO_WARNINGS
//#endif

#include <core/engine/utils/PrecompiledHeader.h>
#include "storage_manager.h"

#include "core.h"

//#include <cstring>
//#include <memory>

namespace discord {

Result StorageManager::Read(const char* name,
                            std::uint8_t* data,
                            uint32_t dataLength,
                            uint32_t* read)
{
    if (!read) {
        return Result::InternalError;
    }

    auto result = internal_->read(internal_,
                                  const_cast<char*>(name),
                                  reinterpret_cast<uint8_t*>(data),
                                  dataLength,
                                  reinterpret_cast<uint32_t*>(read));
    return static_cast<Result>(result);
}

void StorageManager::ReadAsync(const char* name,
                               std::function<void(Result, std::uint8_t*, uint32_t)> callback)
{
    static auto wrapper =
      [](void* callbackData, EDiscordResult result, uint8_t* data, uint32_t dataLength) -> void {
        std::unique_ptr<std::function<void(Result, std::uint8_t*, uint32_t)>> cb(
          reinterpret_cast<std::function<void(Result, std::uint8_t*, uint32_t)>*>(
            callbackData));
        if (!cb || !(*cb)) {
            return;
        }
        (*cb)(static_cast<Result>(result), data, dataLength);
    };
    std::unique_ptr<std::function<void(Result, std::uint8_t*, uint32_t)>> cb{};
    cb.reset(NEW std::function<void(Result, std::uint8_t*, uint32_t)>(std::move(callback)));
    internal_->read_async(internal_, const_cast<char*>(name), cb.release(), wrapper);
}

void StorageManager::ReadAsyncPartial(
  const char* name,
  std::uint64_t offset,
  std::uint64_t length,
  std::function<void(Result, std::uint8_t*, uint32_t)> callback)
{
    static auto wrapper =
      [](void* callbackData, EDiscordResult result, uint8_t* data, uint32_t dataLength) -> void {
        std::unique_ptr<std::function<void(Result, std::uint8_t*, uint32_t)>> cb(
          reinterpret_cast<std::function<void(Result, std::uint8_t*, uint32_t)>*>(
            callbackData));
        if (!cb || !(*cb)) {
            return;
        }
        (*cb)(static_cast<Result>(result), data, dataLength);
    };
    std::unique_ptr<std::function<void(Result, std::uint8_t*, uint32_t)>> cb{};
    cb.reset(NEW std::function<void(Result, std::uint8_t*, uint32_t)>(std::move(callback)));
    internal_->read_async_partial(
      internal_, const_cast<char*>(name), offset, length, cb.release(), wrapper);
}

Result StorageManager::Write(const char* name, std::uint8_t* data, uint32_t dataLength)
{
    auto result = internal_->write(
      internal_, const_cast<char*>(name), reinterpret_cast<uint8_t*>(data), dataLength);
    return static_cast<Result>(result);
}

void StorageManager::WriteAsync(const char* name,
                                std::uint8_t* data,
                                uint32_t dataLength,
                                std::function<void(Result)> callback)
{
    static auto wrapper = [](void* callbackData, EDiscordResult result) -> void {
        std::unique_ptr<std::function<void(Result)>> cb(
          reinterpret_cast<std::function<void(Result)>*>(callbackData));
        if (!cb || !(*cb)) {
            return;
        }
        (*cb)(static_cast<Result>(result));
    };
    std::unique_ptr<std::function<void(Result)>> cb{};
    cb.reset(NEW std::function<void(Result)>(std::move(callback)));
    internal_->write_async(internal_,
                           const_cast<char*>(name),
                           reinterpret_cast<uint8_t*>(data),
                           dataLength,
                           cb.release(),
                           wrapper);
}

Result StorageManager::Delete(const char* name)
{
    auto result = internal_->delete_(internal_, const_cast<char*>(name));
    return static_cast<Result>(result);
}

Result StorageManager::Exists(const char* name, bool* exists)
{
    if (!exists) {
        return Result::InternalError;
    }

    auto result =
      internal_->exists(internal_, const_cast<char*>(name), reinterpret_cast<bool*>(exists));
    return static_cast<Result>(result);
}

void StorageManager::Count(std::int32_t* count)
{
    if (!count) {
        return;
    }

    internal_->count(internal_, reinterpret_cast<int32_t*>(count));
}

Result StorageManager::Stat(const char* name, FileStat* stat)
{
    if (!stat) {
        return Result::InternalError;
    }

    auto result =
      internal_->stat(internal_, const_cast<char*>(name), reinterpret_cast<DiscordFileStat*>(stat));
    return static_cast<Result>(result);
}

Result StorageManager::StatAt(std::int32_t index, FileStat* stat)
{
    if (!stat) {
        return Result::InternalError;
    }

    auto result = internal_->stat_at(internal_, index, reinterpret_cast<DiscordFileStat*>(stat));
    return static_cast<Result>(result);
}

Result StorageManager::GetPath(char path[4096])
{
    if (!path) {
        return Result::InternalError;
    }

    auto result = internal_->get_path(internal_, reinterpret_cast<DiscordPath*>(path));
    return static_cast<Result>(result);
}

} // namespace discord
