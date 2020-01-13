#include "Faction.h"

using namespace std;

std::vector<FactionInformation> Factions::Database;

FactionInformation::FactionInformation() {
    NameShort = NameLong = FileTag = Description = "";
    ColorShield = ColorText = glm::vec4(1.0f);
}
FactionInformation::FactionInformation(
const string& nameLong, 
const string& nameShort, 
const glm::vec4& colorText, 
const glm::vec4& colorShield, 
const string& fileTag,
const glm::vec4& gUIColor,
const glm::vec4& gUIColorSlightlyDarker,
const glm::vec4& gUIColorDark,
const glm::vec4& gUIColorHighlight,
const glm::vec4& gUIColorText1,
const string& desc) {
    NameLong               = nameLong;
    NameShort              = nameShort;
    ColorText              = colorText;
    ColorShield            = colorShield;
    FileTag                = fileTag;
    Description            = desc;

    GUIColor               = gUIColor;
    GUIColorSlightlyDarker = gUIColorSlightlyDarker;
    GUIColorDark           = gUIColorDark;
    GUIColorHighlight      = gUIColorHighlight;
    GUIColorText1          = gUIColorText1;
}
FactionInformation::~FactionInformation() {

}


void Factions::init() {
    if (Database.size() > 0)
        return;
    Database.resize(FactionEnum::_TOTAL);


    //Fed
    Database[FactionEnum::Federation] = FactionInformation(
        "United Federation of Planets",
        "Federation",
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), //text color
        glm::vec4(0.0822f, 0.408f, 1.0f, 0.7f), //shields color
        "fed",
        glm::vec4(0.5f, 0.78f, 0.94f, 1.0f),
        glm::vec4(0.5f, 0.78f, 0.94f, 1.0f) * glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.5f, 0.78f, 0.94f, 1.0f) * glm::vec4(0.03f, 0.03f, 0.03f, 1.0f),
        glm::vec4(0.5f, 0.78f, 0.94f, 1.0f) + glm::vec4(0.25f, 0.25f, 0.25f, 0.0f),
        glm::vec4(0.94f,0.83f, 0.6f, 1.0f ),
        ("A unification of various cultures and worlds, the United Federation of Planets occupies a large amount of space in the Alpha and Beta quadrants of the galaxy, centered around their capital world of Earth in the Sol system."
            "\n\nThe Federation was built around the principles of exploration and diplomacy, prefering to solve problems with peace and using force only as a last resort. This policy is a core principle that has always remained "
            "constant, but for much of the Federation's existence, it has also guided the development of the Federation as a space faring entity, both technologically and militarily. The Federation built their ships and technology "
            "soley to fullfill their core principles, creating a vast and powerful economy and culture, but developing their defense and military at a much slower pace, for it was not seen as needed at the time due to the relative stability "
            "of Federation space and the neighboring factions. This period of harmony would soon be disrupted however, as a force of unbelieveable power introduces itself to the Federation..."
            "\n\nAfter the forced introduction of the Borg to the Federation by the entity known as Q, the Borg learned of the location of the Federation and sent a cube to assimilate them. This resulted in the battle of Wolf 359, "
            "where 39 Federation starships were destroyed, resulting in over 11,000 lives lost in a valiant effort to halt the borg advance on Earth. The Borg emerged from the battle practically unharmed, and Earth was almost certain to fall. "
            "The U.S.S. Enterprise-D intercepted the Borg cube as it began Earth orbit and used their knowledge of Borg computer codes (acquired by rescuing the assimilated Captain Picard earlier) to destroy the Borg cube by initiating its self destruct sequence. "
            "Earth was given a 'stay of execution' and what could have been much worse was still very much a disaster, the most bloody battle in Federation history was won only due to pure luck. The Federation was faced with a cold and harsh reality; they were arrogant to "
            "the status quo and thought they were immune to any instellar danger, and thought the rest of the galaxy would see interactions as they do - with peace and prosperity. Seeing the destruction left behind by a foe of unimaginable power, the Federation's "
            "nearly neutered combat capabilities needed to be developed and expanded, resulting in a large scale research and development effort to develop new starships and technologies designed for war."
            "\n\nThe research and development gave rise to a new line of starships, like the Defiant, Nova, Akira, Intrepid, Sovereign, Steamrunner, Saber, Norway, and Prometheus class starships. These starships were designed to include the potential for war. Their combat capabilities were "
            "heavily expanded upon, and most civilian facilities were limited. The Federation even removed civilian families onboard their starships. With the threat of the borg dying down due to their disappearance after Wolf 359, most of these projects "
            "were mothballed. Those projects that remained active were developed more slowly. The Federation seemed to have once again put their existence at risk by letting their core principles take priority even after the Borg threat showed no signs of permanent defeat. Perhaps a second catalyst would be needed "
            "to fully convince the Federation that there are others in the galaxy that would threaten their very existance. Unfortunately, that second catalyst would soon manifest itself..."
            "\n\nA faction originating from the Gamma quadrant of the galaxy known as 'The Dominion' would wage war on the Federation, as they saw the Federation as a threat due to the their continuing exploration of the Gamma quadrant and thus violating Dominion territory. The Dominion were xenophobic, and saw outsiders as a threat to their existence. "
            "The Dominion wasted no time planning to take over the Alpha and Beta quadrants of the galaxy, as they would prefer to conquer and subvert these quadrants rather than letting them develop and hope they would remain peaceful. The Dominion War finally convinced the Federation to continue the projects that were previously halted. "
            "The Defiant class escort was launched and proved to be a highly effective warship. The Akira and Saber classes were commissioned shortly after. Eventually the Federation's modern fleet was rebuilt with a new vision and this next generation fleet, along with an alliance between the Klingon and Romulan "
            "Empires allowed for the Dominion War to concluded in an Alpha and Beta quadrant victory. By this time the Federation has fully adopted their new vision of peace and prosperity, backed up by a respectable defensive force, hoping they wouldn't need to use it. Nevertheless, a third and final catalyst would manifest and put the Federation's new fleet to the test, for the Borg have returned..."
            "\n\nThe Borg, thinking that their previous encounter with the Federation and its result was just a fluke, decided to send the same task force to assimilate the Federation again: a single cube. The Federation was way more prepared this time around, using its new fleet and tactics to defeat the Borg."
            "The Borg cube encountered a four stage defensive line of Federation vessels, most of which were of the new generation classes. The Borg, due to their sheer power, manage to push through the first three lines and reached the final defensive line at Earth despite the Borg cube taking heavy damage to their outer hull. It was during this battle, named the Battle of Sector 001 that the U.S.S. Enterprise-E, a newly comissioned Sovereign class starship "
            "came to the aid of the Federation forces despite being ordered to stay out of the fight (this was due to the fact that Captain Picard's previous assimilation meant that most felt he could not be trusted). The Enterprise however proved its worth and rallied the fleet to target a previously unknown vulnerability on the Borg cube, which was discovered due to Picard's innate ability to hear the Borg thoughts during the battle. "
            "This resulted in a tactical victory for the Federation as the Borg cube was destroyed via external weapons fire. This encounter with the Borg resulted in roughly 20 Federation ships destroyed, out of a fleet of at least 30 ships. Despite encounters with the Borg normally concluding with heavy casualities, the fact that much of the fleet survived and that the Borg cube was destroyed conventionally proved that the advancement of Federation technology and firepower was worth it."
            "\n\nThe Federation now stands as a highly advanced and respectable force within the galaxy, clinging to their never changing principles of peace and prosperity, but having a now technologically superior fleet present to defend their ability to carry out this principle.")
    );
    //Kli
    Database[FactionEnum::Klingon] = FactionInformation(
        "Klingon Empire",
        "Klingon",
        glm::vec4(0.72f, 0.11f, 0.11f, 1.0f), //text color
        glm::vec4(1.0f, 0.0f, 0.0f, 0.7f), //shields color
        "kli",
        glm::vec4(0.834f, 0.26f, 0.13f, 1.0f),
        glm::vec4(0.834f, 0.26f, 0.13f, 1.0f) * glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.834f, 0.26f, 0.13f, 1.0f) * glm::vec4(0.03f, 0.03f, 0.03f, 1.0f),
        glm::vec4(0.834f, 0.26f, 0.13f, 1.0f) + glm::vec4(0.25f, 0.25f, 0.25f, 0.0f),
        glm::vec4(0.94f, 0.83f, 0.6f, 1.0f),
        ""
    );
    //Rom
    Database[FactionEnum::Romulan] = FactionInformation(
        "Romulan Star Empire",
        "Romulan",
        glm::vec4(0.33f, 0.72f, 0.48f, 1.0f), //text color
        glm::vec4(0.0f, 1.0f, 0.0f, 0.7f), //shields color
        "rom",
        glm::vec4(0.278f, 0.813f, 0.56f, 1.0f),
        glm::vec4(0.278f, 0.813f, 0.56f, 1.0f) * glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.278f, 0.813f, 0.56f, 1.0f) * glm::vec4(0.03f, 0.03f, 0.03f, 1.0f),
        glm::vec4(0.278f, 0.813f, 0.56f, 1.0f) + glm::vec4(0.25f, 0.25f, 0.25f, 0.0f),
        glm::vec4(0.94f, 0.83f, 0.6f, 1.0f),
        ""
    );
    //Borg
    Database[FactionEnum::Borg] = FactionInformation(
        "The Borg Collective",
        "Borg",
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), //text color
        glm::vec4(0.0f, 1.0f, 0.0f, 0.7f), //shields color
        "borg",
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * glm::vec4(0.03f, 0.03f, 0.03f, 1.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + glm::vec4(0.25f, 0.25f, 0.25f, 0.0f),
        glm::vec4(0.94f, 0.83f, 0.6f, 1.0f),
        ""
    );
}
void Factions::destruct() {
    Database.clear();
}