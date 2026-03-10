#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "stagehand/registry.h"

namespace {
    const std::unordered_set<std::string> GDSCRIPT_KEYWORDS = {
        "if",     "else",   "elif", "for", "while", "match", "return", "break", "continue", "class", "class_name", "extends", "const", "var",  "func",
        "static", "signal", "enum", "in",  "and",   "or",    "not",    "true",  "false",    "null",  "self",       "super",   "await", "pass",
    };

    std::unordered_set<std::string> extract_builtin_identifiers_from_api_json(const std::string &json_content) {
        std::unordered_set<std::string> builtin_identifiers;

        const std::string marker = "\"builtin_classes\"";
        const size_t marker_index = json_content.find(marker);
        if (marker_index == std::string::npos) {
            return builtin_identifiers;
        }

        const size_t array_start = json_content.find('[', marker_index);
        if (array_start == std::string::npos) {
            return builtin_identifiers;
        }

        int bracket_depth = 0;
        size_t array_end = std::string::npos;
        for (size_t index = array_start; index < json_content.size(); ++index) {
            if (json_content[index] == '[') {
                bracket_depth += 1;
            } else if (json_content[index] == ']') {
                bracket_depth -= 1;
                if (bracket_depth == 0) {
                    array_end = index;
                    break;
                }
            }
        }

        if (array_end == std::string::npos || array_end <= array_start) {
            return builtin_identifiers;
        }

        const std::string builtin_section = json_content.substr(array_start, array_end - array_start + 1);
        const std::regex name_regex("\"name\"\\s*:\\s*\"([^\"]+)\"");
        for (std::sregex_iterator iterator(builtin_section.begin(), builtin_section.end(), name_regex), end; iterator != end; ++iterator) {
            if ((*iterator).size() >= 2) {
                builtin_identifiers.insert((*iterator)[1].str());
            }
        }

        return builtin_identifiers;
    }

