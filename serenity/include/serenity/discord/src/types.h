#pragma once

#include "ffi.h"
#include "event.h"

namespace discord {

enum class Result {
    Ok = 0,
    ServiceUnavailable = 1,
    InvalidVersion = 2,
    LockFailed = 3,
    InternalError = 4,
    InvalidPayload = 5,
    InvalidCommand = 6,
    InvalidPermissions = 7,
    NotFetched = 8,
    NotFound = 9,
    Conflict = 10,
    InvalidSecret = 11,
    InvalidJoinSecret = 12,
    NoEligibleActivity = 13,
    InvalidInvite = 14,
    NotAuthenticated = 15,
    InvalidAccessToken = 16,
    ApplicationMismatch = 17,
    InvalidDataUrl = 18,
    InvalidBase64 = 19,
    NotFiltered = 20,
    LobbyFull = 21,
    InvalidLobbySecret = 22,
    InvalidFilename = 23,
    InvalidFileSize = 24,
    InvalidEntitlement = 25,
    NotInstalled = 26,
    NotRunning = 27,
    InsufficientBuffer = 28,
    PurchaseCanceled = 29,
    InvalidGuild = 30,
    InvalidEvent = 31,
    InvalidChannel = 32,
    InvalidOrigin = 33,
    RateLimited = 34,
    OAuth2Error = 35,
    SelectChannelTimeout = 36,
    GetGuildTimeout = 37,
    SelectVoiceForceRequired = 38,
    CaptureShortcutAlreadyListening = 39,
    UnauthorizedForAchievement = 40,
    InvalidGiftCode = 41,
    PurchaseError = 42,
    TransactionAborted = 43,
};

enum class CreateFlags {
    Default = 0,
    NoRequireDiscord = 1,
};

enum class LogLevel {
    Error = 1,
    Warn,
    Info,
    Debug,
};

enum class UserFlag {
    Partner = 2,
    HypeSquadEvents = 4,
    HypeSquadHouse1 = 64,
    HypeSquadHouse2 = 128,
    HypeSquadHouse3 = 256,
};

enum class PremiumType {
    None = 0,
    Tier1 = 1,
    Tier2 = 2,
};

enum class ImageType {
    User,
};

enum class ActivityType {
    Playing,
    Streaming,
    Listening,
    Watching,
};

enum class ActivityActionType {
    Join = 1,
    Spectate,
};

enum class ActivityJoinRequestReply {
    No,
    Yes,
    Ignore,
};

enum class Status {
    Offline = 0,
    Online = 1,
    Idle = 2,
    DoNotDisturb = 3,
};

enum class RelationshipType {
    None,
    Friend,
    Blocked,
    PendingIncoming,
    PendingOutgoing,
    Implicit,
};

enum class LobbyType {
    Private = 1,
    Public,
};

enum class LobbySearchComparison {
    LessThanOrEqual = -2,
    LessThan,
    Equal,
    GreaterThan,
    GreaterThanOrEqual,
    NotEqual,
};

enum class LobbySearchCast {
    String = 1,
    Number,
};

enum class LobbySearchDistance {
    Local,
    Default,
    Extended,
    Global,
};

enum class EntitlementType {
    Purchase = 1,
    PremiumSubscription,
    DeveloperGift,
    TestModePurchase,
    FreePurchase,
    UserGift,
    PremiumPurchase,
};

enum class SkuType {
    Application = 1,
    DLC,
    Consumable,
    Bundle,
};

enum class InputModeType {
    VoiceActivity = 0,
    PushToTalk,
};

using ClientId = std::int64_t;
using Version = std::int32_t;
using Snowflake = std::int64_t;
using Timestamp = std::int64_t;
using UserId = Snowflake;
using Locale = const char*;
using Branch = const char*;
using LobbyId = Snowflake;
using LobbySecret = const char*;
using MetadataKey = const char*;
using MetadataValue = const char*;
using NetworkPeerId = std::uint64_t;
using NetworkChannelId = uint8_t;
using Path = const char*;
using DateTime = const char*;

class User final {
public:
    void SetId(UserId id);
    UserId GetId() const;
    void SetUsername(const char* username);
    const char* GetUsername() const;
    void SetDiscriminator(const char* discriminator);
    const char* GetDiscriminator() const;
    void SetAvatar(const char* avatar);
    const char* GetAvatar() const;
    void SetBot(bool bot);
    bool GetBot() const;

private:
    DiscordUser internal_;
};

class OAuth2Token final {
public:
    void SetAccessToken(const char* accessToken);
    const char* GetAccessToken() const;
    void SetScopes(const char* scopes);
    const char* GetScopes() const;
    void SetExpires(Timestamp expires);
    Timestamp GetExpires() const;

private:
    DiscordOAuth2Token internal_;
};

class ImageHandle final {
public:
    void SetType(ImageType type);
    ImageType GetType() const;
    void SetId(std::int64_t id);
    std::int64_t GetId() const;
    void SetSize(uint32_t size);
    uint32_t GetSize() const;

private:
    DiscordImageHandle internal_;
};

class ImageDimensions final {
public:
    void SetWidth(uint32_t width);
    uint32_t GetWidth() const;
    void SetHeight(uint32_t height);
    uint32_t GetHeight() const;

private:
    DiscordImageDimensions internal_;
};

class ActivityTimestamps final {
public:
    void SetStart(Timestamp start);
    Timestamp GetStart() const;
    void SetEnd(Timestamp end);
    Timestamp GetEnd() const;

private:
    DiscordActivityTimestamps internal_;
};

class ActivityAssets final {
public:
    void SetLargeImage(const char* largeImage);
    const char* GetLargeImage() const;
    void SetLargeText(const char* largeText);
    const char* GetLargeText() const;
    void SetSmallImage(const char* smallImage);
    const char* GetSmallImage() const;
    void SetSmallText(const char* smallText);
    const char* GetSmallText() const;

private:
    DiscordActivityAssets internal_;
};

class PartySize final {
public:
    void SetCurrentSize(std::int32_t currentSize);
    std::int32_t GetCurrentSize() const;
    void SetMaxSize(std::int32_t maxSize);
    std::int32_t GetMaxSize() const;

private:
    DiscordPartySize internal_;
};

class ActivityParty final {
public:
    void SetId(const char* id);
    const char* GetId() const;
    PartySize& GetSize();
    PartySize const& GetSize() const;

private:
    DiscordActivityParty internal_;
};

class ActivitySecrets final {
public:
    void SetMatch(const char* match);
    const char* GetMatch() const;
    void SetJoin(const char* join);
    const char* GetJoin() const;
    void SetSpectate(const char* spectate);
    const char* GetSpectate() const;

private:
    DiscordActivitySecrets internal_;
};

class Activity final {
public:
    void SetType(ActivityType type);
    ActivityType GetType() const;
    void SetApplicationId(std::int64_t applicationId);
    std::int64_t GetApplicationId() const;
    void SetName(const char* name);
    const char* GetName() const;
    void SetState(const char* state);
    const char* GetState() const;
    void SetDetails(const char* details);
    const char* GetDetails() const;
    ActivityTimestamps& GetTimestamps();
    ActivityTimestamps const& GetTimestamps() const;
    ActivityAssets& GetAssets();
    ActivityAssets const& GetAssets() const;
    ActivityParty& GetParty();
    ActivityParty const& GetParty() const;
    ActivitySecrets& GetSecrets();
    ActivitySecrets const& GetSecrets() const;
    void SetInstance(bool instance);
    bool GetInstance() const;

private:
    DiscordActivity internal_;
};

class Presence final {
public:
    void SetStatus(Status status);
    Status GetStatus() const;
    Activity& GetActivity();
    Activity const& GetActivity() const;

private:
    DiscordPresence internal_;
};

class Relationship final {
public:
    void SetType(RelationshipType type);
    RelationshipType GetType() const;
    User& GetUser();
    User const& GetUser() const;
    Presence& GetPresence();
    Presence const& GetPresence() const;

private:
    DiscordRelationship internal_;
};

class Lobby final {
public:
    void SetId(LobbyId id);
    LobbyId GetId() const;
    void SetType(LobbyType type);
    LobbyType GetType() const;
    void SetOwnerId(UserId ownerId);
    UserId GetOwnerId() const;
    void SetSecret(LobbySecret secret);
    LobbySecret GetSecret() const;
    void SetCapacity(uint32_t capacity);
    uint32_t GetCapacity() const;
    void SetLocked(bool locked);
    bool GetLocked() const;

private:
    DiscordLobby internal_;
};

class FileStat final {
public:
    void SetFilename(const char* filename);
    const char* GetFilename() const;
    void SetSize(std::uint64_t size);
    std::uint64_t GetSize() const;
    void SetLastModified(std::uint64_t lastModified);
    std::uint64_t GetLastModified() const;

private:
    DiscordFileStat internal_;
};

class Entitlement final {
public:
    void SetId(Snowflake id);
    Snowflake GetId() const;
    void SetType(EntitlementType type);
    EntitlementType GetType() const;
    void SetSkuId(Snowflake skuId);
    Snowflake GetSkuId() const;

private:
    DiscordEntitlement internal_;
};

class SkuPrice final {
public:
    void SetAmount(uint32_t amount);
    uint32_t GetAmount() const;
    void SetCurrency(const char* currency);
    const char* GetCurrency() const;

private:
    DiscordSkuPrice internal_;
};

class Sku final {
public:
    void SetId(Snowflake id);
    Snowflake GetId() const;
    void SetType(SkuType type);
    SkuType GetType() const;
    void SetName(const char* name);
    const char* GetName() const;
    SkuPrice& GetPrice();
    SkuPrice const& GetPrice() const;

private:
    DiscordSku internal_;
};

class InputMode final {
public:
    void SetType(InputModeType type);
    InputModeType GetType() const;
    void SetShortcut(const char* shortcut);
    const char* GetShortcut() const;

private:
    DiscordInputMode internal_;
};

class UserAchievement final {
public:
    void SetUserId(Snowflake userId);
    Snowflake GetUserId() const;
    void SetAchievementId(Snowflake achievementId);
    Snowflake GetAchievementId() const;
    void SetPercentComplete(uint8_t percentComplete);
    uint8_t GetPercentComplete() const;
    void SetUnlockedAt(DateTime unlockedAt);
    DateTime GetUnlockedAt() const;

private:
    DiscordUserAchievement internal_;
};

class LobbyTransaction final {
public:
    Result SetType(LobbyType type);
    Result SetOwner(UserId ownerId);
    Result SetCapacity(uint32_t capacity);
    Result SetMetadata(MetadataKey key, MetadataValue value);
    Result DeleteMetadata(MetadataKey key);
    Result SetLocked(bool locked);

    IDiscordLobbyTransaction** Receive() { return &internal_; }
    IDiscordLobbyTransaction* Internal() { return internal_; }

private:
    IDiscordLobbyTransaction* internal_;
};

class LobbyMemberTransaction final {
public:
    Result SetMetadata(MetadataKey key, MetadataValue value);
    Result DeleteMetadata(MetadataKey key);

    IDiscordLobbyMemberTransaction** Receive() { return &internal_; }
    IDiscordLobbyMemberTransaction* Internal() { return internal_; }

private:
    IDiscordLobbyMemberTransaction* internal_;
};

class LobbySearchQuery final {
public:
    Result Filter(MetadataKey key,
                  LobbySearchComparison comparison,
                  LobbySearchCast cast,
                  MetadataValue value);
    Result Sort(MetadataKey key, LobbySearchCast cast, MetadataValue value);
    Result Limit(uint32_t limit);
    Result Distance(LobbySearchDistance distance);

    IDiscordLobbySearchQuery** Receive() { return &internal_; }
    IDiscordLobbySearchQuery* Internal() { return internal_; }

private:
    IDiscordLobbySearchQuery* internal_;
};

} // namespace discord