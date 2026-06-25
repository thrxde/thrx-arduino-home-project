# Hard Lockup Investigation (2026-06-25)

## Problem

The Arduino Mega 2560 entered a **total hard lockup** on 2026-06-25:
- No MQTT publishing
- No serial output
- DTR reset (serial pin toggle) had **no effect**
- **Physical reset button had no effect**
- Only USB unplug/replug (full power cycle) recovered the device

This is NOT a normal software hang — the WDT (8s, deployed in v1.1.1) should catch any software freeze. This lockup is at the hardware level.

## Timeline

- Device was running v1.1.1 (WDT enabled, all stability fixes from 2026-06-23)
- Uptime at time of lockup: unknown (MQTT was already dead when discovered)
- Recovery: USB unplug → replug → Arduino came back with uptime 0

## Self-Healing Status

An automated recovery system was deployed (HA → SSH → DTR reset on entry):
- Works for software freezes / W5100 Ethernet lockups
- Cannot recover from this hard lockup (DTR/reset pin ineffective)

## Hypotheses for Hard Lockup

### H1: Latch-up condition
- Overcurrent on an I/O pin causes internal CMOS latch-up
- AVR becomes unresponsive to all signals including RESET
- Only power removal clears the latch-up
- Likely if: external wiring (serial meter connection) has voltage spikes

### H2: WDT disabled by code path
- If wdt_disable() is called or WDT config register gets corrupted
- Check: are there any code paths that touch WDTCSR/MCUSR?
- The bootloader may disable WDT on reset — if bootloader is entered but not properly, MCU hangs

### H3: Bootloader loop / corruption
- Arduino Mega has optiboot/stk500v2 bootloader
- If bootloader is entered (e.g., during WDT reset) but UART is busy, it may hang waiting for upload
- The Mega bootloader has a 1s timeout but if flash is partially corrupted it may loop
- Check: Does MCUSR get cleared in setup()? If not, WDT reset flag persists and bootloader may behave differently

### H4: Ethernet W5100 chip pulling RESET low
- W5100 shares the SPI bus and has its own reset circuitry
- If W5100 enters a fault state, it MIGHT hold RESET low (unlikely but check schematic)
- Check: Measure RESET pin voltage during lockup (if reproducible)

### H5: Power supply issue under load
- The Arduino Mega 2560 is powered via USB from entry server
- If USB port current limit is hit (e.g., W5100 + serial driver + LEDs), voltage drops
- Brown-out detection may not be configured, leading to undefined MCU behavior
- Check: What is the BOD fuse setting? (Mega default: BOD at 2.7V)

### H6: Interrupt storm / stack overflow
- If an ISR fires continuously (e.g., SPI interrupt from W5100), WDT never gets fed
- But: WDT interrupt has highest priority on AVR, should still fire
- Unless: stack overflow corrupts the interrupt vector table in RAM
- Check: How much SRAM is used? String operations in powerserial.cpp are suspicious

## Code Areas to Review

Priority HIGH:
- `src/main.cpp` — MCUSR handling in setup(), WDT re-enable after reset, bootloader interaction
- `src/powerserial.cpp` — Stack/heap usage from String operations, blocking loops

Priority MEDIUM:
- `src/main.cpp` — Interrupt usage, SPI conflicts between Ethernet and anything else
- `platformio.ini` — Fuse settings, BOD configuration, bootloader variant
- Hardware — USB power delivery (measure with multimeter if possible)

## Specific Code Questions

1. Is `MCUSR = 0;` called early in setup() BEFORE wdt_disable()? (Required for reliable WDT behavior)
2. Are there any paths where WDT could be accidentally disabled?
3. What is the total SRAM usage? (sizeof all globals + estimated stack depth)
4. Does the W5100 library use interrupts? Could SPI contention cause a hang?
5. Is the serial meter connection optically isolated? (voltage spike protection)

## Recommended Actions

1. **Immediate:** Add `MCUSR = 0; wdt_disable();` as the VERY FIRST thing in setup() (before any other code)
2. **Measure:** USB power draw with multimeter during normal operation
3. **Add:** Free SRAM reporting via MQTT (freeMemory() from MemoryFree.h)
4. **Consider:** Adding a hardware external watchdog (e.g., TPL5010) that power-cycles USB if heartbeat stops
5. **Consider:** Replace String usage in powerserial.cpp with fixed char buffers to prevent heap fragmentation / stack collision
6. **Long-term:** If latch-up is confirmed, add TVS diodes on serial meter input lines

## Recovery Commands

```bash
# Check if Arduino is alive
ssh omv "docker exec mosquitto mosquitto_sub -u openhabian -P mqtt4openhab -t 'arduino/1/#' -C 3 -W 15 -v"

# Trigger DTR reset (works for software hangs, NOT for hard lockup)
ssh entry "/usr/local/bin/arduino-reset.sh"

# Full reflash (if serial works)
ssh entry "cd ~/work/thrx-arduino-home-project && ~/.platformio-venv/bin/pio run -t upload"

# Nuclear option (requires physical access)
# Unplug USB cable from entry server, wait 5s, replug
```

## Update: Second Outage (2026-06-25, ~14:15)

A second outage occurred approximately 12 hours after the USB power cycle recovery:

- **Failure mode:** Soft lockup (MQTT stopped, uptime stalled)
- **Recovery:** Automated DTR reset from HA worked! Device came back on its own.
- **Uptime at failure:** ~12h (43,979,573 ms)
- **Self-healing confirmed working** for this failure type

### Two Distinct Failure Modes Observed Today

| # | Time | Type | Reset button | DTR reset | USB power cycle |
|---|------|------|:---:|:---:|:---:|
| 1 | ~01:00 | Hard lockup (MCU unresponsive) | NO | NO | YES |
| 2 | ~14:15 | Soft lockup (MQTT/Ethernet stall) | n/a | YES (auto) | n/a |

### Implications for Code Review

- The hard lockup (incident 1) points to **hardware-level** issue (latch-up, power, or bootloader)
- The soft lockup (incident 2) points to **software** issue (W5100 timeout, MQTT disconnect loop, heap fragmentation)
- Both happened within 12-13 hours — suggests a **time-based trigger** (millis overflow at 49.7 days is not it, but could be memory leak / fragmentation accumulating over hours)
- The 24h forced reset (`millis() >= 86400000`) may have been masking a 12h failure pattern

### Additional Questions for Code Review

6. Is there a memory leak? Does free SRAM decrease over the 12h runtime?
7. Does the Ethernet library reconnect properly after TCP timeout? (W5100 socket state)
8. Are there any `delay()` calls > WDT timeout (8s) that could be hit in edge cases?
9. PubSubClient `loop()` — what happens when the TCP socket is half-open? Can it block?
