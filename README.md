# Yoga.lean

Bindings to the [Yoga](https://github.com/facebook/yoga) layout engine `v2.0.0`.
Tested only on Linux and not extensively.

## Configuration

* `skipTests` - do not compile native functions needed only for testing ffi.
* `cCompiler` `cppCompiler` `cppStdlib` - path to c and c++ compilers and cpp stdlib used by yoga and ffi.
  It is recommended to use `clang` (full path due to conflict with Lean's bundled clang),
  `clang++` and `c++` (meaning libc++).

## Notes

* Can cause data races if used from multiple threads (even without mutation)
* Uses a submodule to build Yoga, can be run manually using `lake run buildSubmodule`.
  Use `lake run cleanSubmodule` to delete yoga build.
