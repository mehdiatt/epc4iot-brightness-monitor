# Brightness monitor (Zephyr, NUCLEO-WL55JC)

Hands-on lab of *Embedded Platforms and Communications for IoT* (MUIoT, UPM).

The board measures the ambient light with a light sensor and shows the level on
an RGB LED. Button B2 changes the operating mode. Everything runs on Zephyr RTOS
on a NUCLEO-WL55JC.

## What it does

There are three modes:

- **NORMAL**: every 2 seconds the ADC reads the sensor and the LED shows the
  level: red below 33 %, yellow from 33 % to 66 %, green above 66 %. The ADC
  value is also printed on the serial terminal.
- **BLUE**: the LED stays blue and the ADC is not read.
- **OFF**: the LED is off, the ADC is not read and the terminal prints
  `System OFF`.

Button B2:

- a **short press** switches between NORMAL and BLUE, and goes back to NORMAL
  from OFF;
- a **long press** (1 second or more) goes to OFF from NORMAL or BLUE. It is
  detected as soon as the second is over, we do not wait for the button to be
  released, and releasing it afterwards does not count as a short press.

## Hardware

- **RGB LED, common anode** (the common leg goes to 3V3, so a pin at 0 lights
  the colour), with a series resistor on each colour. Red on D3 (PB3), green on
  D4 (PB5), blue on D5 (PB8).
- **Light sensor module** (an LDR with a resistor on the same board). Signal pin
  on A0 (PB1, ADC channel 5), the middle pin on 3V3 and the last pin on GND.
- **Button B2**, the user button of the board (PA1, devicetree alias `sw1`).

The LED type is only written in `boards/nucleo_wl55jc.overlay`
(`GPIO_ACTIVE_LOW`). The C code does not care about it. With a common cathode LED
we would only change this flag.

The board buttons already have a hardware debounce circuit, so we do not debounce
in software.

### Brightness in percent

The ADC gives a value from 0 to 4095. With our sensor module the value only
moves between about **2650** (a hand over the sensor) and **4072** (phone flash
against it). With the full 0 to 4095 range the LED was almost never red, so we
compute the percentage between two limits that we measured:

```
percent = (raw - 2700) * 100 / (4000 - 2700)      limited to 0..100
```

The two limits are `LDR_RAW_DARK` and `LDR_RAW_BRIGHT` in `src/ldr.h`. They
depend on the sensor and on the light in the room, so they may need a new
adjustment: the `raw` value is printed on the terminal.

## How the code is organised

There are three threads, each in its own file. `main` only prints a banner
(the threads start by themselves with `K_THREAD_DEFINE`).

| Thread | File | Priority | What it does |
|---|---|---|---|
| button | `src/button.c` | 3 | measures how long B2 is pressed |
| controller | `src/controller.c` | 4 | state machine, drives the LED, prints the mode |
| measure | `src/measure.c` | 5 | reads the ADC every 2 s in NORMAL mode |

In Zephyr a smaller number is a higher priority.

How they talk to each other:

1. B2 triggers an **interrupt** on both edges (press and release). The interrupt
   handler only reads the pin and puts the value in a **message queue**
   (`edge_q`). It does nothing else, so it is very short.
2. The **button thread** reads `edge_q` and works out if it was a short or a long
   press. It sends the result to the controller through a second queue,
   `event_q`.
3. The **measure thread** sends each new ADC reading to the same queue.
4. The **controller thread** is the only one that reads `event_q`. It changes the
   mode, sets the LED and prints on the terminal.
5. The controller tells the measure thread if it may measure with a `k_event`
   (`meas_ev`). In BLUE and OFF mode the measure thread just waits on it.

### Why we did it this way

- **Three threads** because there are three different jobs with different
  timing: waiting for the user, measuring every 2 seconds, and deciding what to
  do. The state machine is alone in the controller, which keeps the other two
  threads simple.
