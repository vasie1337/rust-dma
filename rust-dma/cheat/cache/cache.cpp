#include "../../include.hpp"

void Cache::Run()
{
    cached_entities.reserve(1000);
    cached_players.reserve(128);
    entity_ptr_map.reserve(1000);
    player_ptr_map.reserve(128);
}

void Cache::Stop()
{
    cached_entities.clear();
    cached_players.clear();
    entity_ptr_map.clear();
    player_ptr_map.clear();
}

void Cache::TickCache()
{
    static auto scatter_handle = dma.CreateScatterHandle();

	FetchGlobals(scatter_handle);
	FetchEntities(scatter_handle);
	UpdateFrame(scatter_handle);
	UpdateView(scatter_handle);
}

void Cache::FetchGlobals(HANDLE scatter_handle)
{
    if (!RateLimiter::get_globals().should_run())
        return;

    if (!scatter_handle)
        return;

    if (!entity_list_address)
    {
        uintptr_t base_net_workable = dma.Read<uintptr_t>(base_address + Offsets::base_net_workable);
        uintptr_t bn_static_fields = dma.Read<uintptr_t>(base_net_workable + 0xB8);
        uintptr_t bn_wrapper_class_ptr = dma.Read<uintptr_t>(bn_static_fields + 0x38);
        uintptr_t bn_wrapper_class = decryption::BaseNetworkable(base_address, bn_wrapper_class_ptr);
        uintptr_t bn_parent_static_fields = dma.Read<uintptr_t>(bn_wrapper_class + 0x10);
        uintptr_t bn_parent_static_class = decryption::DecryptList(base_address, bn_parent_static_fields);
        entity_list_address = dma.Read<uintptr_t>(bn_parent_static_class + 0x20);
    }

    static uintptr_t main_camera_manager;
    if (!main_camera_manager)
    {
        main_camera_manager = dma.Read<uintptr_t>(base_address + Offsets::main_camera);
    }

    uintptr_t camera_manager = dma.Read<uintptr_t>(main_camera_manager + 0xB8);
    uintptr_t camera = dma.Read<uintptr_t>(camera_manager + 0x98);
    camera_address = dma.Read<uintptr_t>(camera + 0x10);
}

void Cache::FetchEntities(HANDLE scatter_handle)
{
    if (!RateLimiter::get_entities().should_run())
        return;

    if (!scatter_handle)
        return;

    const EntityListData entity_list = dma.Read<EntityListData>(entity_list_address + 0x10);
    if (!entity_list) {
        return;
    }

    entity_ptr_map.clear();
    player_ptr_map.clear();
    
    for (auto& entity : frame_data.entities) {
        entity_ptr_map[entity.object_ptr] = &entity;
    }
    for (auto& player : frame_data.players) {
        player_ptr_map[player.object_ptr] = &player;
    }

    cached_entities.clear();
    cached_entities.reserve(entity_list.size);
    
    std::vector<Entity*> entities_to_update;
    entities_to_update.reserve(entity_list.size);

    for (uint32_t i = 1; i < entity_list.size; i++) {
        cached_entities.emplace_back();
        Entity& entity = cached_entities.back();
        entity.idx = i;
        dma.AddScatterRead(scatter_handle, entity_list.content + (0x20 + (static_cast<uint64_t>(i) * 8)), &entity.object_ptr, sizeof(entity.object_ptr));
    }

    dma.ExecuteScatterRead(scatter_handle);
    
    for (auto& entity : cached_entities) {
        if (entity_ptr_map.find(entity.object_ptr) == entity_ptr_map.end()) {
            entities_to_update.push_back(&entity);
        }
        else {
            entity = *entity_ptr_map[entity.object_ptr];
        }
    }

    if (!entities_to_update.empty()) {
        FetchEntityData(scatter_handle, entities_to_update);
    }

    cached_players.clear();
    std::vector<Player*> players_to_update;
    players_to_update.reserve(cached_entities.size() / 4);

    for (auto& entity : cached_entities) {
        if (entity.tag == 6) {
            if (player_ptr_map.find(entity.object_ptr) == player_ptr_map.end()) {
                cached_players.emplace_back(entity);
                players_to_update.push_back(&cached_players.back());
            }
            else {
                cached_players.push_back(*player_ptr_map[entity.object_ptr]);
            }
        }
    }

    if (!players_to_update.empty()) {
        FetchPlayerData(scatter_handle, players_to_update);
    }

    frame_data.entities = std::move(cached_entities);
    frame_data.players = std::move(cached_players);
    
    cached_entities = frame_data.entities;
    cached_players = frame_data.players;
}

