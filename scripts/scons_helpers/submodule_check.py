import os
import subprocess
import sys


def check_and_init_submodules():
    """Check and optionally initialize required git submodules.

    This uses a list-of-submodules making it easy to extend the set of
    required dependencies. If any listed submodule is missing or empty,
    the user is prompted to run `git submodule update --init` for the
    missing paths.
    """
    # Workspace root (two directories up from this script)
    workspace_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

    # Try to obtain submodule paths from .gitmodules first (recommended).
    missing = []
    candidate_paths = []
    gitmodules_path = os.path.join(workspace_root, ".gitmodules")

    try:
        if os.path.exists(gitmodules_path):
            result = subprocess.run(
                ["git", "config", "--file", gitmodules_path, "--get-regexp", "path"],
                cwd=workspace_root,
                capture_output=True,
                text=True,
            )
            if result.returncode == 0 and result.stdout.strip():
                # output lines look like: "submodule.NAME.path path/to/submodule"
                candidate_paths = [line.split()[-1] for line in result.stdout.splitlines() if line.strip()]
    except Exception:
        candidate_paths = []

    # For each candidate, determine whether git metadata exists (initialized submodule)
    for rel in candidate_paths:
        full_path = os.path.join(workspace_root, rel)
        git_meta_dir = os.path.join(full_path, ".git")
        has_meta = os.path.exists(git_meta_dir)
        exists = os.path.exists(full_path)
        non_empty = exists and bool(os.listdir(full_path))

        if not has_meta:
            # Consider missing or uninitialized if the directory doesn't exist
            # or has no git metadata (common for uninitialized submodules).
            if not exists or not non_empty:
                missing.append(rel)
            else:
                # Directory exists and non-empty but lacks .git — still likely not a proper submodule
                missing.append(rel)

    if missing:
        print("\n" + "=" * 70)
        print("WARNING: Git submodules not initialized")
        print("=" * 70)
        pretty = ", ".join(m + ("/" if not m.endswith("/") else "") for m in missing)
        print(f"The following submodule(s) are missing or empty: {pretty}")
        print("\nThis likely happened because the repository was cloned without the")
        print("'--recursive' flag. You need to initialize the submodules by running:")
        print("\n    git submodule update --init <path1> <path2>\n")

        try:
            response = input("Would you like this script to run that command now? (Y/Yes): ").strip()
            if response.upper() in ("Y", "YES"):
                print("\nInitializing missing submodules...")
                cmd = ["git", "submodule", "update", "--init"] + missing
                result = subprocess.run(cmd, cwd=workspace_root, capture_output=True, text=True)

                if result.returncode == 0:
                    print("✓ Submodules initialized successfully!")
                    if result.stdout:
                        print(result.stdout)
                else:
                    print("✗ Failed to initialize submodules:")
                    if result.stderr:
                        print(result.stderr)
                    sys.exit(1)
            else:
                print("\nPlease run 'git submodule update --init' manually before building.")
                sys.exit(1)
        except (KeyboardInterrupt, EOFError):
            print("\n\nSubmodule initialization cancelled.")
            sys.exit(1)

        print("=" * 70 + "\n")
