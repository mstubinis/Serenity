#include <serenity/discord/DiscordActivityEvent.h>
#include <serenity/discord/DiscordModule.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/window/Window.h>

::DiscordActivityEvent::DiscordActivityEvent() {
    m_Activity = discord::Activity{};

    const auto& window = Engine::Resources::getWindow();

    m_Activity.SetApplicationId(Engine::Discord::getClientID());
    m_Activity.SetName(window.getTitle().c_str());
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
bool ::DiscordActivityEvent::post() noexcept {
    return Engine::Discord::update_activity(*this);
}
const discord::Activity& ::DiscordActivityEvent::getActivity() const noexcept {
    return m_Activity;
}

void ::DiscordActivityEvent::setState(const char* state) noexcept {
    m_Activity.SetState(state);
}

void ::DiscordActivityEvent::setDetail(const char* detail) noexcept {
    m_Activity.SetDetails(detail);
}

void ::DiscordActivityEvent::setPartySizeCurrent(unsigned int currentSize) noexcept {
    m_Activity.GetParty().GetSize().SetCurrentSize(currentSize);
}

void ::DiscordActivityEvent::setPartySizeMax(unsigned int maxSize) noexcept {
    m_Activity.GetParty().GetSize().SetMaxSize(maxSize);
}

void ::DiscordActivityEvent::setImageLarge(const char* largeImage) noexcept {
    m_Activity.GetAssets().SetLargeImage(largeImage);
}

void ::DiscordActivityEvent::setImageLargeText(const char* largeImageText) noexcept {
    m_Activity.GetAssets().SetLargeText(largeImageText);
}

void ::DiscordActivityEvent::setImageSmall(const char* smallImage) noexcept {
    m_Activity.GetAssets().SetSmallImage(smallImage);
}

void ::DiscordActivityEvent::setImageSmallText(const char* smallImageText) noexcept {
    m_Activity.GetAssets().SetSmallText(smallImageText);
}

void ::DiscordActivityEvent::setPartyID(const char* partyID) noexcept {
    m_Activity.GetParty().SetId(partyID);
}

void ::DiscordActivityEvent::setInstance(bool instance) noexcept {
    m_Activity.SetInstance(instance);
}

void ::DiscordActivityEvent::setTimestampStartAsNow() noexcept {
    std::int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now);
}

void ::DiscordActivityEvent::setTimestampStart(std::int64_t start) noexcept {
    m_Activity.GetTimestamps().SetStart(start);
}

void ::DiscordActivityEvent::setTimestampEndSinceNow(std::int64_t end) noexcept {
    std::int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now + end);
}

void ::DiscordActivityEvent::setTimestampEndSinceNowSeconds(unsigned int end) noexcept {
    std::int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now + (std::int64_t)(end * 1000U));
}

void ::DiscordActivityEvent::setTimestampEnd(std::int64_t end) noexcept {
    m_Activity.GetTimestamps().SetEnd(end);
}

void ::DiscordActivityEvent::setSecretJoin(const char* join) noexcept {
    m_Activity.GetSecrets().SetJoin(join);
}

void ::DiscordActivityEvent::setSecretSpectate(const char* spectate) noexcept {
    m_Activity.GetSecrets().SetSpectate(spectate);
}

void ::DiscordActivityEvent::setSecretMatch(const char* match) noexcept {
    m_Activity.GetSecrets().SetMatch(match);
}

void ::DiscordActivityEvent::setType(const discord::ActivityType& type) noexcept {
    m_Activity.SetType(type);
}