#pragma once

#include "ecs.hpp"
#include "common.hpp"

enum BoneAsset
{
	SKULL,
	SCAPULA,
	VERTEBRA_1,
	VERTEBRA_2,
	VERTEBRA_3,
	VERTEBRA_4,
    FLIPPER
};

struct Bone
{
public:
	Bone(BoneAsset type);
	BoneAsset type;
	static entt::entity createBone(vec2 position, BoneAsset assetIndex);
};
