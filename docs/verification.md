# Verification record

Checks were performed during repository preparation on October 15, 2026. No physical car was connected or flashed.

## Firmware build

```bash
pio run -e nanoatmega328
```

| Setting/result | Recorded value |
|---|---|
| PlatformIO Core | 6.2.0 |
| Atmel AVR platform | 5.3.0 |
| Board | `nanoatmega328` |
| Arduino AVR framework package | 5.4.0 |
| AVR compiler | 7.3.0 |
| RAM | 326 / 2048 bytes (15.9%) |
| Flash | 5714 / 30720 bytes (18.6%) |
| Result | Successful compilation, linking and HEX generation |

The build includes the supplied NeoPixel files. Existing warnings remain for an unused `lastLineState`, initialized `extern` pulse counters and signed/unsigned comparisons inside the legacy distance helper. They were recorded rather than hidden by changing the controller.

The PlatformIO configuration is a new build entry based on the report's Nano identification; it is not an invented original development lockfile. The platform selector remains unpinned, and the table records the actual resolved environment.

## Host control checks

```bash
python3 tests/run_host_checks.py
```

Six scenario groups passed against the actual sketch functions:

1. Centered-line equal forward PWM.
2. Left/right outer-sensor sharp-turn targets.
3. Distinct inner/middle-sensor corrections.
4. The strict `>500 ms` transition to search and the absence of an automatic LOST transition.
5. Line reacquisition restoring FOLLOWING_LINE.
6. Per-iteration ramp limiting and target non-overshoot.

The harness provides minimal time, serial, motor and LED stubs and emulates Arduino's generated function declarations. It does not copy the controller into a separate implementation. A C++ host compiler and Python 3 are sufficient; binaries are created in a temporary directory.

## Source integrity and preparation

All seven meaningful source files from the supplied archive are retained. The sketch/support files were co-located, Arduino header casing was made portable, and Chinese comments/line endings were normalized. The two Adafruit files are byte-identical to the supplied copies. Original state transitions, numeric tuning, sensor decoding, pin assignments and motor formulas were preserved.

The source report/manual were read for context but are not uploaded. No cover-page identifiers, signatures, per-person contribution percentages or unrelated digital-clock/cat-house material are included.

## Not established by these checks

- Physical sensor polarity, board revision or actual motor wiring.
- Track completion, lap time, turning accuracy or measured vehicle speed.
- Encoder ISR timing, wheel-slip effects or the legacy distance helper's reliability.
- Collision avoidance, a search timeout or a fully automatic LOST stop.

Those behaviors require physical tests or additional implementation. No such result is inferred from a successful compile or a qualitative report description.
