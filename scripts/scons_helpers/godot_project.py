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
        # Ensure the project cpp subdirectory exists
        cpp_dir = os.path.join(project_directory, "cpp")
        if not os.path.exists(cpp_dir):
            os.makedirs(cpp_dir)
            print(f"Notice: Created the {cpp_dir} directory. You can place your project-specific C++ source files and folders there.")
        
        # Create .gdignore in project/cpp/ if it doesn't exist
        gdignore_path = os.path.join(cpp_dir, ".gdignore")
        if not os.path.exists(gdignore_path):
            with open(gdignore_path, "w") as f:
                pass
            print(f"Notice: Created {gdignore_path} in 'cpp' directory to exclude it from Godot's asset scanning.")
    
        # Create .gitignore in project/cpp/ if it doesn't exist, or ensure it contains bin/
        gitignore_path = os.path.join(cpp_dir, ".gitignore")
        if not os.path.exists(gitignore_path):
            with open(gitignore_path, "w") as f:
                f.write("bin/\n")
            print(f"Notice: Created {gitignore_path} with 'bin/' to exclude build artifacts from git.")
        else:
            with open(gitignore_path, "r+") as f:
                content = f.read()
                if "bin/" not in [line.strip() for line in content.splitlines()]:
                    if content and not content.endswith("\n"):
                        f.write("\n")
                    f.write("bin/\n")
                    print(f"Notice: Added 'bin/' to {gitignore_path}.")
        

    return project_directory
