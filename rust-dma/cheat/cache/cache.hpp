#pragma once
#include "../../include.hpp"

class Cache : public CacheData
{
public:
	Cache() {}
	~Cache() {}

	void Run()
	{
		view_scatter_handle = dma.CreateScatterHandle();

		globals_thread.Run();
		entities_thread.Run();
		bones_thread.Run();
		pos_thread.Run();
		bones_update_thread.Run();

		threads = { globals_thread, entities_thread, bones_thread, pos_thread, bones_update_thread };
	}

	void Stop()
	{
		globals_thread.Stop();
		entities_thread.Stop();
		bones_thread.Stop();
		pos_thread.Stop();
		bones_update_thread.Stop();

		dma.CloseScatterHandle(view_scatter_handle);
	}

	static void UpdateViewMatrix()
	{
		if (view_scatter_handle)
		{
			Matrix4x4 new_view_matrix;
			Vector3 new_camera_pos;

			dma.AddScatterRead(view_scatter_handle, camera_object.load() + Offsets::view_matrix, &new_view_matrix, sizeof(new_view_matrix));
			dma.AddScatterRead(view_scatter_handle, camera_object.load() + Offsets::camera_pos, &new_camera_pos, sizeof(new_camera_pos));

			dma.ExecuteScatterRead(view_scatter_handle);

			view_matrix.store(new_view_matrix);
			camera_pos.store(new_camera_pos);
		}
	}

private:
	void FetchGlobals(HANDLE scatter_handle);
	void FetchEntities(HANDLE scatter_handle);
	void FetchBones(HANDLE scatter_handle);
	void UpdatePositions(HANDLE scatter_handle);
	void UpdateBones(HANDLE scatter_handle);

	std::string FormatObjectName(const std::string& object_name);

public:
	CacheThread globals_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchGlobals, this, std::placeholders::_1)), 
		3000,
		"Globals Fetch"
	);
	CacheThread entities_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchEntities, this, std::placeholders::_1)),
		1000,
		"Entities Fetch"
	);
	CacheThread bones_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchBones, this, std::placeholders::_1)),
		1000,
		"Bones Fetch"
	);
	CacheThread pos_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::UpdatePositions, this, std::placeholders::_1)),
		100,
		"Positions Update"
	);
	CacheThread bones_update_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::UpdateBones, this, std::placeholders::_1)),
		10,
		"Bones Update"
	);

	static inline std::vector<std::reference_wrapper<CacheThread>> threads = {};
	static inline HANDLE view_scatter_handle = 0;
};

void Cache::FetchGlobals(HANDLE scatter_handle)
{
	if (!entity_list.load())
	{
		uintptr_t base_net_workable = dma.Read<uintptr_t>(base_address.load() + Offsets::base_net_workable);

		uintptr_t bn_static_fields = dma.Read<uintptr_t>(base_net_workable + 0xB8);

		uintptr_t bn_wrapper_class_ptr = dma.Read<uintptr_t>(bn_static_fields + 0x30);

		uintptr_t bn_wrapper_class = decryption::BaseNetworkable(base_address.load(), bn_wrapper_class_ptr);

		uintptr_t bn_parent_static_fields = dma.Read<uintptr_t>(bn_wrapper_class + 0x10);

		uintptr_t bn_parent_static_class = decryption::BaseNetworkable(base_address.load(), bn_parent_static_fields);

		entity_list.store(dma.Read<uintptr_t>(bn_parent_static_class + 0x18));
	}
	
	static uintptr_t main_camera_manager;
	if (!main_camera_manager)
		main_camera_manager = dma.Read<uintptr_t>(base_address.load() + Offsets::main_camera);

	uintptr_t camera_manager = dma.Read<uintptr_t>(main_camera_manager + 0xB8);

	uintptr_t camera = dma.Read<uintptr_t>(camera_manager + 0xC0);

	camera_object.store(dma.Read<uintptr_t>(camera + 0x10));
}

