#pragma once
#include "../../include.hpp"

class Filter
{
public:
    Filter() = default;
    ~Filter() = default;

    static EntityCategorie& GetCategory(const std::string& obj_name);

    static inline EntityCategorie collectable_ores = EntityCategorie(ImColor(0.25f, 0.75f, 0.25f, 1.0f), false, "Collectable Ores");
    static inline EntityCategorie food = EntityCategorie(ImColor(1.0f, 0.5f, 0.0f, 1.0f), false, "Food");
    static inline EntityCategorie hemp = EntityCategorie(ImColor(0.2f, 0.4f, 1.0f, 1.0f), false, "Hemp");
    static inline EntityCategorie berries = EntityCategorie(ImColor(0.9f, 0.1f, 0.9f, 1.0f), false, "Berries");
    static inline EntityCategorie ores = EntityCategorie(ImColor(0.0f, 0.6f, 1.0f, 1.0f), false, "Ores");
    static inline EntityCategorie misc = EntityCategorie(ImColor(0.8f, 0.6f, 0.1f, 1.0f), false, "Misc");
    static inline EntityCategorie barrels = EntityCategorie(ImColor(0.5f, 0.2f, 0.7f, 1.0f), false, "Barrels");
    static inline EntityCategorie crates = EntityCategorie(ImColor(0.7f, 0.4f, 0.5f, 1.0f), false, "Crates");
    static inline EntityCategorie vehicles = EntityCategorie(ImColor(0.7f, 0.7f, 0.7f, 1.0f), false, "Vehicles", false);
    static inline EntityCategorie npcs = EntityCategorie(ImColor(0.9f, 0.1f, 0.1f, 1.0f), false, "NPC's", false);
    static inline EntityCategorie dropped_items = EntityCategorie(ImColor(0.8f, 0.6f, 0.1f, 1.0f), true, "Dropped Items", false);

    static inline std::vector<std::reference_wrapper<EntityCategorie>> categories = {};

private:
    static void PopulateCategories();
	static bool IsDroppedItem(const std::string& obj_name);

    static inline std::once_flag initialized_flag;
};