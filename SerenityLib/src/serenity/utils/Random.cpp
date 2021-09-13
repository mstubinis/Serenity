#include <serenity/utils/Random.h>


std::mt19937  Engine::priv::RandomImpl::m_MT = std::mt19937(std::random_device{}());