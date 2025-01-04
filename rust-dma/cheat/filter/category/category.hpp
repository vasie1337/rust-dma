#pragma once
#include "../../../include.hpp"

class EntityCategory
{
public:
    EntityCategory() = default;
    EntityCategory(ImColor color, bool enabled = false, std::string name = "Unknown", bool is_obj_static = true)
        : color(color), enabled(enabled), name(name), is_obj_static(is_obj_static)
    {
    }
    ~EntityCategory() = default;

    EntityCategory& operator=(const EntityCategory& other)
    {
        path_trie = other.path_trie;
        enabled = other.enabled;
        color = other.color;
        is_obj_static = other.is_obj_static;
        name = other.name;
        return *this;
    }

    bool IsEntityInCategory(const std::string& obj_name) const
    {
        const TrieNode* current = &path_trie;
        for (char c : obj_name) {
            auto it = current->children.find(c);
            if (it == current->children.end()) {
                return false;
            }
            current = &it->second;
            if (current->is_end) {
                return true;
            }
        }
        return false;
    }

    bool Enabled() const { return enabled; }
    bool IsStatic() const { return is_obj_static; }
    ImColor GetColor() const { return color; }
    std::string Name() const { return name; }

    void AddPath(const std::string& path)
    {
        TrieNode* current = &path_trie;
        for (char c : path) {
            current = &current->children[c];
        }
        current->is_end = true;
    }

	std::vector<std::string> GetPaths() const
	{
		std::vector<std::string> paths;
		std::function<void(const TrieNode&, std::string)> dfs = [&](const TrieNode& node, std::string path) {
			if (node.is_end) {
				paths.push_back(path);
			}
			for (const auto& [c, child] : node.children) {
				dfs(child, path + c);
			}
			};
		dfs(path_trie, "");
		return paths;
	}

private:
    struct TrieNode {
        std::unordered_map<char, TrieNode> children;
        bool is_end = false;
    };

    TrieNode path_trie;

public:
    bool enabled = true;
    bool is_obj_static = true;
    ImColor color;
    std::string name;
};