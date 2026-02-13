#!/usr/bin/env python3
import os
import sys
import subprocess
import platform
import argparse
import shutil

def main():
    # ─── Stagehand Integration Test Runner ───────────────────────────────────────
    # Builds and runs the integration test suite.

    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.dirname(script_dir)
    project_dir = os.path.join(repo_root, "tests", "integration")
    tests_dir = os.path.join(project_dir, "tests")

    # Detect Godot binary
    godot_bin = os.environ.get("GODOT")
    if not godot_bin:
        system = platform.system()
        if system == "Darwin":
            godot_bin = "/Applications/Godot.app/Contents/MacOS/Godot"
        elif system == "Windows":
            godot_bin = "Godot_console.exe"
        else:
            godot_bin = "./bin/godot"

    # Verify Godot binary exists (check path or PATH)
    if not os.path.exists(godot_bin) and not shutil.which(godot_bin):
        print(f"Error: Godot binary not found at '{godot_bin}'", file=sys.stderr)
        sys.exit(1)

    # Parse arguments
    parser = argparse.ArgumentParser(description="Stagehand Integration Test Runner")
    parser.add_argument("-q", "--quiet", action="store_true", help="Suppress output (default)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Show output")
    parser.add_argument("scene", nargs="?", help="Specific scene file to run")
    args = parser.parse_args()

    # Determine verbosity (Default to quiet, verbose flag overrides)
    quiet = True
    if args.verbose:
        quiet = False
    elif args.quiet:
        quiet = True

    print("")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("  Running Stagehand integration tests...")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")

    # Find all integration test scenes
    all_scenes = []
    for root, _, files in os.walk(tests_dir):
        for file in files:
            if file.lower().endswith(".tscn"):
                all_scenes.append(os.path.join(root, file))
    all_scenes.sort(key=os.path.split)

    # Determine which tests to run
    tests_to_run = []
    if args.scene:
        input_path = args.scene
        # Check if absolute path provided
        if os.path.isfile(input_path):
            tests_to_run = [os.path.abspath(input_path)]
        # Check if relative to tests dir
        elif os.path.isfile(os.path.join(tests_dir, input_path)):
            tests_to_run = [os.path.join(tests_dir, input_path)]
        # Check if relative to current working dir
        elif os.path.isfile(os.path.abspath(input_path)):
             tests_to_run = [os.path.abspath(input_path)]
        else:
            print(f"Test scene file '{input_path}' not found. Running all integration test scenes.")
            tests_to_run = all_scenes
    else:
        tests_to_run = all_scenes

    total_tests = 0
    passed_tests = 0
    failed_tests = 0

    for scene_path in tests_to_run:
        total_tests += 1
        try:
            test_name = os.path.relpath(scene_path, tests_dir)
        except ValueError:
            test_name = scene_path
        
        cmd = [
            godot_bin,
            "--headless",
            "--no-header",
            "--quit",
            "--path", project_dir,
            "--scene", scene_path
        ]

        if quiet:
            print(f"Running test: {test_name} ... ", end="", flush=True)
            # Capture output to hide it unless failure
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            
            if result.returncode == 0:
                print("PASSED")
                passed_tests += 1
            else:
                print("FAILED")
                print(f"Re-running test {test_name} with output...")
                # Re-run to show output to user, or just print captured output
                print(result.stdout)
                failed_tests += 1
                break
        else:
            print(f"Running test: {test_name}")
            result = subprocess.run(cmd)
            if result.returncode == 0:
                passed_tests += 1
            else:
                failed_tests += 1
                break

    print("")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("  Summary")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print(f"Total:  {total_tests}")
    print(f"Passed: {passed_tests}")
    print(f"Failed: {failed_tests}")

    if failed_tests > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
