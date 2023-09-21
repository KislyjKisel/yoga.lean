# Yoga.lean

Bindings to the [Yoga](https://github.com/facebook/yoga) layout engine `v2.0.0`.
Not usable for now: work in progress.

## Notes

* Can cause data races if used from multiple threads (even without mutation)
* Uses a submodule to build Yoga, can be run manually using `lake run buildYoga`
* Use option `skipTests` when using as a dependency.
