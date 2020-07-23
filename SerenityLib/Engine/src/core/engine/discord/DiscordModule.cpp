#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/discord/DiscordModule.h>
#include <core/engine/discord/DiscordActivityEvent.h>
#include <core/engine/discord/src/discord.h>

using namespace std;
using namespace Engine::priv;

DiscordModule* discordManager = nullptr;

DiscordModule::DiscordModule() {
    m_DiscordCore  = {};
    discordManager = this;
}
DiscordModule::~DiscordModule() {
    SAFE_DELETE(m_DiscordCore);
}
const discord::ClientId DiscordModule::get_client_id() const {
    return m_ClientID;
}
bool DiscordModule::activate(const discord::ClientId& clientID) {
    if (m_Active || !m_DiscordCore) {
        return false;
    }
    m_ClientID = clientID;

    auto result = discord::Core::Create(m_ClientID, EDiscordCreateFlags::DiscordCreateFlags_NoRequireDiscord, &m_DiscordCore);
    if (result == discord::Result::Ok) {
        auto log_func = [&](discord::LogLevel level, const char* message) {
            if (m_LoggingErrors) {
                cout << ("Discord: " + to_string(static_cast<unsigned int>(level)) + " - " + string(message)) << "\n";
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
            cout << internal_get_result_string_message(result) << "\n";
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
            cout << internal_get_result_string_message(result) << "\n";
        }
    });
    return true;
}
void DiscordModule::log_errors(const bool logErrors) {
    m_LoggingErrors = logErrors;
}
string DiscordModule::internal_get_result_string_message(const discord::Result& result) {
    string error = "";
    switch (result) {
        case discord::Result::Ok: {
            error = "There is no error!"; break;
        }case discord::Result::ServiceUnavailable: {
            error = "Discord isn't working"; break;
        }case discord::Result::LockFailed: {
            error = "an internal error on transactional operations"; break;
        }case discord::Result::InternalError: {
            error = "something on our side went wrong"; break;
        }case discord::Result::InvalidVersion: {
            error = "the SDK version may be outdated"; break;
        }case discord::Result::InvalidPayload: {
            error = "the data you sent didn't match what we expect"; break;
        }case discord::Result::InvalidCommand: {
            error = "that's not a thing you can do"; break;
        }case discord::Result::InvalidPermissions: {
            error = "you aren't authorized to do that"; break;
        }case discord::Result::NotFetched: {
            error = "couldn't fetch what you wanted"; break;
        }case discord::Result::NotFound: {
            error = "what you're looking for doesn't exist"; break;
        }case discord::Result::Conflict: {
            error = "user already has a network connection open on that channel"; break;
        }case discord::Result::InvalidSecret: {
            error = "activity secrets must be unique and not match party id"; break;
        }/*case discord::Result::InvalidJoinRequest: {
            error = "join request for that user does not exist"; break;
        }*/case discord::Result::NoEligibleActivity: {
            error = "you accidentally set an ApplicationId in your UpdateActivity() payload"; break;
        }case discord::Result::InvalidInvite: {
            error = "your game invite is no longer valid"; break;
        }case discord::Result::NotAuthenticated: {
            error = "the internal auth call failed for the user, and you can't do this"; break;
        }case discord::Result::InvalidAccessToken: {
            error = "the user's bearer token is invalid"; break;
        }case discord::Result::ApplicationMismatch: {
            error = "access token belongs to another application"; break;
        }case discord::Result::InvalidDataUrl: {
            error = "something internally went wrong fetching image data"; break;
        }case discord::Result::InvalidBase64: {
            error = "not valid Base64 data"; break;
        }case discord::Result::NotFiltered: {
            error = "you're trying to access the list before creating a stable list with Filter()"; break;
        }case discord::Result::LobbyFull: {
            error = "the lobby is full"; break;
        }case discord::Result::InvalidLobbySecret: {
            error = "the secret you're using to connect is wrong"; break;
        }case discord::Result::InvalidFilename: {
            error = "file name is too long"; break;
        }case discord::Result::InvalidFileSize: {
            error = "file is too large"; break;
        }case discord::Result::InvalidEntitlement: {
            error = "the user does not have the right entitlement for this game"; break;
        }case discord::Result::NotInstalled: {
            error = "Discord is not installed"; break;
        }case discord::Result::NotRunning: {
            error = "Discord is not running"; break;
        }default: {
            error = "Uh oh, discord could not find out what error this is"; break;
        }
    }
    return error;
}
bool DiscordModule::update() {
    if (!m_Active || !m_DiscordCore) {
        return false;
    }
    const auto result = m_DiscordCore->RunCallbacks();
    if (m_LoggingErrors) {
        cout << internal_get_result_string_message(result) << '\n';
    }
    return true;
}


const discord::ClientId Engine::Discord::getClientID() {
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
bool Engine::Discord::activate(const discord::ClientId& clientID){
    if (!discordManager) {
        return false;
    }
    return discordManager->activate(clientID);
}