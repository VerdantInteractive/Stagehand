import json, os, re


def _strip_json_trailing_commas(json_text):
    """
    Removes trailing commas from JSON text to make it parseable by strict JSON parsers.
    Handles commas before closing braces and brackets.
    """
    # Remove commas before closing braces/brackets (with optional whitespace)
    json_text = re.sub(r',(\s*[}\]])', r'\1', json_text)
    return json_text


def sync_cpp_standard(workspace_root, cpp_standard):
    """
    Synchronizes the C++ standard setting across VS Code configuration files.
    
    Args:
        workspace_root: Absolute path to workspace root directory
        cpp_standard: C++ standard version (e.g., "c++23", "c++20")
        
    Prints informational messages when files are updated.
    """
    vscode_dir = os.path.join(workspace_root, ".vscode")
    
    if not os.path.exists(vscode_dir):
        return  # No VS Code configuration to update
    
    # Update c_cpp_properties.json
    cpp_props_path = os.path.join(vscode_dir, "c_cpp_properties.json")
    if os.path.exists(cpp_props_path):
        _update_cpp_properties(cpp_props_path, cpp_standard)
    
    # Update settings.json
    settings_path = os.path.join(vscode_dir, "settings.json")
    if os.path.exists(settings_path):
        _update_settings(settings_path, cpp_standard)


def _update_cpp_properties(file_path, cpp_standard):
    """Updates cppStandard in c_cpp_properties.json configurations."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        content = _strip_json_trailing_commas(content)
        data = json.loads(content)
        
        modified = False
        if "configurations" in data:
            for config in data["configurations"]:
                if "cppStandard" in config and config["cppStandard"] != cpp_standard:
                    config["cppStandard"] = cpp_standard
                    modified = True
        
        if modified:
            with open(file_path, 'w') as f:
                json.dump(data, f, indent=4)
            print(f"Updated C++ standard to '{cpp_standard}' in c_cpp_properties.json")
    
    except (json.JSONDecodeError, OSError) as e:
        print(f"Warning: Could not update c_cpp_properties.json: {e}")


def _update_settings(file_path, cpp_standard):
    """Updates C_Cpp.default.cppStandard in settings.json."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        content = _strip_json_trailing_commas(content)
        data = json.loads(content)
        
        setting_key = "C_Cpp.default.cppStandard"
        current_value = data.get(setting_key)
        
        if current_value != cpp_standard:
            data[setting_key] = cpp_standard
            with open(file_path, 'w') as f:
                json.dump(data, f, indent=4)
            print(f"Updated C++ standard to '{cpp_standard}' in settings.json")
    
    except (json.JSONDecodeError, OSError) as e:
        print(f"Warning: Could not update settings.json: {e}")


def sync_vscode_cpp_standard(cpp_standard):
    """
    Synchronizes the C++ standard setting in VS Code configuration files.
    Automatically determines the workspace root from the script location.
    
    Args:
        cpp_standard: C++ standard version (e.g., "c++23", "c++20")
        
    Prints informational messages when files are updated.
    """
    # Navigate from scripts/scons-helpers/ to workspace root
    workspace_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    sync_cpp_standard(workspace_root, cpp_standard)
