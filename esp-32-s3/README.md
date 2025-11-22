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