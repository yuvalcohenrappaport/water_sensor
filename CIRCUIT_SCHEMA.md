# Water Sensor Circuit Schema

## System Overview

```
┌───────────────────────────────────────────────────────────────────┐
│                                                                     │
│                      ESP32-WROOM-32D                               │
│                     ┌──────────────────┐                           │
│                     │                  │                           │
│                GND  │  ●●●●●●●●●●●●●  │  5V                      │
│                 │   │  ●●●●●●●●●●●●●  │   │                      │
│                 │   │                  │   │                      │
│                 │   │  GPIO 34 (IN)    │   │                      │
│                 │   │       ●          │   │                      │
│                 │   └──────────────────┘   │                      │
│                 │          │               │                      │
│                 │          │               │                      │
│                 │          │ (via voltage  │                      │
│                 │          │  divider)     │                      │
│                 │          │               │                      │
│                 │    ┌─────┘               │                      │
│                 │    │                     │                      │
│                 │   [10kΩ] R1             │                      │
│                 │    │                     │                      │
│                 │    ├──── Signal ──────── XKC-Y25-PNP (Yellow)  │
│                 │    │                     │                      │
│                 │   [20kΩ] R2             │                      │
│                 │    │                     │                      │
│                 │    └──── GND            │                      │
│                 │          │               │                      │
│                 └──────────┤               │                      │
│                            │               │                      │
│           XKC-Y25-PNP GND ┘               │                      │
│           (Blue/Black)        XKC-Y25-PNP VCC (Brown/Red)        │
│                                            │                      │
└────────────────────────────────────────────┘                      │
                                                                     │
  ┌──────────────────────────────────┐                               │
  │   XKC-Y25-PNP Sensor            │                               │
  │   (Non-Contact Capacitive)      │                               │
  │                                  │                               │
  │   Brown/Red ── VCC (5V-24V)  ───┼───────────────────────────────┘
  │   Yellow    ── Signal (OUT)     │
  │   Blue/Black── GND             │
  │                                  │
  │   ┌────────────┐                │
  │   │ ~~~~~~~~   │ ← Mount flat  │
  │   │ ~ Water ~  │   against     │
  │   │ ~~~~~~~~   │   tank wall   │
  │   └────────────┘                │
  └──────────────────────────────────┘
```

## Voltage Divider Detail

The XKC-Y25-PNP outputs ~5V HIGH when water is detected. ESP32 GPIOs are
**3.3V tolerant only**, so a voltage divider is **required**:

```
  XKC-Y25-PNP Signal (Yellow) ──┬── [10kΩ R1] ── ESP32 GPIO 34
                                 │
                                [20kΩ R2]
                                 │
                                GND

  Output at GPIO 34: 5V × 20k / (10k + 20k) = 3.33V ✓
```

## Detailed Wiring

### ESP32 WROOM-32D to XKC-Y25-PNP Sensor

| Component | ESP32 Pin | Wire Color (Suggested) | Function |
|-----------|-----------|--------|----------|
| Sensor VCC (Brown/Red) | 5V | Red | Power Supply (5V required) |
| Sensor Signal (Yellow) | GPIO 34 via voltage divider | Yellow | Digital Input (HIGH=water) |
| Sensor GND (Blue/Black) | GND | Black | Ground Reference |
| Voltage Divider R1 (10kΩ) | Between sensor signal & GPIO 34 | — | Level shifting |
| Voltage Divider R2 (20kΩ) | Between GPIO 34 & GND | — | Level shifting |

### ESP32 WROOM-32D Pinout

```
                    ┌─────────────────────────┐
                    │   ESP32-WROOM-32D       │
                    │                         │
      GND     ● ●   │    EN   SVCC            │
      GPIO23  ● ●   │  GPIO5  GPIO17          │
      GPIO22  ● ●   │  GPIO4  GPIO16          │
      GPIO21  ● ●   │  GPIO0  GPIO15          │
      GPIO19  ● ●   │  GPIO2  GPIO14          │
      GPIO18  ● ●   │  GPIO25 GPIO13          │
      GPIO3   ● ●   │  GPIO26 GPIO12          │
      RXD0    ● ●   │  GPIO27 GPIO11          │
      TXD0    ● ●   │  GPIO14 GPIO10          │
      GPIO9   ● ●   │  GPIO9  GPIO9           │
      GPIO10  ● ●   │  GPIO10 GPIO10          │
      GPIO11  ● ●   │  GPIO11 GPIO6           │
      GND     ● ●   │  GND    GPIO7           │
      VUSB    ● ●   │  SVP    GPIO8           │
      GPIO6   ● ●   │  SVN    GPIO19          │
      GPIO7   ● ●   │  GPIO36 GPIO23          │
      GPIO8   ● ●   │  GPIO39 GPIO22          │
      EN      ● ●   │  GPIO34 ⟸ SENSOR PIN   │
      SVCC    ● ●   │  GPIO35 GPIO21          │
      5V      ● ●   │  GPIO32 GPIO20          │
      GND     ● ●   │  GPIO33 GPIO3           │
                    │  GND    RXD0/GPIO1      │
                    │                         │
                    └─────────────────────────┘
```

