# Endless Car Escape

A real-time embedded systems project featuring an interactive game implemented on an Arduino Uno microcontroller using concurrent state machine task scheduling.

## 🎮 Quick Overview

This is a game where you control a car sprite on a 16x2 LCD display to dodge incoming obstacles using an IR remote. The project demonstrates advanced embedded systems concepts including real-time multitasking, interrupt-driven timer systems, and sensor integration.

**[📄 Full Project Report](./Project%20Report%20Read%20Me.pdf)** — See the detailed technical documentation and design analysis.

## 🎯 Gameplay

- **Control:** Use IR remote to jump over obstacles
- **Boost:** Adjust potentiometer for extra points
- **Score:** Points increase as you survive longer
- **Game Over:** One collision ends the game

## 🔧 Hardware

- Arduino Uno
- 16x2 LCD Display
- IR Receiver & Remote
- Passive Buzzer (audio)
- Active Buzzer (horn)
- Red & Green LEDs
- 10kΩ Potentiometer

## 🏗️ Technical Highlights

- **6 concurrent state machine tasks** running on GCD-based scheduler (10ms intervals)
- **Interrupt-driven timer system** for precise task scheduling
- **Custom LCD sprite system** for real-time game rendering
- **IR protocol decoding** for robust remote control
- **ADC sensor integration** for analog input processing

## 📂 Project Structure

```
├── src/
│   └── rvill101_Finalproject.cpp    # Main source code
├── include/                          # Header files
├── lib/                              # PlatformIO libraries
├── platformio.ini                    # Build configuration
├── Project Report Read Me.pdf        # Detailed technical report
└── README.md                         # This file
```

## 🚀 Build & Run

```bash
platformio run --target upload
```

Then use your IR remote to power on and play!

## 👨‍💻 Credits

- **Author:** Russell Villanueva (rvill101@ucr.edu)
- **Course:** UCR - Introduction to Embedded Systems
- **Demo:** [Watch on YouTube](https://youtu.be/qUb6zL_hB5A)

---

*I acknowledge all content contained herein, excluding template or example code, is my own original work.*
