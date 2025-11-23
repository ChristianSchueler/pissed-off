# Pissed Off ESP32-S3 Code

controls:

- load cell
- peristaltic pump
- coin acceptor

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```

## Ideas and Todo

- nive to have: count all coins across the session and store overall coint coin, preferably by day + have a way to output the amount
  - could use NVS to store an int 32 per day, marked by key name, e.g. key = "coins_2025-11-23", value = 4000 meaning EUR 40,-
  - would have to use NVS iterator to get all coins OR simply try to get keys for certain dates

## Loop

- count coins, measure load
- INSERT_COIN: when coins reach price AND load = empty cup:
  - switch on peristaltic pump
  - state -> DISPENSING
- DISPENSING: when load = full cup OR timeout:
    - switch off peristaltic pump
    - state -> TAKE_CUP
- TAKE_CUP: when load = empty:
    - reset coins
    - (tare?)
    - state -> INSERT_COIN

# Connect to ESP32

powershell:
```
usbipd list
usbipd attach --wsl --auto-attach --busid 10-2
```