void Cache::FetchEntityData(HANDLE scatter_handle, std::vector<Entity*>& entities_to_update)
{
    if (entities_to_update.empty())
        return;

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

void Cache::FetchPlayerData(HANDLE scatter_handle, std::vector<Player*>& players_to_update)
{
    if (players_to_update.empty())
        return;

    for (auto* player : players_to_update) {
        dma.AddScatterRead(scatter_handle, player->object_ptr + Offsets::player_model, &player->player_model, sizeof(player->player_model));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        dma.AddScatterRead(scatter_handle, player->player_model + Offsets::is_npc, &player->is_npc, sizeof(player->is_npc));
    }
    dma.ExecuteScatterRead(scatter_handle);

    players_to_update.erase(std::remove_if(players_to_update.begin(), players_to_update.end(), [](const Player* player) { return player->is_npc; }), players_to_update.end());

    for (auto* player : players_to_update) {
        dma.AddScatterRead(scatter_handle, player->object_ptr + Offsets::model, &player->model, sizeof(player->model));
        dma.AddScatterRead(scatter_handle, player->object_ptr + Offsets::player_name, &player->nameptr, sizeof(player->nameptr));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        dma.AddScatterRead(scatter_handle, player->model + Offsets::bone_transforms, &player->bone_transforms, sizeof(player->bone_transforms));
        dma.AddScatterRead(scatter_handle, player->nameptr + 0x14, player->name_buffer, sizeof(player->name_buffer));
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        player->player_name = player->name_buffer;
        if (player->bones.empty()) {
            player->bones.reserve(BoneList::max_bones);
            player->bones.resize(BoneList::max_bones);
        }
    }

    FetchPlayerBones(scatter_handle, players_to_update);
}

void Cache::FetchPlayerBones(HANDLE scatter_handle, std::vector<Player*>& players_to_update)
{
    if (players_to_update.empty())
        return;

    for (auto* player : players_to_update) {
        for (int i = 0; i < max_bones; i++) {
            if (!player->IsIndexValid(i))
                continue;

            dma.AddScatterRead(scatter_handle, player->bone_transforms + (0x20 + (static_cast<uint64_t>(i) * 0x8)), &player->bones[i].address, sizeof(player->bones[i].address));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        for (auto& bone : player->bones) {
            if (bone.address == 0)
                continue;
                
            dma.AddScatterRead(scatter_handle, bone.address + 0x10, &bone.address_internal, sizeof(bone.address_internal));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        for (auto& bone : player->bones) {
            if (bone.address_internal == 0)
                continue;
                
            dma.AddScatterRead(scatter_handle, bone.address_internal + 0x38, &bone.transformAccess, sizeof(bone.transformAccess));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        for (auto& bone : player->bones) {
            if (bone.transformAccess.hierarchyAddr == 0)
                continue;
                
            dma.AddScatterRead(scatter_handle, bone.transformAccess.hierarchyAddr + 0x18, &bone.transformArrays, sizeof(bone.transformArrays));
        }
    }
    dma.ExecuteScatterRead(scatter_handle);

    for (auto* player : players_to_update) {
        for (auto& bone : player->bones) {
            bone.UpdateTrsXBuffer(scatter_handle);
            bone.UpdateParentIndicesBuffer(scatter_handle);
        }
    }
    
    dma.ExecuteScatterRead(scatter_handle);
}

void Cache::UpdateFrame(HANDLE scatter_handle)
{
    if (!RateLimiter::get_frame().should_run())
        return;

    if (!scatter_handle)
        return;

    size_t position_reads = 0;
    for (auto& entity : frame_data.entities)
    {
        if (!entity.visual_state)
            continue;
        if (entity.is_static && !entity.position.invalid())
            continue;
        
        position_reads++;
    }

    if (position_reads > 0)
    {
        for (auto& entity : frame_data.entities)
        {
            if (!entity.visual_state)
                continue;
            if (entity.is_static && !entity.position.invalid())
                continue;

            dma.AddScatterRead(scatter_handle, entity.visual_state + Offsets::vec3_position, &entity.position, sizeof(entity.position));
        }
    }

    for (auto& player : frame_data.players)
    {
        for (auto& bone_transform : player.bones)
        {
            bone_transform.UpdateTrsXBuffer(scatter_handle);
            bone_transform.UpdateParentIndicesBuffer(scatter_handle);
        }
    }

    dma.ExecuteScatterRead(scatter_handle);

    for (auto& player : frame_data.players)
    {
        for (auto& bone_transform : player.bones)
        {
            bone_transform.CalculatePosition();
        }
    }
}

void Cache::UpdateView(HANDLE scatter_handle)
{
    dma.AddScatterRead(scatter_handle, camera_address + Offsets::view_matrix, &frame_data.view_matrix, sizeof(frame_data.view_matrix));
    dma.AddScatterRead(scatter_handle, camera_address + Offsets::camera_pos, &frame_data.camera_pos, sizeof(frame_data.camera_pos));
    dma.ExecuteScatterRead(scatter_handle);
}
