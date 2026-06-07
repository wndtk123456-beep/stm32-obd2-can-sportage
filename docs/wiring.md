# Wiring Notes

## Signal path

```text
Sportage OBD-II
  -> CAN transceiver
  -> NUCLEO-F042K6 CAN peripheral
  -> ST-LINK Virtual COM Port
  -> PC serial monitor
```

The exact CAN transceiver module used during the test was not recorded in the
source files. Check the module datasheet before applying power.

## OBD-II and STM32 signals

| Source | Destination | Purpose |
|---|---|---|
| OBD-II pin 6 | Transceiver CANH | CAN high |
| OBD-II pin 14 | Transceiver CANL | CAN low |
| OBD-II pin 4 or 5 | Common ground | Signal reference |
| Transceiver RXD | STM32 PA11 | CAN_RX |
| Transceiver TXD | STM32 PA12 | CAN_TX |
| STM32 PA2 | ST-LINK VCP RX | USART2_TX |
| STM32 PA15 | ST-LINK VCP TX | USART2_RX |

## Interface settings

- CAN bitrate: 500 kbit/s
- CAN timing: prescaler 6, BS1 13 TQ, BS2 2 TQ
- USART2: 9,600 bit/s
- MCU clock: 48 MHz

## Safety

- A CAN transceiver is required between the vehicle bus and STM32 CAN pins.
- Do not connect OBD-II pin 16 vehicle battery voltage directly to an STM32
  power pin.
- Use a regulated, protected supply and a shared signal ground.
- Test while the vehicle is stationary in a controlled environment.
- Verify pin assignments and voltage levels against the actual board and
  transceiver datasheets before connecting hardware.
