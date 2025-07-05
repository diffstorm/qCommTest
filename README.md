# qCommTest

[![License](https://img.shields.io/github/license/diffstorm/qCommTest)](https://github.com/diffstorm/qCommTest/blob/main/LICENSE)
[![Language](https://img.shields.io/github/languages/top/diffstorm/qCommTest)](https://github.com/diffstorm/qCommTest)

A simple tool for testing serial (RS232) and TCP communication.

## Features

-   **Serial Port Communication:** Test communication with devices connected to serial ports.
-   **TCP Server:** Create a TCP server to test network communication.
-   **Cross-Platform:** Build and run on Windows, macOS, and Linux.
-   **Qt5 and Qt6 Support:** Compatible with both Qt5 and Qt6 versions.

## Usage

![Usage](usage.gif)

1.  **Configure and start the communication port:**
    -   For serial communication, select the port, baud rate, and other settings.
    -   For TCP communication, set the port and start the server.
2.  **Run the test code on the device:** The device should send and receive data according to the test protocol.
3.  **Observe the log section:** The log section will show the test results and any errors that occur.

## Installation

### Prerequisites

-   **CMake:** A cross-platform build system generator.
-   **Qt:** A cross-platform application framework. You can use either Qt5 or Qt6.
-   **C++ Compiler:** A C++ compiler that is compatible with the Qt version you are using.

### Windows

1.  **Install Chocolatey:** If you don't have Chocolatey, install it by following the instructions on the [Chocolatey website](https://chocolatey.org/install).
2.  **Install CMake, Qt, and a compiler:**
    ```bash
    choco install cmake qt-creator-ide qt6-base qt6-serialport
    ```
3.  **Build the project:**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

### macOS

1.  **Install Homebrew:** If you don't have Homebrew, install it by following the instructions on the [Homebrew website](https://brew.sh/).
2.  **Install CMake and Qt:**
    ```bash
    brew install cmake qt5
    ```
    *Note: You can also use `qt6` instead of `qt5`.*
3.  **Build the project:**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

### Linux (Ubuntu)

1.  **Install dependencies:**
    ```bash
    sudo apt-get update
    sudo apt-get install build-essential cmake qtcreator qtbase5-dev qtserialport5-dev
    ```
    *Note: For Qt6, you can use `qt6-base-dev` and `qt6-serialport-dev`.*
2.  **Build the project:**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

## Author

Eray Öztürk ([@diffstorm](https://github.com/diffstorm))

## License

This project is licensed under the [GPL-3.0-only License](LICENSE).