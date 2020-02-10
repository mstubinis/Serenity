#include <core/engine/discord/Discord.h>
#include <core/engine/discord/src/discord.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/window/Window.h>

#include <iostream>
#include <chrono>

using namespace std;


discord::Core* core{};
Engine::Discord::DiscordCore* discordManager = nullptr;


Engine::Discord::DiscordActivity::DiscordActivity() {
    m_Activity = discord::Activity();
    if (discordManager) {
        const auto& window = Resources::getWindow();

        m_Activity.SetApplicationId(discordManager->m_ClientID);
        m_Activity.SetName(window.name().c_str());
        m_Activity.SetDetails("");
        m_Activity.SetState("");
        m_Activity.SetInstance(false);
        m_Activity.GetAssets().SetLargeImage("");
        m_Activity.GetAssets().SetLargeText("");
        m_Activity.GetAssets().SetSmallImage("");
        m_Activity.GetAssets().SetSmallText("");
        m_Activity.GetTimestamps().SetStart(0);
        m_Activity.GetTimestamps().SetEnd(0);
        m_Activity.SetType(discord::ActivityType::Playing);
        m_Activity.GetParty().SetId("");
        m_Activity.GetParty().GetSize().SetCurrentSize(1);
        m_Activity.GetParty().GetSize().SetMaxSize(1);
        m_Activity.GetSecrets().SetJoin("");
        m_Activity.GetSecrets().SetMatch("");
        m_Activity.GetSecrets().SetSpectate("");
    }
}
Engine::Discord::DiscordActivity::~DiscordActivity() {

}
const discord::Activity& Engine::Discord::DiscordActivity::getActivity() const {
    return m_Activity;
}
void Engine::Discord::DiscordActivity::setState(const std::string& state) {
    m_Activity.SetState(state.c_str());
}
void Engine::Discord::DiscordActivity::setDetail(const std::string& detail) {
    m_Activity.SetDetails(detail.c_str());
}
void Engine::Discord::DiscordActivity::setPartySizeCurrent(const unsigned int& currentSize) {
    m_Activity.GetParty().GetSize().SetCurrentSize(currentSize);
}
void Engine::Discord::DiscordActivity::setPartySizeMax(const unsigned int& maxSize) {
    m_Activity.GetParty().GetSize().SetMaxSize(maxSize);
}
void Engine::Discord::DiscordActivity::setImageLarge(const std::string& largeImage) {
    m_Activity.GetAssets().SetLargeImage(largeImage.c_str());
}
void Engine::Discord::DiscordActivity::setImageLargeText(const std::string& largeImageText) {
    m_Activity.GetAssets().SetLargeText(largeImageText.c_str());
}
void Engine::Discord::DiscordActivity::setImageSmall(const std::string& smallImage) {
    m_Activity.GetAssets().SetSmallImage(smallImage.c_str());
}
void Engine::Discord::DiscordActivity::setImageSmallText(const std::string& smallImageText) {
    m_Activity.GetAssets().SetSmallText(smallImageText.c_str());
}
void Engine::Discord::DiscordActivity::setPartyID(const std::string& partyID) {
    m_Activity.GetParty().SetId(partyID.c_str());
}
void Engine::Discord::DiscordActivity::setInstance(const bool& instance) {
    m_Activity.SetInstance(instance);
}
void Engine::Discord::DiscordActivity::setTimestampStartAsNow() {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now);
}
void Engine::Discord::DiscordActivity::setTimestampStart(const std::int64_t start) {
    m_Activity.GetTimestamps().SetStart(start);
}
void Engine::Discord::DiscordActivity::setTimestampEndSinceNow(const std::int64_t end) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now + end);
}
void Engine::Discord::DiscordActivity::setTimestampEndSinceNowSeconds(const unsigned int& end) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now + static_cast<std::int64_t>(end * 1000));
}
void Engine::Discord::DiscordActivity::setTimestampEnd(const std::int64_t end) {
    m_Activity.GetTimestamps().SetEnd(end);
}
void Engine::Discord::DiscordActivity::setSecretJoin(const std::string& join) {
    m_Activity.GetSecrets().SetJoin(join.c_str());
}
void Engine::Discord::DiscordActivity::setSecretSpectate(const std::string& spectate) {
    m_Activity.GetSecrets().SetSpectate(spectate.c_str());
}
void Engine::Discord::DiscordActivity::setSecretMatch(const std::string& match) {
    m_Activity.GetSecrets().SetMatch(match.c_str());
}
void Engine::Discord::DiscordActivity::setType(const discord::ActivityType& type) {
    m_Activity.SetType(type);
}



Engine::Discord::DiscordCore::DiscordCore() {
    m_ClientID = 0;
    m_LoggingErrors = true;
    m_Active = false;
    discordManager = this;
}
Engine::Discord::DiscordCore::~DiscordCore() {
    SAFE_DELETE(core);
}