void Cache::FetchEntities(HANDLE scatter_handle)
{
	const EntityListData entity_list_data = dma.Read<EntityListData>(entity_list.load() + 0x10);
	if (!entity_list_data)
	{
		return;
	}

	std::unordered_map<uintptr_t, Entity> existing_entity_map;
	std::unordered_map<uintptr_t, Player> existing_player_map;

	for (const auto& entity : entities.load())
	{
		existing_entity_map[entity.object_ptr] = entity;
	}

	for (const auto& player : players.load())
	{
		existing_player_map[player.object_ptr] = player;
	}

	std::vector<Entity> new_entities(entity_list_data.size);
	std::vector<Player> new_players;

	for (uint32_t i = 0; i < entity_list_data.size; i++)
	{
		new_entities[i].idx = i;
		dma.AddScatterRead(scatter_handle, entity_list_data.content + (0x20 + (static_cast<unsigned long long>(i) * 8)), &new_entities[i].object_ptr, sizeof(new_entities[i].object_ptr));
	}
	dma.ExecuteScatterRead(scatter_handle);

	std::vector<std::reference_wrapper<Entity>> entities_to_update;
	for (auto& entity : new_entities)
	{
		auto it = existing_entity_map.find(entity.object_ptr);
		if (it == existing_entity_map.end())
		{
			entities_to_update.push_back(std::ref(entity));
		}
		else
		{
			entity = it->second;
		}
	}

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		dma.AddScatterRead(scatter_handle, entity.object_ptr + 0x10, &entity.base_object, sizeof(entity.base_object));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		dma.AddScatterRead(scatter_handle, entity.base_object + 0x30, &entity.object, sizeof(entity.object));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		dma.AddScatterRead(scatter_handle, entity.object + 0x30, &entity.object_class, sizeof(entity.object_class));
		dma.AddScatterRead(scatter_handle, entity.object + 0x60, &entity.nameptr, sizeof(entity.nameptr));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		dma.AddScatterRead(scatter_handle, entity.object_class + 0x8, &entity.transform, sizeof(entity.transform));
		dma.AddScatterRead(scatter_handle, entity.nameptr, entity.name_buffer, sizeof(entity.name_buffer));
		dma.AddScatterRead(scatter_handle, entity.object + 0x54, &entity.tag, sizeof(entity.tag));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		dma.AddScatterRead(scatter_handle, entity.transform + 0x38, &entity.visual_state, sizeof(entity.visual_state));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		entity.obj_name = entity.name_buffer;
		entity.formatted_name = FormatObjectName(entity.obj_name);
	}

	for (auto& entity : new_entities)
	{
		if (entity.tag == 6) // player
		{
			if (entity.idx != 0) // not local player idx
			{
				auto it = existing_player_map.find(entity.object_ptr);
				if (it == existing_player_map.end())
				{
					new_players.push_back(Player(entity));
				}
				else
				{
					new_players.push_back(it->second);
				}
			}
			else
			{
				local_player.store(Player(entity));
			}
		}
	}

	std::vector<std::reference_wrapper<Player>> players_to_update;
	for (auto& player : new_players)
	{
		if (existing_player_map.find(player.object_ptr) == existing_player_map.end())
		{
			players_to_update.push_back(std::ref(player));
		}
	}

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		dma.AddScatterRead(scatter_handle, player.object_ptr + 0xC8, &player.model, sizeof(player.model));
		dma.AddScatterRead(scatter_handle, player.object_ptr + 0x310, &player.player_model, sizeof(player.player_model));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		dma.AddScatterRead(scatter_handle, player.player_model + 0x2E2, &player.is_npc, sizeof(player.is_npc));
		dma.AddScatterRead(scatter_handle, player.model + 0x50, &player.bone_transforms, sizeof(player.bone_transforms));
	}
	dma.ExecuteScatterRead(scatter_handle);

	entities.store(new_entities);
	players.store(new_players);
}

