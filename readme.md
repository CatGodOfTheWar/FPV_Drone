# DSHOT Motor Control System

## Overview

The DSHOT Motor Control System is a comprehensive application designed to manage motor control using the DSHOT protocol. The system provides an intuitive interface for initializing, controlling, and finalizing motor operations, ensuring precise motor management.

## Features

- **Motor Initialization**: Allows users to initialize motor pins and set up the GPIO interface.
- **Motor Control**: Enables users to control motor speed and direction in real-time.
- **3D Mode**: Supports bidirectional rotation and reverse rotation logic.
- **Calibration**: Provides functionality to calibrate motors for optimal performance.
- **Multi-threading**: Utilizes multi-threading to handle motor control and data collection concurrently.

## Project Structure

```plaintext
.
├── .vscode/
│   ├── settings.json
├── compile.sh
├── DSHOT.c
├── DSHOT.s
├── main.cpp
├── readme_model.md
├── readme.md
```

## Installation

1. **Clone the repository**:
    ```sh
    git clone https://github.com/yourusername/dshot-motor-control-system.git
    cd dshot-motor-control-system
    ```

2. **Install dependencies**:
    ```sh
    sudo apt-get install build-essential pigpio libpthread-stubs0-dev
    ```

## Usage

1. **Compile the project**:
    ```sh
    ./compile.sh
    ```

2. **Run the application**:
    ```sh
    ./main
    ```

3. **Motor Initialization**:
    - Initialize motor pins and set up the GPIO interface.

4. **Motor Control**:
    - Control motor speed and direction in real-time using the provided functions.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any improvements or bug fixes.

## Acknowledgements

- [pigpio](http://abyz.me.uk/rpi/pigpio/)
- [pthread](https://man7.org/linux/man-pages/man7/pthreads.7.html)
- [Marian Vittek](https://github.com/Marian-Vittek)

## Contact
For any inquiries or support, please contact [chirieacandrei@proton.me](mailto:chirieacandrei@proton.me).