void Engine::Discord::DiscordCore::update_activity(const discord::Activity& activity) {
    if (!m_Active)
        return;

    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        //std::cout << get_result_string_message(result) << std::endl;
    });
}
void Engine::Discord::DiscordCore::clear_activity() {
    if (!m_Active)
        return;

    core->ActivityManager().ClearActivity([](discord::Result result) {
        //std::cout << get_result_string_message(result) << std::endl;
    });
}
const bool Engine::Discord::DiscordCore::activate(const discord::ClientId& clientID) {
    if (m_Active) {
        return false;
    }
    m_ClientID = clientID;

    auto result = discord::Core::Create(m_ClientID, EDiscordCreateFlags::DiscordCreateFlags_NoRequireDiscord, &core);
    if (result == discord::Result::Ok) {
        auto log_func = [&](discord::LogLevel level, const char* message) {
            if (m_LoggingErrors) {
                std::cout << ("Discord: " + std::to_string(static_cast<unsigned int>(level)) + " - " + string(message)) << std::endl;
            }
        };
        //TODO: remove / restrict logging to debug builds
        core->SetLogHook(discord::LogLevel::Debug, log_func);
        m_Active = true;
        return true;
    }
    return false;
}
void Engine::Discord::DiscordCore::log_errors(const bool log) {
    m_LoggingErrors = log;
}
void Engine::Discord::DiscordCore::update() {
    if (!m_Active)
        return;
    const auto result = core->RunCallbacks();
    //std::cout << Engine::Discord::DiscordCore::get_result_string_message(result) << std::endl;
}
const string Engine::Discord::DiscordCore::get_result_string_message(const discord::Result& result) {
    string error = "";
    switch (result) {
        case discord::Result::Ok: {
            error = "There is no error!"; break;
        }
        case discord::Result::ServiceUnavailable: {
            error = "Discord isn't working"; break;
        }
        case discord::Result::LockFailed: {
            error = "an internal error on transactional operations"; break;
        }
        case discord::Result::InternalError: {
            error = "something on our side went wrong"; break;
        }
        case discord::Result::InvalidVersion: {
            error = "the SDK version may be outdated"; break;
        }
        case discord::Result::InvalidPayload: {
            error = "the data you sent didn't match what we expect"; break;
        }
        case discord::Result::InvalidCommand: {
            error = "that's not a thing you can do"; break;
        }
        case discord::Result::InvalidPermissions: {
            error = "you aren't authorized to do that"; break;
        }
        case discord::Result::NotFetched: {
            error = "couldn't fetch what you wanted"; break;
        }
        case discord::Result::NotFound: {
            error = "what you're looking for doesn't exist"; break;
        }
        case discord::Result::Conflict: {
            error = "user already has a network connection open on that channel"; break;
        }
        case discord::Result::InvalidSecret: {
            error = "activity secrets must be unique and not match party id"; break;
        }
        //case discord::Result::InvalidJoinRequest: {
        //    error = "join request for that user does not exist"; break;
        //}
        case discord::Result::NoEligibleActivity: {
            error = "you accidentally set an ApplicationId in your UpdateActivity() payload"; break;
        }
        case discord::Result::InvalidInvite: {
            error = "your game invite is no longer valid"; break;
        }
        case discord::Result::NotAuthenticated: {
            error = "the internal auth call failed for the user, and you can't do this"; break;
        }
        case discord::Result::InvalidAccessToken: {
            error = "the user's bearer token is invalid"; break;
        }
        case discord::Result::ApplicationMismatch: {
            error = "access token belongs to another application"; break;
        }
        case discord::Result::InvalidDataUrl: {
            error = "something internally went wrong fetching image data"; break;
        }
        case discord::Result::InvalidBase64: {
            error = "not valid Base64 data"; break;
        }
        case discord::Result::NotFiltered: {
            error = "you're trying to access the list before creating a stable list with Filter()"; break;
        }
        case discord::Result::LobbyFull: {
            error = "the lobby is full"; break;
        }
        case discord::Result::InvalidLobbySecret: {
            error = "the secret you're using to connect is wrong"; break;
        }
        case discord::Result::InvalidFilename: {
            error = "file name is too long"; break;
        }
        case discord::Result::InvalidFileSize: {
            error = "file is too large"; break;
        }
        case discord::Result::InvalidEntitlement: {
            error = "the user does not have the right entitlement for this game"; break;
        }
        case discord::Result::NotInstalled: {
            error = "Discord is not installed"; break;
        }
        case discord::Result::NotRunning: {
            error = "Discord is not running"; break;
        }
        default: {
            error = "Uh oh, discord could not find out what error this is"; break;
        }
    }
    return error;
}
void Engine::Discord::update_activity(const Engine::Discord::DiscordActivity& activity_) {
    discordManager->update_activity(activity_.getActivity());
}
void Engine::Discord::clear_activity() {
    discordManager->clear_activity();
}
const bool Engine::Discord::activate(const discord::ClientId& clientID) {
    if (!discordManager)
        return false;
    return discordManager->activate(clientID);
}