import os
import subprocess
import sys


def check_and_init_submodules():
    """
    Checks if git submodules (flecs/ and godot-cpp/) have been initialized.
    If not, prompts the user to run 'git submodule update --init' and
    optionally runs it for them.
    """
    # Get the workspace root (two directories up from this script)
    workspace_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    
    flecs_path = os.path.join(workspace_root, "dependencies", "flecs")
    godot_cpp_path = os.path.join(workspace_root, "dependencies", "godot-cpp")
    
    # Check if submodule directories are empty or missing
    flecs_empty = not os.path.exists(flecs_path) or not os.listdir(flecs_path)
    godot_cpp_empty = not os.path.exists(godot_cpp_path) or not os.listdir(godot_cpp_path)
    
    if flecs_empty or godot_cpp_empty:
        missing = []
        if flecs_empty:
            missing.append("flecs/")
        if godot_cpp_empty:
            missing.append("godot-cpp/")
        
        print("\n" + "=" * 70)
        print("WARNING: Git submodules not initialized")
        print("=" * 70)
        print(f"The following submodule(s) are missing or empty: {', '.join(missing)}")
        print("\nThis likely happened because the repository was cloned without the")
        print("'--recursive' flag. You need to initialize the submodules by running:")
        print("\n    git submodule update --init\n")
        
        # Prompt user
        try:
            response = input("Would you like this script to run that command now? (Y/Yes): ").strip()
            if response.upper() in ("Y", "YES"):
                print("\nInitializing submodules...")
                result = subprocess.run(
                    ["git", "submodule", "update", "--init"],
                    cwd=workspace_root,
                    capture_output=True,
                    text=True
                )
                
                if result.returncode == 0:
                    print("✓ Submodules initialized successfully!")
                    print(result.stdout)
                else:
                    print("✗ Failed to initialize submodules:")
                    print(result.stderr)
                    sys.exit(1)
            else:
                print("\nPlease run 'git submodule update --init' manually before building.")
                sys.exit(1)
        except (KeyboardInterrupt, EOFError):
            print("\n\nSubmodule initialization cancelled.")
            sys.exit(1)
        
        print("=" * 70 + "\n")
