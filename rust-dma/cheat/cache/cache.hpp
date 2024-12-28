#pragma once
#include "../../include.hpp"

class Cache : public CacheData
{
public:
	Cache() {}
	~Cache() {}

	void Run()
	{
		globals_thread.Run();
		entities_thread.Run();
		bones_thread.Run();
		pos_thread.Run();
		bones_update_thread.Run();
	}

	void Stop()
	{
		globals_thread.Stop();
		entities_thread.Stop();
		bones_thread.Stop();
		pos_thread.Stop();
		bones_update_thread.Stop();
	}

	static void UpdateViewMatrix()
	{
		const static HANDLE scatter_handle = dma.CreateScatterHandle();

		Matrix4x4 new_view_matrix;
		Vector3 new_camera_pos;

		dma.AddScatterRead(scatter_handle, camera_object.Get() + Offsets::view_matrix, &new_view_matrix, sizeof(new_view_matrix));
		dma.AddScatterRead(scatter_handle, camera_object.Get() + Offsets::camera_pos, &new_camera_pos, sizeof(new_camera_pos));
 
		dma.ExecuteScatterRead(scatter_handle);

		view_matrix.Set(new_view_matrix);
		camera_pos.Set(new_camera_pos);
	}

private:
	void FetchGlobals(HANDLE scatter_handle);
	void FetchEntities(HANDLE scatter_handle);
	void FetchBones(HANDLE scatter_handle);
	void FetchPositions(HANDLE scatter_handle);
	void UpdateBones(HANDLE scatter_handle);

	std::string FormatObjectName(const std::string& object_name);
	
protected:
	CacheThread globals_thread = CacheThread(std::function<void(HANDLE)>(std::bind(&Cache::FetchGlobals, this, std::placeholders::_1)), 3000);
	CacheThread entities_thread = CacheThread(std::function<void(HANDLE)>(std::bind(&Cache::FetchEntities, this, std::placeholders::_1)), 1000);
	CacheThread bones_thread = CacheThread(std::function<void(HANDLE)>(std::bind(&Cache::FetchBones, this, std::placeholders::_1)), 1000);
	CacheThread pos_thread = CacheThread(std::function<void(HANDLE)>(std::bind(&Cache::FetchPositions, this, std::placeholders::_1)), 10);
	CacheThread bones_update_thread = CacheThread(std::function<void(HANDLE)>(std::bind(&Cache::UpdateBones, this, std::placeholders::_1)), 10);
};

void Cache::FetchGlobals(HANDLE scatter_handle)
{
	uintptr_t base_net_workable = dma.Read<uintptr_t>(base_address.Get() + Offsets::base_net_workable);

	uintptr_t bn_static_fields = dma.Read<uintptr_t>(base_net_workable + 0xB8);

	uintptr_t bn_wrapper_class_ptr = dma.Read<uintptr_t>(bn_static_fields + 0x30);

	uintptr_t bn_wrapper_class = decryption::BaseNetworkable(base_address.Get(), bn_wrapper_class_ptr);

	uintptr_t bn_parent_static_fields = dma.Read<uintptr_t>(bn_wrapper_class + 0x10);

	uintptr_t bn_parent_static_class = decryption::BaseNetworkable(base_address.Get(), bn_parent_static_fields);

	entity_list.Set(dma.Read<uintptr_t>(bn_parent_static_class + 0x18));

	uintptr_t main_camera_manager = dma.Read<uintptr_t>(base_address.Get() + Offsets::main_camera);

	uintptr_t camera_manager = dma.Read<uintptr_t>(main_camera_manager + 0xB8);

	uintptr_t camera = dma.Read<uintptr_t>(camera_manager + 0xC0);

	camera_object.Set(dma.Read<uintptr_t>(camera + 0x10));
}

