# Project Instructions — thrx-arduino-home-project

## 1. Project Overview

- **Purpose:** Home automation electricity meter reader using an Arduino Mega 2560. Reads SML telegrams from two power meters (SWU grid + solar) via RS232 serial and publishes data to MQTT.
- **Framework:** PlatformIO with Arduino framework for `megaatmega2560`.
- **Key Libraries:**
  - `PubSubClient` (~2.7): MQTT communication
  - `Ethernet` (^2.0.2): W5100 Ethernet shield networking
- **Hardware:**
  - Arduino Mega 2560 with W5100 Ethernet Shield
  - Connected via USB (`/dev/ttyACM0`) to entry server (192.168.1.5)
  - Serial2 (9600 7E1): SWU grid meter (SML protocol)
  - Serial3 (9600 7E1): Solar meter (SML protocol)
  - Static IP: 192.168.1.8
  - MQTT broker: 192.168.1.3 (Mosquitto on OMV)

## 2. Directory Structure

```
src/
  main.cpp          - Setup, loop, MQTT connection, 24h reset logic
  main.h            - Includes and function prototypes
  config.h          - Build-time config validation (checks private_config.ini values)
  mqtthandler.h/cpp - MQTT publish wrapper (uses PubSubClient reference)
  powerserial.h/cpp - SML telegram parser + MQTT transmitter
platformio.ini      - PlatformIO project config
private_config.ini  - SECRETS: MQTT credentials, MAC, IPs (NEVER commit/read)
```

## 3. Development Workflow

### Build
```bash
pio run -e megaatmega2560
```

### Upload (from entry server via /dev/ttyACM0)
```bash
pio run -t upload
```

### Serial Monitor
```bash
pio device monitor -p /dev/ttyACM0 -b 9600
```

## 4. MQTT Topics

| Topic | Direction | Description |
|-------|-----------|-------------|
| `arduino/1/status` | Publish | "online" / "offline" / "rebooting" |
| `arduino/1/will` | Publish (LWT) | "connected" / "disconnected" |
| `arduino/1/reset` | Publish | "true" when 24h reset imminent |
| `arduino/1/uptime` | Publish | millis() value (monotonic counter) |
| `arduino/1/command` | Subscribe | Commands: "reboot" triggers HW reset |
| `swu/zaehler/strom/...` | Publish | Grid meter readings |
| `solar/zaehler/strom/...` | Publish | Solar meter readings |

## 5. Coding Conventions

- `camelCase` for variables and functions
- `PascalCase` for classes
- `UPPER_SNAKE_CASE` for constants and macros
- Use `Serial.println()` for debug logging
- Hardware watchdog (WDT) is enabled at 8s — `loop()` must call `wdt_reset()` within 8s
- Any blocking operation must feed the watchdog periodically or use a timeout

## 6. Critical Rules

### DO:
- Read `platformio.ini` and existing code structure before changes
- Keep changes minimal and well-scoped
- Ensure `wdt_reset()` is called in any loop that could take >8 seconds
- Verify build compiles: `pio run`
- Use `private_config.ini` for secrets — add placeholders to a template

### DON'T:
- **NEVER** read/modify/print `private_config.ini` (contains MQTT credentials, network config)
- **NEVER** hardcode secrets in source code
- **NEVER** use `String` concatenation in tight loops (heap fragmentation on 8KB SRAM)
- **NEVER** use software reset (`void(*)(void) = 0`) — always use `hardwareReset()` via WDT
- **NEVER** add blocking loops without a timeout and WDT feeding
- **NEVER** add new libraries without asking first

## 7. Known Constraints

- **8KB SRAM** (Arduino Mega 2560) — minimize heap allocations, avoid `String` in loops
- **W5100 Ethernet chip** — must be properly reset via hardware reset, not software jump
- **Serial ports:** Serial0=debug, Serial2=SWU meter, Serial3=Solar meter
- **24h auto-reset:** Device resets via WDT every 24 hours to prevent millis() drift issues
- **MQTT keepalive:** PubSubClient default 15s — `mqttClient.loop()` must be called regularly
