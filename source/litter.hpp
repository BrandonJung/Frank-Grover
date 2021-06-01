#pragma once

#include "ecs.hpp"
#include "common.hpp"

enum LitterAsset
{
	BANANA,
	FRUITS,
	CANS,
	SHOES,
	PLASTICCUP,
	PAPERCUP,
	BAG,
	TEDDY
};

struct Litter
{
public:
	static entt::entity createLitter(vec2 position, LitterAsset assetIndex);
	LitterAsset asset;
};