void Cache::FetchBones(HANDLE scatter_handle)
{
	std::vector<Player> new_players = players.load();
	if (new_players.empty())
		return;

	std::unordered_map<uintptr_t, Player> existing_players;
	for (const auto& player : players.load())
	{
		if (!player.bones.empty() && player.bone_transforms != 0)
		{
			existing_players[player.object_ptr] = player;
		}
	}

	std::vector<std::reference_wrapper<Player>> players_to_update;

	for (auto& player : new_players)
	{
		auto it = existing_players.find(player.object_ptr);

		if (player.bones.empty())
		{
			player.bones.resize(BoneList::max_bones);
		}

		if (it == existing_players.end() || it->second.bone_transforms != player.bone_transforms)
		{
			players_to_update.push_back(std::ref(player));
		}
		else
		{
			player.bones = it->second.bones;
		}
	}

	if (existing_players.empty())
	{
		players_to_update.clear();
		for (auto& player : new_players)
		{
			players_to_update.push_back(std::ref(player));
		}
	}

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		for (int idx = 0; idx < BoneList::max_bones; ++idx)
		{
			if (player.IsIndexValid(idx))
			{
				dma.AddScatterRead(scatter_handle,
					player.bone_transforms + (0x20 + (static_cast<unsigned long long>(idx) * 0x8)),
					&player.bones[idx].transform,
					sizeof(player.bones[idx].transform)
				);
			}
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		for (auto& bone : player.bones)
		{
			if (bone.transform == 0)
				continue;
			dma.AddScatterRead(scatter_handle,
				bone.transform + 0x10,
				&bone.transform_internal,
				sizeof(bone.transform_internal)
			);
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		for (auto& bone : player.bones)
		{
			if (bone.transform_internal == 0)
				continue;
			dma.AddScatterRead(scatter_handle,
				bone.transform_internal + 0x38,
				&bone.transform_access_readonly,
				sizeof(bone.transform_access_readonly)
			);
			dma.AddScatterRead(scatter_handle,
				bone.transform_internal + 0x40,
				&bone.index,
				sizeof(bone.index)
			);
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		for (auto& bone : player.bones)
		{
			if (bone.transform_access_readonly == 0)
				continue;
			dma.AddScatterRead(scatter_handle,
				bone.transform_access_readonly + 0x18,
				&bone.transform_array,
				sizeof(bone.transform_array)
			);
			dma.AddScatterRead(scatter_handle,
				bone.transform_access_readonly + 0x20,
				&bone.transform_indices,
				sizeof(bone.transform_indices)
			);
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		for (auto& bone : player.bones)
		{
			if (bone.transform_array == 0)
				continue;
			dma.AddScatterRead(scatter_handle,
				bone.transform_array + static_cast<unsigned long long>(0x30) * bone.index,
				&bone.result,
				sizeof(bone.result)
			);
			dma.AddScatterRead(scatter_handle,
				bone.transform_indices + static_cast<unsigned long long>(0x4) * bone.index,
				&bone.transform_index,
				sizeof(bone.transform_index)
			);
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player_ref : players_to_update)
	{
		auto& player = player_ref.get();
		for (auto& bone : player.bones)
		{
			if (bone.matrix_reads.empty())
			{
				int iteration_count = bone.GetIterationCount(bone.transform_index);

				bone.transform_array_reads.reserve(iteration_count);
				int current_transform_index = bone.transform_index;

				for (int iteration = 0; iteration < iteration_count; ++iteration) {
					bone.transform_array_reads.push_back(bone.transform_array + 0x30 * static_cast<unsigned long long>(current_transform_index));
					current_transform_index = dma.Read<int>(bone.transform_indices + 0x4 * static_cast<unsigned long long>(current_transform_index));
				}

				bone.matrix_reads.resize(iteration_count);
			}
		}
	}

	players.store(new_players);
}

void Cache::UpdatePositions(HANDLE scatter_handle)
{
	std::vector<Entity> new_entities = entities.load();

	for (auto& entity : new_entities)
	{
		dma.AddScatterRead(scatter_handle, entity.visual_state + 0x90, &entity.position, sizeof(Vector3));
	}

	dma.ExecuteScatterRead(scatter_handle);

	entities.store(new_entities);
}

void Cache::UpdateBones(HANDLE scatter_handle)
{
	std::vector<Player> new_players = players.load();
	if (new_players.empty())
		return;

	for (auto& player : new_players)
	{
		for (auto& bone : player.bones)
		{
			for (size_t i = 0; i < bone.transform_array_reads.size(); ++i)
			{
				dma.AddScatterRead(scatter_handle, bone.transform_array_reads[i], &bone.matrix_reads[i], sizeof(bone.matrix_reads[i]));
			}
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player : new_players)
	{
		for (auto& bone : player.bones)
		{
			bone.TransformToWorld();
		}
	}

	players.store(new_players);
}

inline std::string Cache::FormatObjectName(const std::string& object_name)
{
	size_t start = object_name.find_last_of('/');
	std::string name = (start == std::string::npos) ? object_name : object_name.substr(start + 1);

	const std::array<std::string_view, 4> suffixes = { ".prefab", ".entity", "_spawned", "deployed" };

	for (const auto& suffix : suffixes) {
		if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
			name.resize(name.size() - suffix.size());
			break;
		}
	}

	std::string result;
	result.reserve(name.size());

	bool capitalize_next = true;
	for (char c : name) {
		if (c == '.') continue;

		if (c == '-' || c == '_') {
			result.push_back(' ');
			capitalize_next = true;
			continue;
		}

		if (std::isdigit(c)) continue;

		if (std::isalpha(c)) {
			if (capitalize_next) {
				result.push_back(std::toupper(c));
				capitalize_next = false;
			}
			else {
				result.push_back(std::tolower(c));
			}
		}
		else {
			result.push_back(c);
		}
	}
	return result;
}