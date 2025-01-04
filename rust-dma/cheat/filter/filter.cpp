#include "../../include.hpp"

EntityCategorie& Filter::GetCategory(const std::string& obj_name)
{
    std::call_once(initialized_flag, PopulateCategories);

    if (IsDroppedItem(obj_name))
    {
        return dropped_items;
    }

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

    base_items.AddPath("assets/prefabs/deployable/advanced lootbox/advancedlootbox.prefab");
    base_items.AddPath("assets/prefabs/deployable/bed/bed_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/dropbox/dropbox.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/gun_rack");
    base_items.AddPath("assets/prefabs/deployable/weaponracks/weaponrack_horizontal.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/assets/prefabs/deployable/weaponracks/weaponrack_single1.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/assets/prefabs/deployable/weaponracks/weaponrack_single2.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/assets/prefabs/deployable/weaponracks/weaponrack_single3.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/assets/prefabs/deployable/weaponracks/weaponrack_stand.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/weaponracks/weaponrack_tall.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/weaponracks/weaponrack_wide.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/large wood storage/box.wooden.large.prefab");
    base_items.AddPath("assets/prefabs/deployable/liquidbarrel/waterbarrel.prefab");
    base_items.AddPath("assets/prefabs/deployable/locker/locker.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/loot barrels/hobo_barrel.prefab");
    base_items.AddPath("assets/prefabs/deployable/mixingtable/mixingtable.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/primitive lootbox/primitivelootbox.prefab");
    base_items.AddPath("assets/prefabs/deployable/ptz security camera/ptz_cctv_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/repair bench/repairbench_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/research table/researchtable_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/search light/searchlight.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/sleeping bag/sleepingbag_leather_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/small stash/small_stash_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/tier 1 workbench/workbench1.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/tier 2 workbench/workbench2.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/tier 3 workbench/workbench3.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/tool cupboard/cupboard.tool.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/vendingmachine/vendingmachine.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/wooden loot crates/");
    base_items.AddPath("assets/prefabs/deployable/woodenbox/woodbox_deployed.prefab");
    base_items.AddPath("assets/prefabs/misc/decor_dlc/storagebarrel");
    base_items.AddPath("assets/prefabs/misc/decor_dlc/rail road planter/railroadplanter.deployed.prefab");
    base_items.AddPath("assets/prefabs/misc/twitch/hobobarrel/hobobarrel.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/planters");
    base_items.AddPath("assets/prefabs/deployable/fridge/fridge.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/bbq/bbq.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/furnace");
    base_items.AddPath("assets/prefabs/deployable/legacyfurnace");
    base_items.AddPath("assets/prefabs/deployable/composter/composter.prefab");
    base_items.AddPath("assets/prefabs/deployable/drone/drone.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/playerioents/electricfurnace/electricfurnace.deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/playerioents/generators");
    base_items.AddPath("assets/prefabs/deployable/playerioents/batteries");
    base_items.AddPath("assets/prefabs/deployable/oil refinery/refinery_small_deployed.prefab");
    base_items.AddPath("assets/prefabs/deployable/oil refinery/refinery_large_deployed.prefab");

    base_construction.AddPath("assets/prefabs/building/door.hinged");
    base_construction.AddPath("assets/prefabs/building/door.double.hinged");
    base_construction.AddPath("assets/prefabs/building/wall.frame.garagedoor/wall.frame.garagedoor.prefab");
    base_construction.AddPath("assets/prefabs/building/floor.ladder.hatch");
    base_construction.AddPath("assets/prefabs/building/floor.triangle.ladder");
    base_construction.AddPath("assets/prefabs/building/ladder.wall.wood");
    base_construction.AddPath("assets/prefabs/building/legacy.shelter.wood/legacy.shelter.wood.deployed.prefab");

    traps.AddPath("assets/prefabs/npc/autoturret/autoturret_deployed.prefab");
    traps.AddPath("assets/prefabs/npc/sam_site_turret/sam_site_turret_deployed.prefab");
    traps.AddPath("assets/prefabs/npc/flame turret/flameturret.deployed.prefab");
    traps.AddPath("assets/prefabs/deployable/bear trap/beartrap.prefab");
    traps.AddPath("assets/prefabs/deployable/floor spikes/spikes.floor.prefab");
    traps.AddPath("assets/prefabs/deployable/landmine/landmine.prefab");
    traps.AddPath("assets/prefabs/deployable/playerioents/teslacoil/teslacoil.deployed.prefab");
    traps.AddPath("assets/prefabs/deployable/single shot trap/guntrap.deployed.prefab");

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
    vehicles.AddPath("assets/content/vehicles/trains/workcart/workcart.entity.prefab");
    vehicles.AddPath("assets/content/vehicles/trains/locomotive/locomotive.entity.prefab");
    vehicles.AddPath("assets/rust.ai/nextai/testridablehorse.prefab");
    vehicles.AddPath("assets/prefabs/deployable/hot air balloon/hotairballoon.prefab");

    npcs.AddPath("assets/rust.ai/agents");

    categories = {
        std::ref(collectable_ores),
        std::ref(food),
        std::ref(hemp),
        std::ref(berries),
        std::ref(ores),
        std::ref(misc),
        std::ref(traps),
		std::ref(base_items),
		std::ref(base_construction),
        std::ref(barrels),
        std::ref(crates),
        std::ref(vehicles),
        std::ref(npcs),
        std::ref(dropped_items)
    };
}

bool Filter::IsDroppedItem(const std::string& obj_name)
{
    return obj_name.size() >= 7 && obj_name.compare(obj_name.size() - 7, 7, "(world)") == 0;
}
