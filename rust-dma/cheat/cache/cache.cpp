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
	auto camera_address = CacheData::camera_address.load();

	Matrix4x4 new_view_matrix;
	Vector3 new_camera_pos;

	dma.AddScatterRead(scatter_handle, camera_address + Offsets::view_matrix, &new_view_matrix, sizeof(new_view_matrix));
	dma.AddScatterRead(scatter_handle, camera_address + Offsets::camera_pos, &new_camera_pos, sizeof(new_camera_pos));

	dma.ExecuteScatterRead(scatter_handle);

	view_matrix.store(new_view_matrix);
	camera_pos.store(new_camera_pos);
}

void Cache::FetchGlobals(HANDLE scatter_handle)
{
	if (!entity_list_address.load())
	{
		uintptr_t base_net_workable = dma.Read<uintptr_t>(base_address + Offsets::base_net_workable);
		uintptr_t bn_static_fields = dma.Read<uintptr_t>(base_net_workable + 0xB8);
		uintptr_t bn_wrapper_class_ptr = dma.Read<uintptr_t>(bn_static_fields + 0x28);
		uintptr_t bn_wrapper_class = decryption::BaseNetworkable(base_address, bn_wrapper_class_ptr);
		uintptr_t bn_parent_static_fields = dma.Read<uintptr_t>(bn_wrapper_class + 0x10);
		uintptr_t bn_parent_static_class = decryption::BaseNetworkable(base_address, bn_parent_static_fields);
		entity_list_address.store(dma.Read<uintptr_t>(bn_parent_static_class + 0x18));
	}

    static uintptr_t main_camera_manager;
	if (!main_camera_manager)
	{
		main_camera_manager = dma.Read<uintptr_t>(base_address + Offsets::main_camera);
	}

    uintptr_t camera_manager = dma.Read<uintptr_t>(main_camera_manager + 0xB8);
    uintptr_t camera = dma.Read<uintptr_t>(camera_manager + 0xE0);
    camera_address.store(dma.Read<uintptr_t>(camera + 0x10));
}

void Cache::FetchEntities(HANDLE scatter_handle)
{
    const EntityListData entity_list = dma.Read<EntityListData>(entity_list_address.load() + 0x10);
    if (!entity_list) {
        return;
    }

    const auto current_entities = entities.load();
    const auto current_players = players.load();

    std::unordered_map<uintptr_t, Entity> entity_cache;
    std::unordered_map<uintptr_t, Player> player_cache;

    entity_cache.reserve(current_entities.size());
    player_cache.reserve(current_players.size());

    for (const auto& entity : current_entities) {
        entity_cache[entity.object_ptr] = entity;
    }
    for (const auto& player : current_players) {
        player_cache[player.object_ptr] = player;
    }

    std::vector<Entity> new_entities;
    new_entities.reserve(entity_list.size);

    std::vector<Entity*> entities_to_update;
    entities_to_update.reserve(entity_list.size);

    for (uint32_t i = 1; i < entity_list.size; i++) {
        auto& entity = new_entities.emplace_back();
        entity.idx = i;
        dma.AddScatterRead(
            scatter_handle,
            entity_list.content + (0x20 + (static_cast<uint64_t>(i) * 8)),
            &entity.object_ptr,
            sizeof(entity.object_ptr)
        );
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto& entity : new_entities) {
        auto it = entity_cache.find(entity.object_ptr);
        if (it == entity_cache.end()) {
            entities_to_update.push_back(&entity);
        }
        else {
            entity = it->second;
        }
    }

    if (!entities_to_update.empty()) {
        FetchEntityData(scatter_handle, entities_to_update);
    }

    std::vector<Player> new_players;
    new_players.reserve(new_entities.size() / 4);
    std::vector<Player*> players_to_update;
    players_to_update.reserve(new_players.capacity());

    for (const auto& entity : new_entities) {
        if (entity.tag == 6) {
            auto it = player_cache.find(entity.object_ptr);
            if (it == player_cache.end()) {
                auto& player = new_players.emplace_back(entity);
                players_to_update.push_back(&player);
            }
            else {
                new_players.push_back(it->second);
            }
        }
    }

    if (!players_to_update.empty()) {
        FetchPlayerData(scatter_handle, players_to_update);
    }

	new_players.erase(std::remove_if(new_players.begin(), new_players.end(), [](const Player& player) { return player.is_npc; }), new_players.end());

    entities.store(std::move(new_entities));
    players.store(std::move(new_players));
}

