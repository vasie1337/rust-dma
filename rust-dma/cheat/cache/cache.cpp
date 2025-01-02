#include "../../include.hpp"

void Cache::Run()
{
	globals_thread.Run();
	entities_thread.Run();
	pos_thread.Run();
	view_thread.Run();

	threads = { globals_thread, entities_thread, pos_thread, view_thread };
}

void Cache::Stop()
{
	globals_thread.Stop();
	entities_thread.Stop();
	pos_thread.Stop();
	view_thread.Stop();
}

void Cache::UpdateViewMatrix(HANDLE scatter_handle)
{
	Matrix4x4 new_view_matrix;
	Vector3 new_camera_pos;

	dma.AddScatterRead(scatter_handle, camera_object.load() + Offsets::view_matrix, &new_view_matrix, sizeof(new_view_matrix));
	dma.AddScatterRead(scatter_handle, camera_object.load() + Offsets::camera_pos, &new_camera_pos, sizeof(new_camera_pos));

	dma.ExecuteScatterRead(scatter_handle);

	view_matrix.store(new_view_matrix);
	camera_pos.store(new_camera_pos);
}

void Cache::FetchGlobals(HANDLE scatter_handle)
{
	if (!entity_list.load())
	{
		uintptr_t base_net_workable = dma.Read<uintptr_t>(base_address.load() + Offsets::base_net_workable);

		uintptr_t bn_static_fields = dma.Read<uintptr_t>(base_net_workable + 0xB8);

		uintptr_t bn_wrapper_class_ptr = dma.Read<uintptr_t>(bn_static_fields + 0x28);

		uintptr_t bn_wrapper_class = decryption::BaseNetworkable(base_address.load(), bn_wrapper_class_ptr);

		uintptr_t bn_parent_static_fields = dma.Read<uintptr_t>(bn_wrapper_class + 0x10);

		uintptr_t bn_parent_static_class = decryption::BaseNetworkable(base_address.load(), bn_parent_static_fields);

		entity_list.store(dma.Read<uintptr_t>(bn_parent_static_class + 0x18));
	}

	static uintptr_t main_camera_manager;
	if (!main_camera_manager)
		main_camera_manager = dma.Read<uintptr_t>(base_address.load() + Offsets::main_camera);

	uintptr_t camera_manager = dma.Read<uintptr_t>(main_camera_manager + 0xB8);

	uintptr_t camera = dma.Read<uintptr_t>(camera_manager + 0xE0);

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

	if (entities_to_update.empty())
	{
		entities.store(new_entities);
		return;
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
		dma.AddScatterRead(scatter_handle, entity.object_ptr + 0xC8, &entity.model, sizeof(entity.model));
	}
	dma.ExecuteScatterRead(scatter_handle);

	for (auto& entity_ref : entities_to_update)
	{
		auto& entity = entity_ref.get();
		entity.obj_name = entity.name_buffer;
		entity.formatted_name = FormatObjectName(entity.obj_name);
		auto& category = Filter::GetCategory(entity.obj_name);
		entity.is_static = category.IsStatic();
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
	if (!players_to_update.empty())
	{
		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			dma.AddScatterRead(scatter_handle, player.object_ptr + 0x258, &player.player_model, sizeof(player.player_model));
			dma.AddScatterRead(scatter_handle, player.object_ptr + 0x270, &player.nameptr, sizeof(player.nameptr));
		}
		dma.ExecuteScatterRead(scatter_handle);

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			dma.AddScatterRead(scatter_handle, player.player_model + 0x2E2, &player.is_npc, sizeof(player.is_npc));
			dma.AddScatterRead(scatter_handle, player.model + 0x50, &player.bone_transforms, sizeof(player.bone_transforms));
			dma.AddScatterRead(scatter_handle, player.nameptr + 0x14, player.name_buffer, sizeof(player.name_buffer));
		}
		dma.ExecuteScatterRead(scatter_handle);

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			player.player_name = player.name_buffer;
			if (player.bones.empty())
			{
				player.bones.resize(BoneList::max_bones);
			}
		}

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			for (int i = 0; i < max_bones; i++)
			{
				if (!player.IsIndexValid(i))
					continue;

				dma.AddScatterRead(scatter_handle,
					player.bone_transforms + (0x20 + (static_cast<unsigned long long>(i) * 0x8)),
					&player.bones[i].address,
					sizeof(player.bones[i].address)
				);
			}
		}
		dma.ExecuteScatterRead(scatter_handle);

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			for (auto& bone_transform : player.bones)
			{
				if (!bone_transform.address)
					continue;

				if (!bone_transform.address_internal)
				{
					dma.AddScatterRead(scatter_handle,
						bone_transform.address + 0x10,
						&bone_transform.address_internal,
						sizeof(bone_transform.address_internal)
					);
				}
			}
		}
		dma.ExecuteScatterRead(scatter_handle);

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			for (auto& bone_transform : player.bones)
			{
				if (!bone_transform.address_internal)
					continue;

				if (!bone_transform.transformAccess)
				{
					dma.AddScatterRead(scatter_handle,
						bone_transform.address_internal + 0x38,
						&bone_transform.transformAccess,
						sizeof(bone_transform.transformAccess)
					);
				}
			}
		}
		dma.ExecuteScatterRead(scatter_handle);

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			for (auto& bone_transform : player.bones)
			{
				if (!bone_transform.transformAccess)
					continue;

				if (!bone_transform.transformArrays)
				{
					dma.AddScatterRead(scatter_handle,
						bone_transform.transformAccess.hierarchyAddr + 0x18,
						&bone_transform.transformArrays,
						sizeof(bone_transform.transformArrays)
					);
				}
			}
		}
		dma.ExecuteScatterRead(scatter_handle);

		for (auto& player_ref : players_to_update)
		{
			auto& player = player_ref.get();
			for (auto& bone_transform : player.bones)
			{
				bone_transform.UpdateTrsXBuffer(scatter_handle);
				bone_transform.UpdateParentIndicesBuffer(scatter_handle);
			}
		}
		dma.ExecuteScatterRead(scatter_handle);
	}

	entities.store(new_entities);
	players.store(new_players);
}

void Cache::UpdatePositions(HANDLE scatter_handle)
{
	std::vector<Entity> new_entities = entities.load();
	std::vector<Player> new_players = players.load();

	for (auto& entity : new_entities)
	{
		if (!entity.visual_state)
			continue;
		if (entity.is_static && !entity.position.invalid())
			continue;

		dma.AddScatterRead(scatter_handle, entity.visual_state + 0x90, &entity.position, sizeof(Vector3));
	}

	for (auto& player : new_players)
	{
		for (auto& bone_transform : player.bones)
		{
			bone_transform.UpdateTrsXBuffer(scatter_handle);
			bone_transform.UpdateParentIndicesBuffer(scatter_handle);
		}
	}

	dma.ExecuteScatterRead(scatter_handle);

	players.store(new_players);
	entities.store(new_entities);
}