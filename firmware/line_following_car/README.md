# Firmware source map

Start with [`line_following_car.ino`](line_following_car.ino), renamed from the supplied `Car Code.ino` to match its Arduino sketch directory. Supporting files have been brought into the same directory so Arduino's sketch build can find and compile them.

| File | Role |
|---|---|
| `line_following_car.ino` | State update, line-following/search commands, PWM ramp, LED patterns and serial diagnostics |
| `comm.h` / `comm.cpp` | Two-byte shift-register read and decoding into IR/switch/key fields |
| `motor.h` / `motor.cpp` | Signed PWM conversion, encoder callbacks and a separate legacy velocity/distance helper |
| `Adafruit_NeoPixel.h` / `.cpp` | Supplied third-party RGB LED library snapshot, preserved byte for byte |

From the [repository root](../../README.md), run `pio run -e nanoatmega328`. The bundled library is used as supplied; no current library version is substituted silently.

Read [design notes](../../docs/design.md) before running the controller on real hardware. `motor_step()` is included for completeness but is not called by this sketch. It retains blocking distance loops and historical encoder-handling limitations, so its presence is not evidence of a verified closed-loop controller.

Control constants, pin assignments, state transitions and PWM formulas are unchanged from the supplied source. Chinese comments were translated, line endings normalized, and `<arduino.h>` was corrected to `<Arduino.h>` in the two support files for case-sensitive environments.