## Component Specifications

### ESP32-WROOM-32D
- **Operating Voltage**: 3.3V (GPIOs are **NOT** 5V tolerant)
- **GPIO 34**: Input-only pin, used as digital input
- **Features**: WiFi, Bluetooth, 240MHz dual-core processor

### XKC-Y25-PNP Non-Contact Water Level Sensor
- **Operating Voltage**: 5V - 24V DC
- **Output Type**: Digital (PNP — HIGH when water detected, LOW when dry)
- **Detection**: Capacitive, through container wall (up to ~20mm non-metallic)
- **Response Time**: ~500ms
- **Mounting**: Flat against outside of tank/pipe wall (non-metallic only)
- **Wire Colors**: Brown/Red = VCC, Yellow = Signal, Blue/Black = GND

### Connection Notes
- GPIO 34 is an **input-only** pin (no output capability) — perfect for sensor input
- Maximum safe voltage on GPIO 34: **3.6V** — voltage divider is **mandatory** with 5V sensor
- The voltage divider (10kΩ + 20kΩ) brings 5V signal down to ~3.33V (safe for ESP32)
- Mount the XKC-Y25-PNP flat against the tank wall at the desired detection level
- Works through non-metallic walls only (plastic, glass, ceramic) up to ~20mm thick
- Keep signal wires short and away from motor/relay interference

## Power Supply Options

### Option 1: USB Power (Recommended for Testing)
- Connect ESP32 micro-USB port to computer or 5V USB adapter
- Sensor draws minimal power (~10mA)
- Suitable for development and testing

### Option 2: External 5V Supply
- Connect 5V+ to ESP32 5V pin (with USB disconnected)
- Connect GND to ESP32 GND
- Allows standalone operation
- Can add lithium battery with appropriate charging circuit for battery backup

### Option 3: Battery Powered (Long-term Monitoring)
- 2x 3.7V lithium cells in series (~7.4V)
- Voltage regulator to 5V for ESP32
- Enables months of monitoring with solar charging

## Sensor Setup

The XKC-Y25-PNP is a digital sensor — **no calibration needed**.

1. Mount the sensor flat against the outside of the tank/pipe wall
2. Secure it at the water level you want to monitor
3. The sensor has a built-in sensitivity potentiometer (small screw) — adjust if needed
4. Verify via Serial Monitor: should read HIGH with water, LOW without

```
Water present  → Sensor output: HIGH → ESP32 reads HIGH (~3.3V via divider)
No water       → Sensor output: LOW  → ESP32 reads LOW  (0V)
```

## Safety Considerations

1. **Non-Contact Sensor**: The XKC-Y25-PNP mounts outside the tank — no water contact needed
2. **Wall Material**: Only works through non-metallic walls (plastic, glass, ceramic)
3. **Voltage Divider**: REQUIRED — do not connect 5V signal directly to ESP32 GPIO
4. **WiFi**: Ensure adequate antenna clearance (keep away from metal)
5. **Enclosure**: House ESP32 in waterproof container if in a wet environment
6. **Ventilation**: Allow airflow in enclosure to prevent condensation

## Testing Checklist

- [ ] ESP32 USB connection recognized by computer
- [ ] Serial Monitor shows boot messages at 115200 baud
- [ ] Voltage divider wired correctly (measure ~3.3V at GPIO 34 when sensor is HIGH)
- [ ] Sensor reads HIGH when water present, LOW when dry
- [ ] WiFi connects successfully
- [ ] Telegram alert received on water detection
- [ ] Verify automatic reconnection after WiFi loss
