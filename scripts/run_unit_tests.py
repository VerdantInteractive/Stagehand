#!/usr/bin/env python3
import os
import sys
import subprocess
import shlex

def main():
    # ─── Stagehand Unit Test Runner ──────────────────────────────────────────────

    # Setup paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    tests_dir = os.path.join(project_root, "tests", "unit")
    # Adjust extension for Windows if necessary, though SCons usually handles output names
    test_binary_name = "stagehand_tests"
    if os.name == 'nt':
        test_binary_name += ".exe"
    test_binary = os.path.join(tests_dir, "build", test_binary_name)

    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("  Building Stagehand unit tests...")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")

    # Build the test binary
    os.chdir(project_root)

    # Construct SCons command
    cmd = ["scons", "--quiet", "unit_tests"]
    
    target = os.environ.get("TARGET")
    if not target:
        cmd.extend(["debug_symbols=yes", "optimize=debug"])
    else:
        cmd.append(f"target={target}")

    cxx = os.environ.get("CXX")
    if cxx:
        cmd.append(f"CXX={cxx}")

    scons_args = os.environ.get("SCONS_ARGS")
    if scons_args:
        cmd.extend(shlex.split(scons_args))

    # Run build
    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
        sys.exit(1)

    print("")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("  Running Stagehand unit tests...")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")

    # Run the test binary, forwarding all script arguments
    if not os.path.exists(test_binary):
        print(f"Error: Test binary not found at {test_binary}")
        sys.exit(1)

    run_cmd = [test_binary] + sys.argv[1:]
    
    try:
        # On Windows, execv behaves slightly differently, but subprocess is a safe fallback
        if os.name == 'nt':
            sys.exit(subprocess.call(run_cmd))
        else:
            os.execv(test_binary, run_cmd)
    except OSError as e:
        print(f"Error executing test binary: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
