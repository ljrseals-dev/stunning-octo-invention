# Changelog

## 0.1.0 — reconstructed workspace
- Created a C++17/CMake baseline and C ABI.
- Added bounded sample-rate/channel/frame validation.
- Added finite-value parameter validation and strict flat numeric JSON preset parsing.
- Initialized Dolby Stretch coefficient arrays before processing.
- Added preallocated ADT delay storage.
- Added regression smoke tests and build instructions.

## Known gaps
- The supplied original seven source files were not present as downloadable files; this is a reconstruction, not a line-by-line repair.
- De-esser, Dolby Stretch, ADT, compression, warmth and layer handling require listening tests and reference comparisons.
- Parameter smoothing is incomplete; metadata/smoothing table from the original spec was not reproduced.
