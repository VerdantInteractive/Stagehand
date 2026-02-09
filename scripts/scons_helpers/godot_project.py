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
                     Warning: Godot project file not found at '{project_file_path}'. 
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
    
        # Update .gitignore in project/cpp/ with C++ build artifacts block from root .gitignore
        gitignore_path = os.path.join(cpp_dir, ".gitignore")
        
        # Read the block from the project root .gitignore
        # This script is in scripts/scons_helpers/godot_project.py, so root is 3 levels up
        root_gitignore_path = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), ".gitignore")
        
        block_content = []
        start_marker = "### C++ build artifacts."
        end_marker = "### End of C++ build artifacts"

        if os.path.exists(root_gitignore_path):
            in_block = False
            with open(root_gitignore_path, "r") as f:
                for line in f:
                    if line.startswith(start_marker):
                        in_block = True
                    
                    if in_block:
                        block_content.append(line)
                        
                    if in_block and line.startswith(end_marker):
                        in_block = False
                        break
        
        if block_content:
            # Read existing cpp/.gitignore
            cpp_gitignore_lines = []
            if os.path.exists(gitignore_path):
                with open(gitignore_path, "r") as f:
                    cpp_gitignore_lines = f.readlines()
            
            # Reconstruct cpp/.gitignore with updated block
            new_cpp_gitignore_lines = []
            in_old_block = False
            block_inserted = False
            
            for line in cpp_gitignore_lines:
                if line.startswith(start_marker):
                    in_old_block = True
                    new_cpp_gitignore_lines.extend(block_content)
                    block_inserted = True
                
                if not in_old_block:
                    new_cpp_gitignore_lines.append(line)
                
                if in_old_block and line.startswith(end_marker):
                    in_old_block = False
            
            if not block_inserted:
                if new_cpp_gitignore_lines and not new_cpp_gitignore_lines[-1].endswith("\n"):
                    new_cpp_gitignore_lines.append("\n")
                new_cpp_gitignore_lines.extend(block_content)
            
            with open(gitignore_path, "w") as f:
                f.writelines(new_cpp_gitignore_lines)
            
            print(f"Notice: Updated {gitignore_path} with C++ build artifacts block.")
        else:
            print(f"Warning: Could not find C++ build artifacts block in {root_gitignore_path}")
        

    return project_directory