    std::unordered_set<std::string> load_builtin_identifiers_from_api_json() {
        const std::vector<std::filesystem::path> candidates = {
            std::filesystem::path("dependencies") / "godot-cpp" / "gdextension" / "extension_api.json",
            std::filesystem::path("..") / "dependencies" / "godot-cpp" / "gdextension" / "extension_api.json",
        };

        for (const std::filesystem::path &candidate : candidates) {
            std::error_code error_code;
            if (!std::filesystem::exists(candidate, error_code) || error_code) {
                continue;
            }

            std::ifstream input_file(candidate, std::ios::binary);
            if (!input_file.is_open()) {
                continue;
            }

            std::string json_content((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
            return extract_builtin_identifiers_from_api_json(json_content);
        }

        return {};
    }

    const std::unordered_set<std::string> &get_gdscript_disallowed_identifiers() {
        static const std::unordered_set<std::string> disallowed = []() {
            std::unordered_set<std::string> identifiers = GDSCRIPT_KEYWORDS;
            const std::unordered_set<std::string> builtin_identifiers = load_builtin_identifiers_from_api_json();
            identifiers.insert(builtin_identifiers.begin(), builtin_identifiers.end());
            return identifiers;
        }();
        return disallowed;
    }

    struct RegistryNode {
        std::map<std::string, RegistryNode> children;
        std::vector<std::pair<std::string, std::string>> constants;
    };

    std::string escape_gd_string(const std::string &value) {
        std::string escaped;
        escaped.reserve(value.size());
        for (const char character : value) {
            if (character == '\\' || character == '"') {
                escaped.push_back('\\');
            }
            escaped.push_back(character);
        }
        return escaped;
    }

    std::vector<std::string> split_path_segments(const std::string &path) {
        std::vector<std::string> segments;
        size_t start_index = 0;

        while (start_index <= path.size()) {
            const size_t separator_index = path.find("::", start_index);
            if (separator_index == std::string::npos) {
                const std::string tail = path.substr(start_index);
                if (!tail.empty()) {
                    segments.push_back(tail);
                }
                break;
            }

            if (separator_index > start_index) {
                segments.push_back(path.substr(start_index, separator_index - start_index));
            }

            start_index = separator_index + 2;
        }

        return segments;
    }

    std::string make_identifier(const std::string &text, const bool preserve_case) {
        std::string identifier;
        identifier.reserve(text.size());

        bool previous_was_underscore = false;
        for (const char character : text) {
            const bool is_lower = character >= 'a' && character <= 'z';
            const bool is_upper = character >= 'A' && character <= 'Z';
            const bool is_digit = character >= '0' && character <= '9';

            if (is_lower) {
                identifier.push_back(preserve_case ? character : static_cast<char>(character - ('a' - 'A')));
                previous_was_underscore = false;
            } else if (is_upper || is_digit) {
                identifier.push_back(character);
                previous_was_underscore = false;
            } else if (!previous_was_underscore) {
                identifier.push_back('_');
                previous_was_underscore = true;
            }
        }

        while (!identifier.empty() && identifier.back() == '_') {
            identifier.pop_back();
        }

        if (identifier.empty()) {
            identifier = "ENTITY";
        }
        if (identifier.front() >= '0' && identifier.front() <= '9') {
            identifier = "_" + identifier;
        }

        return identifier;
    }

    std::string make_unique_name(const std::string &base_name, std::unordered_map<std::string, int> &used_names) {
        int &count = used_names[base_name];
        count += 1;
        if (count == 1) {
            return base_name;
        }
        return base_name + "_" + std::to_string(count);
    }

    std::string sanitize_gdscript_identifier(std::string identifier) {
        if (identifier.empty()) {
            return "ENTITY";
        }

        const std::unordered_set<std::string> &disallowed_identifiers = get_gdscript_disallowed_identifiers();
        while (disallowed_identifiers.contains(identifier)) {
            identifier += "_";
        }

        return identifier;
    }

    void insert_entry_into_tree(RegistryNode &root, const std::string &full_path) {
        std::vector<std::string> segments = split_path_segments(full_path);
        if (segments.empty()) {
            return;
        }

        RegistryNode *current = &root;
        std::unordered_map<std::string, int> sibling_class_names;
        for (size_t index = 0; index + 1 < segments.size(); ++index) {
            std::string class_name = make_identifier(segments[index], true);
            class_name = sanitize_gdscript_identifier(class_name);
            class_name = make_unique_name(class_name, sibling_class_names);
            current = &current->children[class_name];
            sibling_class_names.clear();
        }

        std::string constant_name = make_identifier(segments.back(), true);
        constant_name = sanitize_gdscript_identifier(constant_name);

        std::unordered_map<std::string, int> constant_names;
        for (const std::pair<std::string, std::string> &pair : current->constants) {
            constant_names[pair.first] += 1;
        }
        constant_name = make_unique_name(constant_name, constant_names);

        current->constants.emplace_back(constant_name, full_path);
    }

    void append_tree_node(const RegistryNode &node, const std::string &indent, std::string &output) {
        if (node.children.empty() && node.constants.empty()) {
            output += indent + "const NONE = \"\"\n";
            return;
        }

        for (const std::pair<std::string, RegistryNode> &child_pair : node.children) {
            output += indent + "class " + child_pair.first + ":\n";
            append_tree_node(child_pair.second, indent + "\t", output);
            output += "\n";
        }

        for (const std::pair<std::string, std::string> &constant_pair : node.constants) {
            output += indent + "const " + constant_pair.first + " = \"" + escape_gd_string(constant_pair.second) + "\"\n";
        }
    }

    std::string build_registry_script(const std::vector<stagehand::RegisteredEntityInfo> &entries) {
        std::string output;
        output += "# This file is auto-generated by stagehand/utilities/generators/ecs_registry.cpp\n";
        output += "# Do not edit manually; regenerate by rebuilding the extension.\n\n";
        output += "class_name ECS\n";
        output += "extends Object\n\n";

        std::vector<stagehand::RegisteredEntityInfo> components;
        std::vector<stagehand::RegisteredEntityInfo> prefabs;
        std::vector<stagehand::RegisteredEntityInfo> systems;
        components.reserve(entries.size());
        prefabs.reserve(entries.size());
        systems.reserve(entries.size());

        for (const stagehand::RegisteredEntityInfo &entry : entries) {
            if (entry.is_component) {
                components.push_back(entry);
            }
            if (entry.is_prefab) {
                prefabs.push_back(entry);
            }
            if (entry.is_system) {
                systems.push_back(entry);
            }
        }

        auto append_namespace_class = [&output](const char *class_name, const std::vector<stagehand::RegisteredEntityInfo> &class_entries) {
            output += "class ";
            output += class_name;
            output += ":\n";

            if (class_entries.empty()) {
                output += "\tconst NONE = \"\"\n\n";
                return;
            }

            RegistryNode root;
            for (const stagehand::RegisteredEntityInfo &entry : class_entries) {
                insert_entry_into_tree(root, entry.path);
            }

            append_tree_node(root, "\t", output);
            output += "\n";
        };

        append_namespace_class("components", components);
        append_namespace_class("prefabs", prefabs);
        append_namespace_class("systems", systems);

        output += "const SCHEMA := {\n";
        output += "\t\"components\": {\n";
        for (const stagehand::RegisteredEntityInfo &entry : components) {
            output += "\t\t\"";
            output += escape_gd_string(entry.path);
            output += "\": {\"name\": \"";
            output += escape_gd_string(entry.name);
            output += "\", \"namespace\": \"";
            output += escape_gd_string(entry.namespace_path);
            output += "\", \"data_type\": \"";
            output += escape_gd_string(entry.component_data_type);
            output += "\", \"is_change_detection_tag\": ";
            output += entry.is_change_detection_tag ? "true" : "false";
            output += "},\n";
        }
        output += "\t},\n";

        output += "\t\"prefabs\": {\n";
        for (const stagehand::RegisteredEntityInfo &entry : prefabs) {
            output += "\t\t\"";
            output += escape_gd_string(entry.path);
            output += "\": {\"name\": \"";
            output += escape_gd_string(entry.name);
            output += "\", \"namespace\": \"";
            output += escape_gd_string(entry.namespace_path);
            output += "\"},\n";
        }
        output += "\t},\n";

        output += "\t\"systems\": {\n";
        for (const stagehand::RegisteredEntityInfo &entry : systems) {
            output += "\t\t\"";
            output += escape_gd_string(entry.path);
            output += "\": {\"name\": \"";
            output += escape_gd_string(entry.name);
            output += "\", \"namespace\": \"";
            output += escape_gd_string(entry.namespace_path);
            output += "\"},\n";
        }
        output += "\t},\n";

        output += "}\n";

        return output;
    }
} // namespace

int main(int argc, char **argv) {
    std::string output_path = "generated/ecs_registry.gd";
    if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0') {
        output_path = argv[1];
    }

    flecs::world world;
    stagehand::register_components_and_systems_with_world(world);

    const std::vector<std::string> module_names = stagehand::get_registered_module_names();
    for (const std::string &module_name : module_names) {
        stagehand::run_module_callbacks_for(world, module_name);
    }

    const std::vector<stagehand::RegisteredEntityInfo> entries = stagehand::collect_registered_entities(world, false);
    const std::string script = build_registry_script(entries);

    const std::filesystem::path output_file_path = output_path;
    const std::filesystem::path output_directory = output_file_path.parent_path();
    if (!output_directory.empty()) {
        std::error_code directory_error;
        std::filesystem::create_directories(output_directory, directory_error);
        if (directory_error) {
            std::cerr << "Failed to create output directory: " << output_directory.string() << "\n";
            return 1;
        }
    }

    std::ofstream output_file(output_file_path, std::ios::binary | std::ios::trunc);
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output file: " << output_file_path.string() << "\n";
        return 1;
    }

    output_file << script;
    output_file.flush();
    if (!output_file.good()) {
        std::cerr << "Failed to write output file: " << output_file_path.string() << "\n";
        return 1;
    }

    std::cout << "Generated ECS registry script: " << output_file_path.string() << "\n";
    return 0;
}
