#pragma once

#include "types.h"

namespace discord {

class ActivityManager final {
public:
    ~ActivityManager() = default;

    Result RegisterCommand(const char* command);
    Result RegisterSteam(uint32_t steamId);
    void UpdateActivity(Activity const& activity, std::function<void(Result)> callback);
    void ClearActivity(std::function<void(Result)> callback);
    void SendRequestReply(UserId userId,
                          ActivityJoinRequestReply reply,
                          std::function<void(Result)> callback);
    void SendInvite(UserId userId,
                    ActivityActionType type,
                    const char* content,
                    std::function<void(Result)> callback);
    void AcceptInvite(UserId userId, std::function<void(Result)> callback);

    Event<const char*> OnActivityJoin;
    Event<const char*> OnActivitySpectate;
    Event<User const&> OnActivityJoinRequest;
    Event<ActivityActionType, User const&, Activity const&> OnActivityInvite;

private:
    friend class Core;

    ActivityManager() = default;
    ActivityManager(ActivityManager const& rhs) = delete;
    ActivityManager& operator=(ActivityManager const& rhs) = delete;
    ActivityManager(ActivityManager&& rhs) = delete;
    ActivityManager& operator=(ActivityManager&& rhs) = delete;

    IDiscordActivityManager* internal_;
    static IDiscordActivityEvents events_;
};

} // namespace discord
