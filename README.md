# Vocal Hi-Fi — C++17 workspace

A framework-free, offline-buildable reconstruction workspace for the Vocal Hi-Fi processor.

## Build

```sh
cmake -S . -B build -DVP_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

No network access or dependency downloads are used by CMake.

## API

The public C ABI is in `include/vocal_hifi_abi.h`. Call `vp_create`, `vp_prepare`, configure parameters, then call planar or interleaved processing. Preset loading accepts a strict flat JSON object of numeric parameter values. The host must quiesce the audio callback while preparing or changing parameters/loading presets.

## DSP chain represented

Input gain and DC/high-pass filtering; mud, presence, air and 2.6 kHz Q4 artifact shaping; asymmetric warmth; de-essing; optional Dolby Stretch; bus ADT; bus compression/warmth/output gain and optional soft clipping.

## Important engineering status

This is a newly generated, buildable baseline reconstructed from the Vocal Hi-Fi requirements discussed in the conversation, not a recovery of the original seven submitted files. It deliberately does not claim feature parity with the unavailable source. The current de-esser is a simplified level-dependent attenuation stage, not a true split-band crossover implementation. `breath_lift` and `layer_gain` are accepted parameters but are not yet applied as independent processing stages. The ADT is a short cross-feedback delay approximation. Multi-layer summing is not exposed by this minimal ABI.

The callback path performs no dynamic allocation, locking, logging, file I/O, or network access. This is a source-level design statement, not a formal real-time certification. Host-side quiescence is required for parameter changes. Coefficient generation currently occurs at the processing boundary and should be moved to a control-side staging model before low-latency production use.

## License

MIT. See `LICENSE`.
