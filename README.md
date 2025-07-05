# qCommTest

[![License](https://img.shields.io/github/license/diffstorm/qCommTest)](https://github.com/diffstorm/qCommTest/blob/main/LICENSE)
[![Language](https://img.shields.io/github/languages/top/diffstorm/qCommTest)](https://github.com/diffstorm/qCommTest)

A good looking UI for stress testing serial (RS232) and TCP communication.

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

## Integration Tests

This project includes integration tests to verify the TCP server functionality. These tests are located in the `test/` directory.

-   `test_tcp.py`: A Python script that acts as a TCP client to test the `qCommTest` TCP server.
-   `test_tcp.c`: A C program that acts as a TCP client, similar to the Python script.
-   `test_tcp.cpp`: A C++17 program that acts as a TCP client, similar to the Python script.

To run these tests, first ensure the `qCommTest` application is running and its TCP server is listening on port 6666.

### Running the Python Test

```bash
python3 test/test_tcp.py
```

### Running the C Test

```bash
gcc -o test/test_tcp test/test_tcp.c
./test/test_tcp
```

### Running the C++ Test

```bash
g++ -std=c++17 -o test/test_tcp_cpp test/test_tcp.cpp
./test/test_tcp_cpp
```

### Windows Specific Compilation for C/C++ Tests

For Windows, you might need to link against `ws2_32` for the C and C++ tests:

```bash
gcc -o test/test_tcp test/test_tcp.c -lws2_32
g++ -std=c++17 -o test/test_tcp_cpp test/test_tcp.cpp -lws2_32
```

### Build with CMake (Qt5/Qt6)

To build the project using CMake:

1.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```

2.  Configure CMake to use a specific Qt version (e.g., Qt6):
    ```bash
    cmake .. -DQT_VERSION=6
    ```
    *Note: Replace `-DQT_VERSION=6` with `-DQT_VERSION=5` to build with Qt5.*

3.  Build the project:
    ```bash
    cmake --build .
    ```

### Build with qmake (Qt5/Qt6)

To build the project using `qmake` (for Qt Creator or command line):

1.  **For Qt5:**
    ```bash
    qmake -qt=qt5 qCommTest.pro
    make
    ```

2.  **For Qt6:**
    ```bash
    qmake -qt=qt6 qCommTest.pro
    make
    ```

    *Note: You might need to specify the full path to `qmake` if it's not in your PATH, e.g., `/path/to/Qt/5.x.x/gcc_64/bin/qmake` or `/path/to/Qt/6.x.x/gcc_64/bin/qmake`.*

### Windows

1.  **Install Chocolatey:** If you don't have Chocolatey, install it by following the instructions on the [Chocolatey website](https://chocolatey.org/install).
2.  **Install CMake, Qt, and a compiler:**
    ```bash
    choco install cmake qt-creator-ide qt6-base qt6-serialport
    ```

### macOS

1.  **Install Homebrew:** If you don't have Homebrew, install it by following the instructions on the [Homebrew website](https://brew.sh/).
2.  **Install CMake and Qt:**
    ```bash
    brew install cmake qt5
    ```
    *Note: You can also use `qt6` instead of `qt5`.*

### Linux (Ubuntu)

1.  **Install dependencies:**
    ```bash
    sudo apt-get update
    sudo apt-get install build-essential cmake qtcreator qtbase5-dev qtserialport5-dev
    ```
    *Note: For Qt6, you can use `qt6-base-dev` and `qt6-serialport-dev`.*

## Author

Eray Öztürk ([@diffstorm](https://github.com/diffstorm))

## License

This project is licensed under the [GPL-3.0-only License](LICENSE).