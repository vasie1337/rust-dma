#pragma once
#include "../../include.hpp"

class Filter
{
public:
    Filter() = default;
    ~Filter() = default;

    static EntityCategorie& GetCategory(std::string obj_name);

    static inline EntityCategorie collectable_ores = EntityCategorie(ImColor(0.25f, 0.75f, 0.25f, 1.0f), "Collectable Ores");
    static inline EntityCategorie food = EntityCategorie(ImColor(1.0f, 0.5f, 0.0f, 1.0f), "Food");
    static inline EntityCategorie hemp = EntityCategorie(ImColor(0.2f, 0.4f, 1.0f, 1.0f), "Hemp");
    static inline EntityCategorie berries = EntityCategorie(ImColor(0.9f, 0.1f, 0.9f, 1.0f), "Berries");
    static inline EntityCategorie ores = EntityCategorie(ImColor(0.0f, 0.6f, 1.0f, 1.0f), "Ores");
    static inline EntityCategorie misc = EntityCategorie(ImColor(0.8f, 0.6f, 0.1f, 1.0f), "Misc");
    static inline EntityCategorie barrels = EntityCategorie(ImColor(0.5f, 0.2f, 0.7f, 1.0f), "Barrels");
    static inline EntityCategorie crates = EntityCategorie(ImColor(0.7f, 0.4f, 0.5f, 1.0f), "Crates");
    static inline EntityCategorie vehicles = EntityCategorie(ImColor(0.7f, 0.7f, 0.7f, 1.0f), "Vehicles", false);
    static inline EntityCategorie npcs = EntityCategorie(ImColor(0.9f, 0.1f, 0.1f, 1.0f), "NPC's", false);

    static inline std::vector<std::reference_wrapper<EntityCategorie>> categories = {};

private:
    static void PopulateCategories();

    static inline std::once_flag initialized_flag;
};