- **A message queue between the ISR and the button thread** because the order of
  the press and release edges matters to measure the 1 second, and `k_msgq_put`
  with `K_NO_WAIT` can be called from an interrupt.
- **A message queue to the controller** because two threads send it messages and
  the messages carry data (the type of event and the ADC value).
- **A `k_event` for start/stop** because it is only an on/off signal: "measuring
  is allowed" or not.
- **No mutex**: each resource has a single user. Only the controller uses the
  LED, only the measure thread uses the ADC, only the button thread uses the
  button. Nothing is shared between two threads, so there is nothing to lock.
- **Priorities**: the button thread is the highest so the 1 second limit is
  noticed on time. The measure thread is the lowest, one reading every 2 seconds
  is not urgent.
- If a measurement arrives after we already left NORMAL mode, the controller
  ignores it.

### The long press

When the button thread sees a press, it waits for the release on `edge_q` for at
most 1 second. If the release comes before, it is a short press. If the time
runs out, it sends the long press at once, then waits for the release and throws
it away, so that a long press never gives a short press too.

## Serial terminal

115200 baud, 8N1.

```
[NORMAL] ADC raw = 3602   brightness =  69 %
[NORMAL] ADC raw = 3645   brightness =  72 %
[NORMAL] ADC raw = 3533   brightness =  64 %
[MODE] BLUE
[MODE] NORMAL
[NORMAL] ADC raw = 3540   brightness =  64 %
[NORMAL] ADC raw = 3577   brightness =  67 %
[NORMAL] ADC raw = 3934   brightness =  94 %
[NORMAL] ADC raw = 3939   brightness =  95 %
[NORMAL] ADC raw = 3939   brightness =  95 %
[NORMAL] ADC raw = 3603   brightness =  69 %
[NORMAL] ADC raw = 3506   brightness =  62 %
[NORMAL] ADC raw = 2869   brightness =  13 %
[NORMAL] ADC raw = 2820   brightness =   9 %
[NORMAL] ADC raw = 2748   brightness =   3 %
[NORMAL] ADC raw = 3661   brightness =  73 %
[NORMAL] ADC raw = 3666   brightness =  74 %
System OFF
[MODE] NORMAL
[NORMAL] ADC raw = 3667   brightness =  74 %
[NORMAL] ADC raw = 3694   brightness =  76 %
```


When the system goes back to NORMAL the LED stays off until the first
measurement, which can take up to 2 seconds.

## Files

```
CMakeLists.txt                 build file
boards/nucleo_wl55jc.overlay   LED pins and ADC channel
confs/prj_nucleo_wl55jc.conf   Zephyr options (GPIO, ADC, events, console)
src/main.c                     banner only
src/events.h                   event types, shared queue and event
src/button.c                   button thread
src/controller.c               controller thread
src/measure.c                  measure thread
src/ldr.c, src/ldr.h           light sensor (ADC, percentage)
src/led_rgb.c, src/led_rgb.h   RGB LED (red, yellow, green, blue, off)
```

## Build and flash

From the Zephyr workspace with the Python virtual environment active:

```
west build -p auto -b nucleo_wl55jc <path-to-repo> -d <path-to-repo>/build \
    -DDTC_OVERLAY_FILE=./boards/nucleo_wl55jc.overlay \
    -DCONF_FILE=./confs/prj_nucleo_wl55jc.conf
```

Flash with STM32CubeProgrammer through the ST-LINK USB port:

```
STM32_Programmer_CLI --connect port=swd mode=UR reset=HWrst \
    --download <path-to-repo>/build/zephyr/zephyr.hex --start
```

For the serial terminal, open the ST-LINK virtual COM port (the number depends on
the computer) at 115200, 8N1, for example
`plink -serial COM5 -sercfg 115200,8,n,1,N`. Press the reset button (B4) to see
the messages from the start.

## Authors

- Mehdi Attafi (GitHub: `mehdiatt`)
- Nouha Madiouni (GitHub: `nouhamadiouni`)