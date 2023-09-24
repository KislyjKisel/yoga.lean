# Yoga.lean

Bindings to the [Yoga](https://github.com/facebook/yoga) layout engine `v2.0.0`.
Not tested at all, but may be usable.

## Notes

* Can cause data races if used from multiple threads (even without mutation)
* Uses a submodule to build Yoga, can be run manually using `lake run yogaBuild` (`lake run yogaClean` to rm built yoga lib).
* Use option `skipTests` when using as a dependency.
