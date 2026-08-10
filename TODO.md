# TODO — Stability & Code Quality

Branch: `feature/watchdog-and-stability-fixes`

## Deployed

- [x] **Fix 1** — MQTT debug switch + remove Serial0 flooding (v1.1.3)
- [x] **Fix 1b** — Pass-by-reference for processLine() and validateValue() (v1.1.3)
- [x] **Fix 3** — LED heartbeat on pin 13 (v1.1.3)
- [x] **Fix 4** — F() macro for all string literals (v1.1.3)
- [x] **Fix 5** — Publish free SRAM via MQTT (`arduino/1/freeram`) (v1.1.2)
- [x] **Fix 6** — `Ethernet.maintain()` in loop() (v1.1.2)
- [x] **Fix 9** — Static pattern strings (shared across instances) (v1.1.3)
- [x] **Fix MCUSR** — Clear MCUSR before wdt_disable() per AVR132 (v1.1.1)
- [x] **Fix callback** — Null terminator bug (v1.1.1)
- [x] **Fix MqttHandler** — Pass by reference (v1.1.1)
- [x] **WDT** — 8s hardware watchdog (v1.1.1)
- [x] **Parse timeout** — 10s max for telegram read (v1.1.2)
- [x] **6h reset** — Preventive reset with proper WDT method (v1.1.2)
- [x] **Throttle status** — Publish uptime/status every 5s not every loop (v1.1.1)
- [x] **millis() fix** — Use relative comparison for 6h reset (v1.1.3)
- [x] **main.h cleanup** — Remove extern C, unused declarations (v1.1.3)

## Next (medium priority — deeper refactors)

- [ ] **Fix 2** — Replace String class with char[] buffers in parseMe() (eliminates heap fragmentation entirely)
- [ ] **Fix 7** — Check W5100 hardware status after Ethernet.begin()
- [ ] **Fix 10** — Replace `String complete` telegram buffer with fixed char[1024]
- [ ] **Fix 11** — Replace String member variables (var_bezug etc.) with char[]
- [ ] **Fix 15** — Fix tryToRead counter (count consecutive nulls, not total)
- [ ] **Fix 16** — Add Serial.availableForWrite() guard (safePrint wrapper)

## Low priority (nice-to-have)

- [ ] **Fix 12** — Remove dead variables (resetTime already done, waitTime in PowerSerial)
- [ ] **Fix 13** — `boolean` → `bool` throughout
- [ ] **Fix 17** — Auto-increment version via git hook
- [ ] **PROGMEM** — Move static pattern strings to flash (after String removal)

## Hardware TODO

- [ ] Buy USB 2.0 hub with per-port power switching (Genesys Logic GL850G)
- [ ] Move Arduino USB cable to hub on entry
- [ ] Upgrade reset script: DTR → usbreset → uhubctl power cycle
