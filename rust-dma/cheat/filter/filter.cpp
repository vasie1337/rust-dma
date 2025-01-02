#include "../../include.hpp"

EntityCategorie& Filter::GetCategory(const std::string& obj_name)
{
    std::call_once(initialized_flag, PopulateCategories);

    auto it = std::find_if(categories.begin(), categories.end(), [&obj_name](const auto& category) {
        return category.get().IsEntityInCategory(obj_name);
    });

    if (it != categories.end())
    {
        return *it;
    }

    static EntityCategorie default_category(ImColor(1.0f, 1.0f, 1.0f, 0.0f));
    return default_category;
}

void Filter::PopulateCategories()
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

    barrels.AddPath("assets/bundled/prefabs/radtown/loot_barrel_1.prefab");
    barrels.AddPath("assets/bundled/prefabs/radtown/loot_barrel_2.prefab");
    barrels.AddPath("assets/bundled/prefabs/radtown/oil_barrel.prefab");
    barrels.AddPath("assets/bundled/prefabs/autospawn/resource/loot/loot-barrel-1.prefab");
    barrels.AddPath("assets/bundled/prefabs/autospawn/resource/loot/loot-barrel-2.prefab");

    crates.AddPath("assets/bundled/prefabs/radtown/crate_basic.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_elite.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_mine.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_normal.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_normal_2.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_normal_2_food.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_normal_2_medical.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_tools.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_underwater_advanced.prefab");
    crates.AddPath("assets/bundled/prefabs/radtown/crate_underwater_basic.prefab");

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
        std::ref(barrels),
        std::ref(crates),
        std::ref(vehicles),
        std::ref(npcs)
    };
}