#include "Faction.h"

using namespace std;

std::vector<FactionInformation> Factions::Database;

FactionInformation::FactionInformation() {
    NameShort = NameLong = "";
    ColorShield = ColorText = glm::vec4(1.0f);
}
FactionInformation::FactionInformation(const string& nameLong, const string& nameShort, const glm::vec4& colorText, const glm::vec4& colorShield) {
    NameLong = nameLong;
    NameShort = nameShort;
    ColorText = colorText;
    ColorShield = colorShield;
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
        glm::vec4(1.0f), //text color
        glm::vec4(0.0822f, 0.408f, 1.0f, 0.7f) //shields color
    );
    //Kli
    Database[FactionEnum::Klingon] = FactionInformation(
        "Klingon Empire",
        "Klingon",
        glm::vec4(0.72f, 0.11f, 0.11f, 1.0f), //text color
        glm::vec4(1, 0, 0, 0.7f) //shields color
    );
    //Rom
    Database[FactionEnum::Romulan] = FactionInformation(
        "Romulan Star Empire",
        "Romulan",
        glm::vec4(0.33f, 0.72f, 0.48f, 1.0f), //text color
        glm::vec4(0, 1, 0, 0.7f) //shields color
    );
    //Borg
    Database[FactionEnum::Borg] = FactionInformation(
        "The Borg Collective",
        "Borg",
        glm::vec4(0, 1, 0, 1), //text color
        glm::vec4(0, 1, 0, 0.7f) //shields color
    );
}
void Factions::destruct() {
    Database.clear();
}