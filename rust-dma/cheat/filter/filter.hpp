#pragma once
#include "../../include.hpp"

class Filter
{
public:
    Filter() = default;
    ~Filter() = default;

    static EntityCategory& GetCategory(const std::string& obj_name);

    static inline EntityCategory collectable_ores = EntityCategory(ImColor(0.25f, 0.75f, 0.25f, 1.0f), false, "Collectable Ores");
    static inline EntityCategory food = EntityCategory(ImColor(1.0f, 0.5f, 0.0f, 1.0f), false, "Food");
    static inline EntityCategory hemp = EntityCategory(ImColor(0.2f, 0.4f, 1.0f, 1.0f), false, "Hemp");
    static inline EntityCategory berries = EntityCategory(ImColor(0.9f, 0.1f, 0.9f, 1.0f), false, "Berries");
    static inline EntityCategory ores = EntityCategory(ImColor(0.0f, 0.6f, 1.0f, 1.0f), false, "Ores");
    static inline EntityCategory base_items = EntityCategory(ImColor(0.4f, 0.9f, 0.6f, 1.0f), false, "Base Items");
    static inline EntityCategory base_construction = EntityCategory(ImColor(0.5f, 0.2f, 0.7f, 1.0f), false, "Base Construction");
    static inline EntityCategory traps = EntityCategory(ImColor(0.8f, 0.1f, 0.1f, 1.0f), false, "Traps");
    static inline EntityCategory barrels = EntityCategory(ImColor(0.5f, 0.2f, 0.7f, 1.0f), false, "Barrels");
    static inline EntityCategory crates = EntityCategory(ImColor(0.7f, 0.4f, 0.5f, 1.0f), false, "Crates");
    static inline EntityCategory vehicles = EntityCategory(ImColor(0.7f, 0.7f, 0.7f, 1.0f), false, "Vehicles", false);
    static inline EntityCategory npcs = EntityCategory(ImColor(0.9f, 0.1f, 0.1f, 1.0f), false, "NPC's", false);
    static inline EntityCategory dropped_items = EntityCategory(ImColor(0.8f, 0.6f, 0.1f, 1.0f), true, "Dropped Items", false);

    static inline std::vector<std::reference_wrapper<EntityCategory>> categories = {};

private:
    static void PopulateCategories();
	static bool IsDroppedItem(const std::string& obj_name);

    static inline std::unordered_map<std::string, EntityCategory&> category_map;
    static inline EntityCategory default_category{ ImColor(1.0f, 1.0f, 1.0f, 0.0f) };
    static inline std::once_flag initialized_flag;

};