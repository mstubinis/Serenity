#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/discord/DiscordActivityEvent.h>
#include <core/engine/discord/DiscordModule.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/window/Window.h>

using namespace std;

::DiscordActivityEvent::DiscordActivityEvent() {
    m_Activity = discord::Activity();

    const auto& window = Engine::Resources::getWindow();

    m_Activity.SetApplicationId(Engine::Discord::getClientID());
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
::DiscordActivityEvent::~DiscordActivityEvent() {

}
bool ::DiscordActivityEvent::post() {
    return Engine::Discord::update_activity(*this);
}
const discord::Activity& ::DiscordActivityEvent::getActivity() const {
    return m_Activity;
}

void ::DiscordActivityEvent::setState(const string& state) {
    m_Activity.SetState(state.c_str());
}

void ::DiscordActivityEvent::setDetail(const string& detail) {
    m_Activity.SetDetails(detail.c_str());
}

void ::DiscordActivityEvent::setPartySizeCurrent(unsigned int currentSize) {
    m_Activity.GetParty().GetSize().SetCurrentSize(currentSize);
}

void ::DiscordActivityEvent::setPartySizeMax(unsigned int maxSize) {
    m_Activity.GetParty().GetSize().SetMaxSize(maxSize);
}

void ::DiscordActivityEvent::setImageLarge(const string& largeImage) {
    m_Activity.GetAssets().SetLargeImage(largeImage.c_str());
}

void ::DiscordActivityEvent::setImageLargeText(const string& largeImageText) {
    m_Activity.GetAssets().SetLargeText(largeImageText.c_str());
}

void ::DiscordActivityEvent::setImageSmall(const string& smallImage) {
    m_Activity.GetAssets().SetSmallImage(smallImage.c_str());
}

void ::DiscordActivityEvent::setImageSmallText(const string& smallImageText) {
    m_Activity.GetAssets().SetSmallText(smallImageText.c_str());
}

void ::DiscordActivityEvent::setPartyID(const string& partyID) {
    m_Activity.GetParty().SetId(partyID.c_str());
}

void ::DiscordActivityEvent::setInstance(bool instance) {
    m_Activity.SetInstance(instance);
}

void ::DiscordActivityEvent::setTimestampStartAsNow() {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now);
}

void ::DiscordActivityEvent::setTimestampStart(int64_t start) {
    m_Activity.GetTimestamps().SetStart(start);
}

void ::DiscordActivityEvent::setTimestampEndSinceNow(int64_t end) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now + end);
}

void ::DiscordActivityEvent::setTimestampEndSinceNowSeconds(unsigned int end) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_Activity.GetTimestamps().SetStart(now + static_cast<std::int64_t>(end * 1000));
}

void ::DiscordActivityEvent::setTimestampEnd(std::int64_t end) {
    m_Activity.GetTimestamps().SetEnd(end);
}

void ::DiscordActivityEvent::setSecretJoin(const string& join) {
    m_Activity.GetSecrets().SetJoin(join.c_str());
}

void ::DiscordActivityEvent::setSecretSpectate(const string& spectate) {
    m_Activity.GetSecrets().SetSpectate(spectate.c_str());
}

void ::DiscordActivityEvent::setSecretMatch(const string& match) {
    m_Activity.GetSecrets().SetMatch(match.c_str());
}

void ::DiscordActivityEvent::setType(const discord::ActivityType& type) {
    m_Activity.SetType(type);
}