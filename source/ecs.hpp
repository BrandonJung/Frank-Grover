#pragma once

#include <entt/entt.hpp>

#include <string>
#include <iostream>
#include <bitset>

extern entt::registry _ecs;

void printECSState(entt::registry& registry);
