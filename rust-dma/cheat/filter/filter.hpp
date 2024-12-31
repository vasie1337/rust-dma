#pragma once
#include "../../include.hpp"

class EntityCategorie
{
public:
	EntityCategorie() = default;
	EntityCategorie(ImColor color, bool is_obj_static = true)
		: color(color), is_obj_static(is_obj_static)
	{
	}
	~EntityCategorie() = default;

	EntityCategorie& operator=(const EntityCategorie& other)
	{
		paths = other.paths;
		enabled = other.enabled;
		color = other.color;
		is_obj_static = other.is_obj_static;
		return *this;
	}

	bool IsEntityInCategory(std::string obj_name) const
	{
		for (const std::string& path : paths)
		{
			if (obj_name.length() >= path.length() && memcmp(obj_name.data(), path.data(), path.length()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	bool Enabled() const
	{
		return enabled && !paths.empty();
	}

	bool IsStatic() const
	{
		return is_obj_static;
	}

	ImColor GetColor() const
	{
		return color;
	}

	void AddPath(const std::string& path)
	{
		paths.push_back(path);
	}

private:
	std::vector<std::string> paths;

public:
	bool enabled = true;
	bool is_obj_static = true;
    ImColor color;
};

class Filter
{
public:
    Filter() = default;
    ~Filter() = default;

    static EntityCategorie& GetCategory(std::string obj_name)
    {
        std::call_once(initialized_flag, PopulateCategories);

        for (auto& category : categories)
        {
            if (category.get().IsEntityInCategory(obj_name))
            {
                return category;
            }
        }

        static EntityCategorie default_category(ImColor(1.0f, 1.0f, 1.0f, 0.0f));
        return default_category;
    }


private:
    static void PopulateCategories()
    {
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/wood_log_pile");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores/metal-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores_sand/metal-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores_snow/metal-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores/stone-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores_sand/stone-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores_snow/stone-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores/sulfur-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores_sand/sulfur-ore.prefab");
        ores.AddPath("assets/bundled/prefabs/autospawn/resource/ores_snow/sulfur-ore.prefab");

        food.AddPath("assets/bundled/prefabs/autospawn/resource/v3_arid_cactus");
        food.AddPath("assets/bundled/prefabs/autospawn/collectable/corn/");
        food.AddPath("assets/bundled/prefabs/autospawn/collectable/mushrooms/");
        food.AddPath("assets/bundled/prefabs/autospawn/collectable/potato/");
        food.AddPath("assets/bundled/prefabs/autospawn/collectable/pumpkin/");

        berries.AddPath("assets/bundled/prefabs/autospawn/collectable/berry-black/");
        berries.AddPath("assets/bundled/prefabs/autospawn/collectable/berry-blue/");
        berries.AddPath("assets/bundled/prefabs/autospawn/collectable/berry-green/");
        berries.AddPath("assets/bundled/prefabs/autospawn/collectable/berry-red/");
        berries.AddPath("assets/bundled/prefabs/autospawn/collectable/berry-white/");
        berries.AddPath("assets/bundled/prefabs/autospawn/collectable/berry-yellow/");

        hemp.AddPath("assets/bundled/prefabs/autospawn/collectable/hemp/");

        collectable_ores.AddPath("assets/bundled/prefabs/autospawn/collectable/stone/metal-collectable.prefab");
        collectable_ores.AddPath("assets/bundled/prefabs/autospawn/collectable/stone/stone-collectable.prefab");
        collectable_ores.AddPath("assets/bundled/prefabs/autospawn/collectable/stone/sulfur-collectable.prefab");
        collectable_ores.AddPath("assets/bundled/prefabs/autospawn/collectable/wood");

        loot_containers.AddPath("assets/bundled/prefabs/autospawn/resource/loot/");
        loot_containers.AddPath("assets/bundled/prefabs/radtown");

        misc.AddPath("assets/content/props/roadsigns/");

        vehicles.AddPath("assets/content/vehicles/modularcar/_base_car_chassis.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/modularcar/2module_car_spawned.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/modularcar/3module_car_spawned.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/modularcar/4module_car_spawned.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/minicopter/minicopter.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/scrap heli carrier/scraptransporthelicopter.prefab");
        vehicles.AddPath("assets/content/vehicles/bikes/motorbike.prefab");
        vehicles.AddPath("assets/content/vehicles/bikes/motorbike_sidecar.prefab");
        vehicles.AddPath("assets/content/vehicles/boats/rowboat/rowboat.prefab");
        vehicles.AddPath("assets/content/vehicles/boats/rhib/rhib.prefab");
        vehicles.AddPath("assets/content/vehicles/boats/kayak/kayak.prefab");
        vehicles.AddPath("assets/content/vehicles/boats/tugboat/tugboat.prefab");
        vehicles.AddPath("assets/content/vehicles/snowmobiles/tomahasnowmobile.prefab");
        vehicles.AddPath("assets/content/vehicles/snowmobiles/snowmobile.prefab");
        vehicles.AddPath("assets/content/vehicles/submarine/submarinesolo.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/submarine/submarineduo.entity.prefab");
        vehicles.AddPath("assets/prefabs/deployable/hot air balloon/hotairballoon.prefab");
        vehicles.AddPath("assets/content/vehicles/trains/workcart/workcart.entity.prefab");
        vehicles.AddPath("assets/content/vehicles/trains/locomotive/locomotive.entity.prefab");
        vehicles.AddPath("assets/rust.ai/nextai/testridablehorse.prefab");

        npcs.AddPath("assets/rust.ai/agents");

        categories = {
            std::ref(collectable_ores),
            std::ref(food),
            std::ref(hemp),
            std::ref(berries),
            std::ref(ores),
            std::ref(misc),
            std::ref(loot_containers),
            std::ref(vehicles),
            std::ref(npcs)
        };
    }

    static inline std::once_flag initialized_flag;
    static inline std::vector<std::reference_wrapper<EntityCategorie>> categories = {};

public:
    static inline EntityCategorie collectable_ores = EntityCategorie(ImColor(0.25f, 0.75f, 0.25f, 1.0f));
    static inline EntityCategorie food = EntityCategorie(ImColor(1.0f, 0.5f, 0.0f, 1.0f)); 
    static inline EntityCategorie hemp = EntityCategorie(ImColor(0.2f, 0.4f, 1.0f, 1.0f)); 
    static inline EntityCategorie berries = EntityCategorie(ImColor(0.9f, 0.1f, 0.9f, 1.0f));
    static inline EntityCategorie ores = EntityCategorie(ImColor(0.0f, 0.6f, 1.0f, 1.0f)); 
    static inline EntityCategorie misc = EntityCategorie(ImColor(0.8f, 0.6f, 0.1f, 1.0f)); 
    static inline EntityCategorie loot_containers = EntityCategorie(ImColor(0.5f, 0.2f, 0.7f, 1.0f));
    static inline EntityCategorie vehicles = EntityCategorie(ImColor(0.7f, 0.7f, 0.7f, 1.0f), false);
    static inline EntityCategorie npcs = EntityCategorie(ImColor(0.9f, 0.1f, 0.1f, 1.0f), false);
};