void Cache::FetchEntityData(HANDLE scatter_handle, const std::vector<Entity*>& entities_to_update)
{
    for (auto* entity : entities_to_update) {
        dma.AddScatterRead(scatter_handle, entity->object_ptr + 0x10, &entity->base_object, sizeof(entity->base_object));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* entity : entities_to_update) {
        dma.AddScatterRead(scatter_handle, entity->base_object + 0x30, &entity->object, sizeof(entity->object));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* entity : entities_to_update) {
        dma.AddScatterRead(scatter_handle, entity->object + Offsets::object_class, &entity->object_class, sizeof(entity->object_class));
        dma.AddScatterRead(scatter_handle, entity->object + Offsets::tag, &entity->tag, sizeof(entity->tag));
        dma.AddScatterRead(scatter_handle, entity->object + Offsets::prefab_name, &entity->nameptr, sizeof(entity->nameptr));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* entity : entities_to_update) {
        dma.AddScatterRead(scatter_handle, entity->object_class + Offsets::transform, &entity->transform, sizeof(entity->transform));
        dma.AddScatterRead(scatter_handle, entity->nameptr, entity->name_buffer, sizeof(entity->name_buffer));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* entity : entities_to_update) {
        entity->obj_name = std::string(entity->name_buffer);
        entity->formatted_name = Cache::FormatObjectName(entity->obj_name);
        auto& category = Filter::GetCategory(entity->obj_name);
        entity->is_static = category.IsStatic();
    }

    for (auto* entity : entities_to_update) {
        dma.AddScatterRead(scatter_handle, entity->transform + Offsets::visual_state, &entity->visual_state, sizeof(entity->visual_state));
    }
    dma.ExecuteScatterRead(scatter_handle);
}

void Cache::FetchPlayerData(HANDLE scatter_handle, const std::vector<Player*>& players_to_update)
{
    for (auto* player : players_to_update) {
        dma.AddScatterRead(scatter_handle, player->object_ptr + Offsets::player_model, &player->player_model, sizeof(player->player_model));
        dma.AddScatterRead(scatter_handle, player->object_ptr + Offsets::model, &player->model, sizeof(player->model));
        dma.AddScatterRead(scatter_handle, player->object_ptr + Offsets::player_name, &player->nameptr, sizeof(player->nameptr));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        dma.AddScatterRead(scatter_handle, player->player_model + Offsets::is_npc, &player->is_npc, sizeof(player->is_npc));
        dma.AddScatterRead(scatter_handle, player->model + Offsets::bone_transforms, &player->bone_transforms, sizeof(player->bone_transforms));
        dma.AddScatterRead(scatter_handle, player->nameptr + 0x14, player->name_buffer, sizeof(player->name_buffer));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        if (player->is_npc) continue;
        player->player_name = player->name_buffer;
        if (player->bones.empty()) {
            player->bones.resize(BoneList::max_bones);
        }
    }

    FetchPlayerBones(scatter_handle, players_to_update);
}

void Cache::FetchPlayerBones(HANDLE scatter_handle, const std::vector<Player*>& players_to_update)
{
    for (auto* player : players_to_update) {
        if (player->is_npc) continue;
        for (int i = 0; i < max_bones; i++) {
            if (!player->IsIndexValid(i)) continue;
            dma.AddScatterRead(scatter_handle,
                player->bone_transforms + (0x20 + (static_cast<uint64_t>(i) * 0x8)),
                &player->bones[i].address,
                sizeof(player->bones[i].address));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        if (player->is_npc) continue;
        for (auto& bone : player->bones) {
            if (!bone.address || bone.address_internal) continue;
            dma.AddScatterRead(scatter_handle,
                bone.address + 0x10,
                &bone.address_internal,
                sizeof(bone.address_internal));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        if (player->is_npc) continue;
        for (auto& bone : player->bones) {
            if (!bone.address_internal || bone.transformAccess) continue;
            dma.AddScatterRead(scatter_handle,
                bone.address_internal + 0x38,
                &bone.transformAccess,
                sizeof(bone.transformAccess));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        if (player->is_npc) continue;
        for (auto& bone : player->bones) {
            if (!bone.transformAccess || bone.transformArrays) continue;
            dma.AddScatterRead(scatter_handle,
                bone.transformAccess.hierarchyAddr + 0x18,
                &bone.transformArrays,
                sizeof(bone.transformArrays));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        if (player->is_npc) continue;
        for (auto& bone : player->bones) {
            bone.UpdateTrsXBuffer(scatter_handle);
            bone.UpdateParentIndicesBuffer(scatter_handle);
        }
    }
    dma.ExecuteScatterRead(scatter_handle);
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

		dma.AddScatterRead(scatter_handle, entity.visual_state + Offsets::vec3_position, &entity.position, sizeof(Vector3));
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