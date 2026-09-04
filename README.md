# device-lab

Hardware and display experiments for small devices.

## Scope

- e-paper devices
- AMOLED devices
- ESP32-based boards
- device-specific firmware experiments
- hardware setup and troubleshooting notes

## Device naming

Each physical device uses a short alias under `devices/`.

Current devices:

| Alias | Device type | Hardware |
| --- | --- | --- |
| `snow` | e-paper | Waveshare ESP32-S3-ePaper-1.54G |

## Repository layout

```text
devices/
  {alias}/
    README.md
    arduino/
    docs/

experiments/
  epaper/
  amoled/
  wifi/
  audio/

shared/
  arduino/
  docs/

references/
```

## Secrets policy

Do not commit Wi-Fi credentials, tokens, local IP addresses, MAC addresses, or other environment-specific identifiers.

Use `secrets.example.h` in Git and keep local `secrets.h` untracked.
