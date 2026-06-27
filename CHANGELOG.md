# Changelog

All notable changes to the thrx-arduino-home-project (electricity meter reader).

## [1.1.3] — 2026-06-27

**Root cause fix: ATmega16U2 USB bridge lockup**

The hard lockups (3 in 2 days, only recoverable by USB power cycle) were caused by
Serial0 debug output flooding the ATmega16U2 USB-to-UART bridge chip. The 2560 MCU
was alive (blinking RX LEDs on Serial2/3) but the USB bridge was wedged.

### Added
- MQTT debug switch: send `debug on`/`debug off` to `arduino/1/command`
  - Default OFF — safe for 24/7 operation
  - Publishes current state on `arduino/1/debug` topic
- LED heartbeat on pin 13 (1s blink confirms 2560 alive even if USB dead)

### Changed
- All verbose Serial.print gated behind `debugMode` flag
- Removed per-character debug prints in parseMe() (`"u"`, `"x"`, `"."`)
- Removed per-loop "waiting" messages (were firing thousands of times/sec)
- `processLine()` and `validateValue()` now take `const String&` (eliminates ~36 heap copies/cycle)
- Pattern/key strings now `static` class members (shared across swu/solar instances)
- All string literals use `F()` macro (saves ~200B SRAM)
- 6h reset uses relative millis() comparison (avoids overflow edge case)
- connectMqttServer() debug output gated behind debugMode

### Removed
- `while (!Serial)` Leonardo guard (not needed on Mega 2560)
- `extern "C"` block from main.h (cargo-culted from Sloeber template)
- Unused `resetTime` variable
- ISSUE-*.md files (tracked in homelab docs)

## [1.1.2] — 2026-06-26

### Added
- `arduino/1/freeram` MQTT topic (publishes free SRAM every 5s)
- `Ethernet.maintain()` in loop (cleans stale W5100 sockets)
- Parse timeout reduced from 30s to 10s (two parseMe() calls must fit within MQTT keepalive)

### Changed
- Preventive reset interval: 24h → 6h (reduces long-term resource exhaustion window)

## [1.1.1] — 2026-06-25

### Added
- AVR hardware watchdog (WDT, 8s timeout) — auto-resets MCU if loop() hangs
- WDT-based `hardwareReset()` function (properly resets ALL peripherals including W5100)
- MQTT reboot command (`arduino/1/command` → `"reboot"`)
- MQTT version topic (`arduino/1/version`) — published once on connect
- 30s timeout in `parseMe()` (prevents indefinite blocking on serial)
- `wdt_reset()` calls in serial read loop (feeds WDT during long telegrams)
- Status/uptime publishes throttled to 5s interval

### Fixed
- MCUSR cleared before wdt_disable() (AVR app note AVR132 compliance)
- Callback buffer null terminator bug (was always at index 0)
- MqttHandler passed by reference (PubSubClient state propagates correctly)
- `String(millis())` heap temporaries eliminated (use ltoa + stack buffer)
- Topic building in transmitDataToMqtt uses snprintf stack buffer

### Changed
- Replaced `resetFunc()` (jump to 0x0000) with proper WDT reset
- `long` → `unsigned long` for lastReconnectAttempt and millis() comparisons

## [1.0.0] — Pre-2026

### Features
- Reads SML telegrams from 2 energy meters (Serial2: SWU grid, Serial3: Solar)
- Parses OBIS codes for energy (kWh) and power (W) per phase
- Publishes to MQTT (Mosquitto) on configurable topics
- Ethernet connectivity via W5100 shield
- Configurable via platformio.ini private_config
