import os
from textwrap import dedent

def check_and_setup_project_file_structure(relative_path):
    """
    Sets up and validates the Godot project directory structure.
    
    Args:
        relative_path: Relative path from this script to the Godot project directory
        
    Returns:
        Absolute path to the project directory
        
    Exits if the project.godot file is not found.
    """
    project_directory = os.path.abspath(relative_path)
    project_file_path = os.path.join(project_directory, "project.godot")
    
    if not os.path.isfile(project_file_path):
        print(dedent(f"""
                     Notice: Godot project file not found at '{project_file_path}'. 
                     Unless this build is for CI/CD, you should ensure that the directory '{project_directory}' contains a valid Godot project and 
                     to integrate Stagehand into your project, its files should be placed in 'addons/stagehand' within the project root.
                     If needed, the base path of the project can be configured in SConstruct by modifying PROJECT_DIRECTORY.\n"""))
    else:
        print(f"✓ Found Godot project file at '{project_file_path}'")
        # Ensure the project cpp subdirectory exists
        cpp_dir = os.path.join(project_directory, "cpp")
        if not os.path.exists(cpp_dir):
            os.makedirs(cpp_dir)
        
        # Create .gdignore in project/cpp/ if it doesn't exist
        gdignore_path = os.path.join(cpp_dir, ".gdignore")
        if not os.path.exists(gdignore_path):
            with open(gdignore_path, "w") as f:
                pass
    
    return project_directory
