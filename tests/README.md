# Host control checks

Run `python3 tests/run_host_checks.py` from the repository root with a C++ compiler available. Set `CXX` if a different compiler is needed.

The harness includes the actual Arduino sketch and substitutes hardware APIs. It checks deterministic command/state behavior, not real motors or sensors. The separate PlatformIO build compiles the actual Arduino/NeoPixel implementation. See [verification](../docs/verification.md).
