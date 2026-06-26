# TODO — Hard Lockup & Stability Fixes

Reference: `ISSUE-hard-lockup-2026-06-25.md`
Branch: `feature/watchdog-and-stability-fixes`

## Deployed (on Arduino for testing)

- [x] **Fix 1** — Clear `MCUSR` before `wdt_disable()` in `setup()` (AVR132 compliance — hard lockup mitigation)
- [x] **Fix 2** — Throttle MQTT status/uptime/reset publishes to 5s interval + eliminate `String(millis())` heap temps
- [x] **Fix 3** — `long` -> `unsigned long` for `lastReconnectAttempt` and `now` (millis() type match)
- [x] **Fix 4** — Replace `String` topic-building in `transmitDataToMqtt()` with `snprintf()` stack buffer

## Next batch (medium priority — hardening)

- [x] **Fix 5** — Publish free SRAM via MQTT (`arduino/1/freeram`) for runtime diagnostics
- [x] **Fix 6** — Add `Ethernet.maintain()` in `loop()` to clean up stale W5100 sockets
- [ ] **Fix 7** — Check W5100 hardware status after `Ethernet.begin()` + guard against IP parse failure
- [ ] **Fix 8** — Pass-by-reference for `processLine()` and `validateValue()` (eliminate ~30 String copies/cycle)

## Low priority (code quality / deeper refactors)

- [ ] **Fix 9** — Make pattern/key strings `static const` in `PowerSerial` (save ~56 bytes SRAM)
- [ ] **Fix 10** — Replace `String complete` telegram buffer with fixed `char[640]` in `parseMe()`
- [ ] **Fix 11** — Replace `String` member variables (`var_bezug` etc.) with `char[]`
- [ ] **Fix 12** — Remove `extern "C"` block from `main.h` (cargo-culted from Sloeber template)
- [ ] **Fix 13** — `boolean` -> `bool`, remove dead vars (`resetTime`, unused `waitTime`)
- [ ] **Fix 14** — Remove `while (!Serial)` Leonardo guard from `PowerSerial::setup()`
- [ ] **Fix 15** — Fix `tryToRead` counter to count consecutive nulls, not total
- [ ] **Fix 16** — Reduce Serial debug verbosity (per-char prints block UART at 9600 baud)
- [ ] **Fix 17** — Auto-increment patch version (`pVersion` in `main.cpp`) on every commit via git pre-commit hook

## Test status

- **Deploy 1:** 2026-06-25 ~14:56 UTC+2 (Fixes 1-4) — soft lockup after ~1h14m, uptime stalled at 86398601
- **Root cause:** parseMe() blocking 30s×2 exceeded MQTT keepalive 15s → broker disconnect → stale socket → freeze
- **Deploy 2:** 2026-06-26 ~19:3x UTC+2 (Fixes 5,6 + parse timeout 30s→10s + reset interval 24h→6h + freeRam diagnostic)
- **Watching for:** freeRam trend in HA (should stay >4000), MQTT stability, clean 6h reset cycle
