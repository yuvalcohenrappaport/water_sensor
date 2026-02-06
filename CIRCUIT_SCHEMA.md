# Water Sensor Circuit Schema

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                                                               │
│                    ESP32-WROOM-32D                           │
│                   ┌──────────────────┐                       │
│                   │                  │                       │
│              GND  │  ●●●●●●●●●●●●●  │  3V3/5V              │
│               │   │  ●●●●●●●●●●●●●  │    │                 │
│               │   │                  │    │                 │
│               │   │  GPIO 34 (ADC0)  │    │                 │
│               │   │       ●          │    │                 │
│               │   └──────────────────┘    │                 │
│               │                           │                 │
│               │                           │                 │
│               └───────┬─────────────────────┘               │
│                       │                                      │
│                       │ GND (0V)                            │
└───────────────────────┼──────────────────────────────────────┘
                        │
                        │
        ┌───────────────┼────────────────┐
        │               │                │
        │        ┌──────────────────┐    │
        │        │  Water Sensor    │    │
        │        │  (Elegoo)        │    │
        │        │                  │    │
        │     VCC│  ●  S  GND  ●    │    │
        │        │  │   │   │       │    │
        │        │  │   │   │       │    │
        │        └──┼───┼───┼───────┘    │
        │           │   │   │            │
        │           │   │   └────────────┼── ESP32 GND
        │           │   │                │
        │           │   └────────────────┼── ESP32 GPIO 34 (ADC)
        │           │                    │
        │           └────────────────────┼── ESP32 5V or 3.3V
        │                                │
        └────────────────────────────────┘
```

## Detailed Wiring

### ESP32 WROOM-32D to Elegoo Water Sensor

| Component | ESP32 Pin | Wire Color (Suggested) | Function |
|-----------|-----------|--------|----------|
| Water Sensor VCC | 5V or 3.3V | Red | Power Supply |
| Water Sensor S (Signal) | GPIO 34 | Yellow | Analog Input |
| Water Sensor GND | GND | Black | Ground Reference |

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
- **Operating Voltage**: 3.3V (5V tolerant on most GPIO pins)
- **ADC Input Range**: 0-3.3V
- **GPIO 34**: ADC0 (Analog-to-Digital Converter channel 0)
- **Features**: WiFi, Bluetooth, 240MHz dual-core processor

### Elegoo Water Sensor
- **Operating Voltage**: 3.3V - 5V
- **Output Type**: Analog (0-3.3V or 0-5V depending on power)
- **Detection Range**: 0-100% water content
- **Response Time**: ~100ms

### Connection Notes
- GPIO 34 is an **input-only** ADC pin (no output capability)
- Maximum safe voltage on GPIO 34: 3.6V
- If using 5V sensor power, connect through resistor divider or use 3.3V directly
- Keep wires short to minimize noise on analog signal
- For stable readings, add a 100nF capacitor between sensor signal and GND near the ESP32

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

## Sensor Calibration Setup

For accurate water detection, calibrate in two states:

```
State 1: Sensor in Air
└─ Read ADC value: ~1000-2000 (depends on humidity)

State 2: Sensor in Water
└─ Read ADC value: ~2500-3500 (depends on water conductivity)

Threshold = (Air Value + Water Value) / 2
```

Monitor Serial output and adjust `SENSOR_THRESHOLD` in config.h accordingly.

## Safety Considerations

1. **Water Exposure**: Do NOT submerge ESP32 itself - only the sensor probe
2. **Electrical Safety**: Keep electrical components away from water tank contact area
3. **WiFi**: Ensure adequate antenna clearance (keep away from metal)
4. **Enclosure**: House ESP32 in waterproof container away from splash zone
5. **Ventilation**: Allow airflow in enclosure to prevent condensation

## Testing Checklist

- [ ] ESP32 USB connection recognized by computer
- [ ] Serial Monitor shows boot messages at 115200 baud
- [ ] Sensor values displayed in Serial Monitor
- [ ] WiFi connects successfully
- [ ] Sensor threshold properly calibrated
- [ ] Test email alert with manual trigger
- [ ] Verify automatic reconnection after WiFi loss
