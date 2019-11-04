#pragma once
#ifndef GAME_AI_H
#define GAME_AI_H

#include "AIIncludes.h"

class AI {
    private:
        AIType::Type m_Type;
    public:
        AI(const AIType::Type type);
        ~AI();

        const AIType::Type& getType() const;

        virtual void update(const double& dt);
};

#endif