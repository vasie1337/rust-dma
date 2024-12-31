#pragma once
#include "../../../include.hpp"

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
	bool bones_updated = false;

	bool is_npc = false;

	bool IsIndexValid(int index)
	{
		switch (index)
		{
		case l_knee:
		case l_foot:
		case r_knee:
		case r_foot:
		case spine1:
		case l_forearm:
		case l_hand:
		case neck:
		case head:
		case r_forearm:
		case r_hand:
			return true;
		default:
			return false;
		}
	}


	Vector3 GetBonePosition(int index)
	{
		//if (!IsIndexValid(index))
		//	return Vector3();
		if (bones.empty())
			return Vector3();
		if (bones.size() <= index)
			return Vector3();
		if (!bones[index].transformAccess)
			return Vector3();

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