void Cache::FetchEntities(HANDLE scatter_handle)
{
	const EntityListData entity_list_data = dma.Read<EntityListData>(entity_list.Get() + 0x10);
	if (!entity_list_data)
	{
		FetchGlobals(scatter_handle);
		return;
	}

	std::unordered_map<uintptr_t, Entity> old_entity_map;
	for (const auto& entity : entities.Get())
	{
		old_entity_map[entity.object_ptr] = entity;
	}

	std::unordered_map<uintptr_t, Player> old_player_map;
	for (const auto& player : players.Get())
	{
		old_player_map[player.object_ptr] = player;
	}

	std::vector<Entity> new_entities(entity_list_data.size);
	std::vector<Player> new_players;

	for (uint32_t i = 0; i < entity_list_data.size; i++)
	{
		new_entities[i].idx = i;
		dma.AddScatterRead(scatter_handle, entity_list_data.content + (0x20 + (i * 8)), &new_entities[i].object_ptr, sizeof(new_entities[i].object_ptr));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity : new_entities)
	{
		dma.AddScatterRead(scatter_handle, entity.object_ptr + 0x10, &entity.base_object, sizeof(entity.base_object));
	}
	dma.ExecuteScatterRead(scatter_handle);

	if (new_entities.empty())
	{
		return;
	}

	for (auto& entity : new_entities)
	{
		if (old_entity_map.find(entity.object_ptr) != old_entity_map.end())
		{
			entity = old_entity_map[entity.object_ptr];
			continue;
		}

		dma.AddScatterRead(scatter_handle, entity.base_object + 0x30, &entity.object, sizeof(entity.object));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity : new_entities)
	{
		if (old_entity_map.find(entity.object_ptr) != old_entity_map.end())
			continue;

		dma.AddScatterRead(scatter_handle, entity.object + 0x30, &entity.object_class, sizeof(entity.object_class));
		dma.AddScatterRead(scatter_handle, entity.object + 0x60, &entity.nameptr, sizeof(entity.nameptr));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity : new_entities)
	{
		if (old_entity_map.find(entity.object_ptr) != old_entity_map.end())
			continue;

		dma.AddScatterRead(scatter_handle, entity.object_class + 0x8, &entity.transform, sizeof(entity.transform));
		dma.AddScatterRead(scatter_handle, entity.nameptr, entity.name_buffer, sizeof(entity.name_buffer));
		dma.AddScatterRead(scatter_handle, entity.object + 0x54, &entity.tag, sizeof(entity.tag));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity : new_entities)
	{
		if (old_entity_map.find(entity.object_ptr) != old_entity_map.end())
			continue;

		dma.AddScatterRead(scatter_handle, entity.transform + 0x38, &entity.visual_state, sizeof(entity.visual_state));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity : new_entities)
	{
		if (old_entity_map.find(entity.object_ptr) != old_entity_map.end())
			continue;

		entity.obj_name = entity.name_buffer;
		entity.formatted_name = FormatObjectName(entity.obj_name);
	}

	for (auto& entity : new_entities)
	{
		if (entity.tag == 6)
		{
			Player player(entity);

			if (entity.idx != 0)
			{
				new_players.push_back(player);
			}
			else
			{
				local_player.Set(player);
			}
		}
	}

	entities.Set(new_entities);

	for (auto& player : new_players)
	{
		if (old_player_map.find(player.object_ptr) != old_player_map.end())
		{
			player = old_player_map[player.object_ptr];
			continue;
		}
		dma.AddScatterRead(scatter_handle, player.object_ptr + 0xC8, &player.model, sizeof(player.model));
		dma.AddScatterRead(scatter_handle, player.object_ptr + 0x310, &player.player_model, sizeof(player.player_model));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& player : new_players)
	{
		if (old_player_map.find(player.object_ptr) != old_player_map.end())
		{
			player = old_player_map[player.object_ptr];
			continue;
		}
		dma.AddScatterRead(scatter_handle, player.player_model + 0x2E2, &player.is_npc, sizeof(player.is_npc));
		dma.AddScatterRead(scatter_handle, player.model + 0x50, &player.bone_transforms, sizeof(player.bone_transforms));
	}
	dma.ExecuteScatterRead(scatter_handle);

	players.Set(new_players);
}

void Cache::FetchBones(HANDLE scatter_handle)
{
	std::vector<Player> new_players = players.Get();
	if (new_players.empty())
		return;

	std::vector<bool> needs_fetch(new_players.size(), false);
	size_t total_new_fetches = 0;

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (new_players[i].bone_transforms == 0 || new_players[i].bones.empty())
		{
			needs_fetch[i] = true;
			++total_new_fetches;
		}
	}

	if (total_new_fetches == 0)
		return;

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (new_players[i].bones.empty())
		{
			new_players[i].bones.resize(max_bones);
		}
	}

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (needs_fetch[i])
		{
			for (int idx = 0; idx < max_bones; ++idx)
			{
				if (new_players[i].IsIndexValid(idx))
				{
					dma.AddScatterRead(scatter_handle,
						new_players[i].bone_transforms + (0x20 + (idx * 0x8)),
						&new_players[i].bones[idx].transform,
						sizeof(new_players[i].bones[idx].transform)
					);
				}
			}
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (needs_fetch[i])
		{
			for (auto& bone : new_players[i].bones)
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
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (needs_fetch[i])
		{
			for (auto& bone : new_players[i].bones)
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
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (needs_fetch[i])
		{
			for (auto& bone : new_players[i].bones)
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
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (size_t i = 0; i < new_players.size(); ++i)
	{
		if (needs_fetch[i])
		{
			for (auto& bone : new_players[i].bones)
			{
				if (bone.transform_array == 0)
					continue;

				dma.AddScatterRead(scatter_handle,
					bone.transform_array + 0x30 * bone.index,
					&bone.result,
					sizeof(bone.result)
				);
				dma.AddScatterRead(scatter_handle,
					bone.transform_indices + 0x4 * bone.index,
					&bone.transform_index,
					sizeof(bone.transform_index)
				);
			}
		}
	}
	dma.ExecuteScatterRead(scatter_handle);

	players.Set(new_players);
}

void Cache::FetchPositions(HANDLE scatter_handle)
{
	std::vector<Entity> new_entities = entities.Get();
	Player new_local_player = local_player.Get();

	for (auto& entity : new_entities)
	{
		dma.AddScatterRead(scatter_handle, entity.visual_state + 0x90, &entity.position, sizeof(Vector3));
	}

	dma.AddScatterRead(scatter_handle, new_local_player.visual_state + 0x90, &new_local_player.position, sizeof(Vector3));

	dma.ExecuteScatterRead(scatter_handle);

	entities.Set(new_entities);
	local_player.Set(new_local_player);
}

void Cache::UpdateBones(HANDLE scatter_handle)
{
	std::vector<Player> new_players = players.Get();
	if (new_players.empty())
		return;

	for (auto& player : new_players)
	{
		for (auto& bone : player.bones)
		{
			if (bone.matrix_reads.empty())
			{
				int iteration_count = bone.GetIterationCount(bone.transform_index);

				bone.transform_array_reads.reserve(iteration_count);
				int current_transform_index = bone.transform_index;

				for (int iteration = 0; iteration < iteration_count; ++iteration) {
					bone.transform_array_reads.push_back(bone.transform_array + 0x30 * current_transform_index);
					current_transform_index = dma.Read<int>(bone.transform_indices + 0x4 * current_transform_index);
				}

				bone.matrix_reads.resize(iteration_count);
			}
		}
	}

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
			bone.Transform();
		}
	}

	players.Set(new_players);
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