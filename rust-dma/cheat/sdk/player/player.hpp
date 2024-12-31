#pragma once
#include "../../../include.hpp"

enum BoneList
{
    l_hip = 1,
    l_knee,
    l_foot,
    l_toe,
    l_ankle_scale,
    pelvis,
    penis,
    GenitalCensor,
    GenitalCensor_LOD0,
    Inner_LOD0,
    GenitalCensor_LOD1,
    GenitalCensor_LOD2,
    r_hip,
    r_knee,
    r_foot,
    r_toe,
    r_ankle_scale,
    spine1,
    spine1_scale,
    spine2,
    spine3,
    spine4,
    l_clavicle,
    l_upperarm,
    l_forearm,
    l_hand,
    l_index1,
    l_index2,
    l_index3,
    l_little1,
    l_little2,
    l_little3,
    l_middle1,
    l_middle2,
    l_middle3,
    l_prop,
    l_ring1,
    l_ring2,
    l_ring3,
    l_thumb1,
    l_thumb2,
    l_thumb3,
    IKtarget_righthand_min,
    IKtarget_righthand_max,
    l_ulna,
    neck,
    head,
    jaw,
    eyeTranform,
    l_eye,
    l_Eyelid,
    r_eye,
    r_Eyelid,
    r_clavicle,
    r_upperarm,
    r_forearm,
    r_hand,
    max_bones
};

struct BoneConnection {
    Vector3 start;
    Vector3 end;
};

class Player : public Entity
{
public:
    Player() = default;
    explicit Player(const Entity& entityRef) : Entity(entityRef) {}
    virtual ~Player() = default;

	bool operator==(const Player& other) const {
		return object_ptr == other.object_ptr;
	}

    std::uintptr_t model = 0;
    std::uintptr_t player_model = 0;
    std::uintptr_t bone_transforms = 0;

	std::vector<Transform> bones;
	bool bones_fetched = false;

	bool is_npc = false;

	Vector3 GetBonePosition(int index)
	{
		if (!bones_fetched)
			return Vector3(0, 0, 0);
		if (!bones.size())
			return Vector3(0, 0, 0);
		if (index >= bones.size())
			return Vector3(0, 0, 0);
		return bones[index].position();
	}

	Vector3 GetHeadPosition()
	{
		return GetBonePosition(BoneList::head);
	}

	std::vector<std::pair<BoneList, BoneList>> SkeletonConnections = {
			{ BoneList::r_foot, BoneList::r_knee },
			{ BoneList::l_foot, BoneList::l_knee },
			{ BoneList::r_knee, BoneList::spine1 },
			{ BoneList::l_knee, BoneList::spine1 },
			{ BoneList::spine1, BoneList::neck },
			{ BoneList::neck, BoneList::head },
			{ BoneList::neck, BoneList::r_forearm },
			{ BoneList::r_forearm, BoneList::r_hand },
			{ BoneList::neck, BoneList::l_forearm },
			{ BoneList::l_forearm, BoneList::l_hand }
	};
};
