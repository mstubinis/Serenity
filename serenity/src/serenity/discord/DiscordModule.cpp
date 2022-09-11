#include <serenity/discord/DiscordModule.h>
#include <serenity/discord/DiscordActivityEvent.h>
#include <serenity/discord/src/discord.h>

#include <serenity/system/Macros.h>

using namespace Engine::priv;

DiscordModule* discordManager = nullptr;

DiscordModule::DiscordModule() {
    m_DiscordCore  = {};
    discordManager = this;
}
DiscordModule::~DiscordModule() {
    SAFE_DELETE(m_DiscordCore);
}
bool DiscordModule::activate(discord::ClientId clientID) {
    if (m_Active || !m_DiscordCore) {
        return false;
    }
    m_ClientID = clientID;

    auto result = discord::Core::Create(m_ClientID, EDiscordCreateFlags::DiscordCreateFlags_NoRequireDiscord, &m_DiscordCore);
    if (result == discord::Result::Ok) {
        auto log_func = [&](discord::LogLevel level, const char* message) {
            if (m_LoggingErrors) {
                ENGINE_LOG("Discord: " << std::to_string(static_cast<unsigned int>(level)) << " - " << message)
            }
        };
        //TODO: remove / restrict logging to debug builds
        m_DiscordCore->SetLogHook(discord::LogLevel::Debug, log_func);
        m_Active = true;
        return true;
    }
    return false;
}
bool DiscordModule::update_activity(const discord::Activity& activity) {
    if (!m_Active || !m_DiscordCore) {
        return false;
    }
    m_DiscordCore->ActivityManager().UpdateActivity(activity, [this](discord::Result result) {
        if (m_LoggingErrors) {
            ENGINE_LOG(internal_get_result_string_message(result))
        }
    });
    return true;
}
bool DiscordModule::clear_activity() {
    if (!m_Active || !m_DiscordCore) {
        return false;
    }
    m_DiscordCore->ActivityManager().ClearActivity([this](discord::Result result) {
        if (m_LoggingErrors) {
            ENGINE_LOG(internal_get_result_string_message(result))
        }
    });
    return true;
}
void DiscordModule::log_errors(bool logErrors) {
    m_LoggingErrors = logErrors;
}
std::string DiscordModule::internal_get_result_string_message(const discord::Result& result) {
    return [&result]() -> std::string {
        switch (result) {
            case discord::Result::Ok: {
                return "There is no error!";
            }case discord::Result::ServiceUnavailable: {
                return "Discord isn't working";
            }case discord::Result::LockFailed: {
                return "an internal error on transactional operations";
            }case discord::Result::InternalError: {
                return "something on our side went wrong";
            }case discord::Result::InvalidVersion: {
                return "the SDK version may be outdated";
            }case discord::Result::InvalidPayload: {
                return "the data you sent didn't match what we expect";
            }case discord::Result::InvalidCommand: {
                return "that's not a thing you can do";
            }case discord::Result::InvalidPermissions: {
                return "you aren't authorized to do that";
            }case discord::Result::NotFetched: {
                return "couldn't fetch what you wanted";
            }case discord::Result::NotFound: {
                return "what you're looking for doesn't exist";
            }case discord::Result::Conflict: {
                return "user already has a network connection open on that channel";
            }case discord::Result::InvalidSecret: {
                return "activity secrets must be unique and not match party id";
            }/*case discord::Result::InvalidJoinRequest: {
                return "join request for that user does not exist";
            }*/case discord::Result::NoEligibleActivity: {
                return "you accidentally set an ApplicationId in your UpdateActivity() payload";
            }case discord::Result::InvalidInvite: {
                return "your game invite is no longer valid";
            }case discord::Result::NotAuthenticated: {
                return "the internal auth call failed for the user, and you can't do this";
            }case discord::Result::InvalidAccessToken: {
                return "the user's bearer token is invalid";
            }case discord::Result::ApplicationMismatch: {
                return "access token belongs to another application";
            }case discord::Result::InvalidDataUrl: {
                return "something internally went wrong fetching image data";
            }case discord::Result::InvalidBase64: {
                return "not valid Base64 data";
            }case discord::Result::NotFiltered: {
                return "you're trying to access the list before creating a stable list with Filter()";
            }case discord::Result::LobbyFull: {
                return "the lobby is full";
            }case discord::Result::InvalidLobbySecret: {
                return "the secret you're using to connect is wrong";
            }case discord::Result::InvalidFilename: {
                return "file name is too long";
            }case discord::Result::InvalidFileSize: {
                return "file is too large";
            }case discord::Result::InvalidEntitlement: {
                return "the user does not have the right entitlement for this game";
            }case discord::Result::NotInstalled: {
                return "Discord is not installed";
            }case discord::Result::NotRunning: {
                return "Discord is not running";
            }default: {
                return "Uh oh, discord could not find out what error this is";
            }
        }
    }();
}
bool DiscordModule::update() {
    if (!m_Active || !m_DiscordCore) {
        return false;
    }
    auto result = m_DiscordCore->RunCallbacks();
    if (m_LoggingErrors) {
        ENGINE_LOG(internal_get_result_string_message(result))
    }
    return true;
}


discord::ClientId Engine::Discord::getClientID() {
    return (discordManager) ? discordManager->get_client_id() : 0;
}
bool Engine::Discord::update_activity(const DiscordActivityEvent& activity) {
    if (!discordManager) {
        return false;
    }
    return discordManager->update_activity(activity.getActivity());
}
bool Engine::Discord::clear_activity() {
    if (!discordManager) {
        return false;
    }
    return discordManager->clear_activity();
}
bool Engine::Discord::activate(discord::ClientId clientID){
    if (!discordManager) {
        return false;
    }
    return discordManager->activate(clientID);
}