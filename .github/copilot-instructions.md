- Refer to the .md files in dependencies/flecs/docs directory for Flecs documentation. The .h & .cpp files in the examples/cpp subdirectory demonstrate API usage patterns.
- Use modern C++ language features, up to and including the C++20 standard.
- Use descriptive names for identifiers for variables and similar constructs over short ones.
- Use explicit type names instead of `auto` unless the type names are extremely long.
- Add code comments only when the code is not self-explanatory.
- Don't make any git commits
- Make sure to keep the tests up to date with code changes. If you add new features, add new tests for them. Maintain high test coverage for the project.

# Validation
- After you make modifications, run the "Build (Debug)" task to verify that the project compiles successfully, and scripts/run_tests.sh to verify that all tests pass.
