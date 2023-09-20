# Yoga.lean

Bindings to the [Yoga](https://github.com/facebook/yoga) layout engine `v2.0.0`.
Not usable for now: work in progress.

## Notes

* All nodes and configs must be freed manually, use-after-free and double-free will cause IO errors
* Uses a submodule to build Yoga, can be run manually using `lake run buildYoga`
