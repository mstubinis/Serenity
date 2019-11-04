#include "AI.h"

using namespace std;

AI::AI(const AIType::Type type) {
    m_Type = type;
}
AI::~AI() {

}

const AIType::Type& AI::getType() const {
    return m_Type;
}

void AI::update(const double& dt) {

}