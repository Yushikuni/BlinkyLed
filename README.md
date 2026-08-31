# 🧙‍♀️ One Button. Three Modes. One Slightly Overengineered STM32.

[C] [STM32] [HAL] [ST-LINK] [Build] [License] [Platform] [GitHub Actions] [Made with ❤️] [100% Human] [Coffee Powered] ☕💀

A small STM32 Nucleo project where **one button controls three different modes**.
Because apparently making an LED blink wasn't enough. 😄

## 🎯 What is this?

This project is a small experiment with:

* GPIO input/output
* button debouncing
* single-click, double-click and long-press detection
* time-based logic using `HAL_GetTick()`
* a simple state machine
* UART communication
* `printf()` retargeting
* non-blocking application logic

The project runs on an **STM32 Nucleo-F103RB** and uses **PuTTY** as a serial terminal for debugging and monitoring.

## 🧠 Three Modes

| Button input | Mode           | Behavior                                   |
| ------------ | -------------- | ------------------------------------------ |
| Single click | `MODE_BLINK`   | LED blinks and reports its state over UART |
| Double click | `MODE_STEALTH` | LED stays off and UART goes silent         |
| Long press   | `MODE_OFF`     | LED is turned off                          |

### 🥷 Stealth Mode

The most important feature, obviously.

In `MODE_STEALTH`, the internal blink logic continues to run, but the physical LED output and UART messages are suppressed.

The MCU is working.

It's just not telling anyone about it. 😎

## ⚙️ Hardware & Software

### Hardware

* **MCU:** STM32 Nucleo-F103RB
* User button
* User LED
* USB connection

### Software

* **IDE:** STM32CubeIDE
* **Language:** C
* **Framework:** STM32 HAL
* **Debugger:** ST-LINK
* **Serial terminal:** PuTTY

## 🏗️ Project Structure

The main application logic is split into two small tasks:

```text
main()
 ├── ButtonTask()
 │    └── Detects button input
 │         ├── Single click
 │         ├── Double click
 │         └── Long press
 │
 └── LedTask()
      └── Handles the current LED mode
           ├── MODE_OFF
           ├── MODE_BLINK
           └── MODE_STEALTH
```

The main loop stays intentionally simple:

```c
while (1)
{
    ButtonTask();
    LedTask();
}
```

Timing is handled using `HAL_GetTick()` instead of blocking the application with `HAL_Delay()`.

## 📡 UART Output

UART is used as a simple debugging and monitoring interface.

Example output:

```text
MODE: BLINK (single click)
LED ON
LED OFF
LED ON
LED OFF
MODE: STEALTH (double click)
MODE: OFF (long press)
```

This makes it possible to see what the firmware thinks is happening without relying only on the physical LED.

## 🐞 Known Challenges

During development I had to deal with:

* button debouncing
* distinguishing single and double clicks
* detecting long presses while the button is held
* matching UART settings between the STM32 and PuTTY
* keeping the main loop responsive
* understanding how generated STM32 HAL code is organized

Because embedded development apparently requires negotiating with a button before it agrees to do anything. 💀

## 🚀 How to Run

1. Clone the repository.
2. Open the project in **STM32CubeIDE**.
3. Connect the STM32 Nucleo-F103RB via USB.
4. Build the project.
5. Flash it using ST-LINK.
6. Open PuTTY and configure the serial connection to match the USART settings from the `.ioc` file.
7. Press the button and observe the different modes.

## 📚 What I Learned

This project helped me understand the difference between simply making something work and making the application **continuously process input without blocking the main loop**.
The biggest takeaway was using elapsed time with `HAL_GetTick()` instead of relying on `HAL_Delay()` for the application logic.
It also gave me a first practical taste of implementing a small **state machine** on an MCU.

## 🔮 What's Next?

Maybe I'll make an intelligent weapon like **Skippy**. 😄
For now, it's just one button and one LED.
But that's how it starts, right?
First an LED.
Then a state machine.
Then sensors.
And suddenly I have an armed embedded system talking to me.
**What could possibly go wrong?** 💀

## 📖 Related Article
Coming soon: the full write-up covering the implementation, debugging process and the decisions behind the project.
