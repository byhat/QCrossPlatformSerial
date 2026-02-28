# QCrossPlatformSerial

[![License: LGPL-2.1](https://img.shields.io/badge/License-LGPL--2.1-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Android-lightgrey.svg)](https://www.qt.io)
[![Qt](https://img.shields.io/badge/Qt-5%20%7C%206-41CD52.svg)](https://www.qt.io)
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)](https://en.cppreference.com/w/cpp/17)

A cross-platform serial port communication library built with Qt, providing a unified API for desktop (Windows, Linux, macOS) and Android platforms.

## Overview

**QCrossPlatformSerial** is a C++ library that abstracts serial port communication across different platforms. It provides a consistent, Qt-style API that works seamlessly on desktop platforms using Qt's SerialPort module and on Android using JNI to interface with USB serial devices.

### Key Features

- **Unified API** - Write once, run on all supported platforms
- **Qt-style signals/slots** - Asynchronous, event-driven communication
- **Comprehensive configuration** - Full control over baud rates, data bits, stop bits, parity, and flow control
- **USB-to-serial adapter support** - Extensive support for common USB serial chipsets on Android
- **Device enumeration** - Query available serial ports with detailed device information
- **Pimpl pattern** - Clean separation of public API and platform-specific implementation

### Why Use QCrossPlatformSerial?

- **Simplified cross-platform development** - No need to write platform-specific code for serial communication
- **Android USB serial support** - Native support for USB serial devices without requiring root access
- **Familiar Qt API** - If you know QSerialPort, you already know QCrossPlatformSerial
- **Production-ready** - Robust error handling and comprehensive feature set

## Supported Platforms

| Platform | Status | Implementation |
|----------|--------|----------------|
| Windows | ✅ Supported | Qt SerialPort |
| Linux | ✅ Supported | Qt SerialPort |
| macOS | ✅ Supported | Qt SerialPort |
| Android | ✅ Supported | JNI + USB Serial for Android |

**Note:** The library automatically selects the appropriate implementation based on the target platform at compile time.

## Features

### Core Functionality

- **Unified API across platforms** - Same code works on desktop and Android
- **Qt-style signals/slots interface** - Event-driven asynchronous I/O
- **Configurable serial parameters:**
  - Baud rates (standard and custom)
  - Data bits (5, 6, 7, 8)
  - Stop bits (1, 1.5, 2)
  - Parity (None, Even, Odd, Space, Mark)
  - Flow control (None, Hardware, Software)
- **Buffer management** - Read/write buffer control with `bytesAvailable()` and `bytesToWrite()`
- **Error handling** - Comprehensive error reporting via signals and error codes

### Device Management

- **Device enumeration** - List all available serial ports
- **Device information** - Retrieve detailed information about each port:
  - Port name
  - Description
  - Manufacturer
  - Serial number
  - Vendor ID (VID)
  - Product ID (PID)
  - System location
- **Standard baud rate query** - Get list of supported baud rates per device

### Android-Specific Features

- **USB-to-serial adapter support** for chipsets including:
  - FTDI (FT232R, FT2232, FT4232, etc.)
  - Silicon Labs CP21xx
  - CH34x
  - Prolific PL2303
  - And many more
- **USB permission handling** - Automatic USB device permission requests
- **Hot-plug detection** - Detect USB device connection/disconnection

## Requirements

### Desktop (Windows, Linux, macOS)

- **Qt5** or **Qt6** (Core, SerialPort, Gui modules)
- **C++17** compatible compiler
- **CMake** 3.16 or later

### Android

- **Qt5** or **Qt6** (Core, SerialPort, Gui modules)
- **C++17** compatible compiler
- **CMake** 3.16 or later
- **Android NDK** r21 or later
- **USB Serial for Android** library (included)

## Installation / Building

### Building the Library

#### Using CMake

```bash
# Clone the repository
git clone https://github.com/yourusername/QCrossPlatformSerial.git
cd QCrossPlatformSerial

# Create build directory
mkdir build && cd build

# Configure (adjust Qt path as needed)
cmake ..

# Build
cmake --build .

# Install (optional)
cmake --install .
```

#### Platform-Specific Notes

**Desktop Platforms:**

```bash
# Standard CMake configuration
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
```

**Android:**

```bash
# Configure for Android
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/Qt/6.x.x/android/src/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/android \
      ..
```

### Integration into Existing Projects

#### CMake Integration

Add the following to your project's `CMakeLists.txt`:

```cmake
# Find QCrossPlatformSerial
find_package(QCrossPlatformSerial REQUIRED)

# Link against the library
target_link_libraries(your_target PRIVATE
    QCrossPlatformSerial
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Gui
)

# Include headers
target_include_directories(your_target PRIVATE
    ${QCrossPlatformSerial_INCLUDE_DIRS}
)
```

#### Android Integration

For Android projects, ensure your `AndroidManifest.xml` includes the necessary USB device filters:

```xml
<activity android:name="...">
    <intent-filter>
        <action android:name="android.hardware.usb.action.USB_DEVICE_ATTACHED" />
    </intent-filter>
    
    <meta-data android:name="android.hardware.usb.action.USB_DEVICE_ATTACHED"
               android:resource="@xml/device_filter" />
</activity>
```

Create `res/xml/device_filter.xml`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <!-- Add your USB device VID/PID combinations here -->
    <usb-device vendor-id="0x0403" product-id="0x6001" />
</resources>
```

## Usage Examples
Comprehensive example https://github.com/byhat/QCrossPlatformSerial-Example
### Basic Example: Opening a Port and Reading Data

```cpp
#include <QCoreApplication>
#include <QCrossPlatformSerialPort>
#include <QCrossPlatformSerialPortInfo>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // List available ports
    auto ports = QCrossPlatformSerialPortInfo::availablePorts();
    if (ports.isEmpty()) {
        qWarning() << "No serial ports available";
        return -1;
    }

    // Use the first available port
    QCrossPlatformSerialPort serial(ports.first());

    // Configure the port
    serial.setBaudRate(9600);
    serial.setDataBits(QCrossPlatformDataBits::Data8);
    serial.setParity(QCrossPlatformParity::NoParity);
    serial.setStopBits(QCrossPlatformStopBits::OneStop);
    serial.setFlowControl(QCrossPlatformFlowControl::NoFlowControl);

    // Connect signals
    QObject::connect(&serial, &QCrossPlatformSerialPort::readyRead, [&]() {
        QByteArray data = serial.readAll();
        qDebug() << "Received:" << data.toHex();
    });

    // Open the port
    if (!serial.open(QIODevice::ReadWrite)) {
        qWarning() << "Failed to open port:" << serial.error();
        return -1;
    }

    qDebug() << "Port opened successfully";
    return app.exec();
}
```

### Advanced Example: Configuration and Error Handling

```cpp
#include <QCrossPlatformSerialPort>
#include <QCrossPlatformSerialPortInfo>
#include <QDebug>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr) : QObject(parent)
    {
        m_serial = new QCrossPlatformSerialPort(this);

        // Connect signals
        connect(m_serial, &QCrossPlatformSerialPort::readyRead,
                this, &SerialManager::handleReadyRead);
        connect(m_serial, &QCrossPlatformSerialPort::bytesWritten,
                this, &SerialManager::handleBytesWritten);
        connect(m_serial, &QCrossPlatformSerialPort::errorOccurred,
                this, &SerialManager::handleError);
    }

    bool connectToDevice(const QString &portName)
    {
        m_serial->setPortName(portName);

        // Configure serial parameters
        if (!m_serial->setBaudRate(115200)) {
            qWarning() << "Failed to set baud rate";
            return false;
        }

        if (!m_serial->setDataBits(QCrossPlatformDataBits::Data8)) {
            qWarning() << "Failed to set data bits";
            return false;
        }

        if (!m_serial->setParity(QCrossPlatformParity::NoParity)) {
            qWarning() << "Failed to set parity";
            return false;
        }

        if (!m_serial->setStopBits(QCrossPlatformStopBits::OneStop)) {
            qWarning() << "Failed to set stop bits";
            return false;
        }

        if (!m_serial->setFlowControl(QCrossPlatformFlowControl::NoFlowControl)) {
            qWarning() << "Failed to set flow control";
            return false;
        }

        // Open the port
        if (!m_serial->open(QIODevice::ReadWrite)) {
            qWarning() << "Failed to open port:" << m_serial->error();
            return false;
        }

        qDebug() << "Connected to" << portName;
        return true;
    }

    void sendData(const QByteArray &data)
    {
        if (!m_serial->isOpen()) {
            qWarning() << "Port is not open";
            return;
        }

        qint64 bytesWritten = m_serial->write(data);
        if (bytesWritten == -1) {
            qWarning() << "Write error:" << m_serial->error();
        } else {
            qDebug() << "Writing" << bytesWritten << "bytes";
        }
    }

private slots:
    void handleReadyRead()
    {
        QByteArray data = m_serial->readAll();
        qDebug() << "Received" << data.size() << "bytes:" << data.toHex();
        // Process data...
    }

    void handleBytesWritten(qint64 bytes)
    {
        qDebug() << bytes << "bytes written successfully";
    }

    void handleError(QCrossPlatformSerialPortError error)
    {
        QString errorString;
        switch (error) {
        case QCrossPlatformSerialPortError::NoError:
            errorString = "No error";
            break;
        case QCrossPlatformSerialPortError::DeviceNotFoundError:
            errorString = "Device not found";
            break;
        case QCrossPlatformSerialPortError::PermissionError:
            errorString = "Permission denied";
            break;
        case QCrossPlatformSerialPortError::OpenError:
            errorString = "Failed to open device";
            break;
        case QCrossPlatformSerialPortError::WriteError:
            errorString = "Write error";
            break;
        case QCrossPlatformSerialPortError::ReadError:
            errorString = "Read error";
            break;
        default:
            errorString = "Unknown error";
            break;
        }
        qWarning() << "Serial error:" << errorString;
    }

private:
    QCrossPlatformSerialPort *m_serial;
};
```

### Device Enumeration Example

```cpp
#include <QCrossPlatformSerialPortInfo>
#include <QDebug>

void listSerialPorts()
{
    auto ports = QCrossPlatformSerialPortInfo::availablePorts();

    qDebug() << "Found" << ports.size() << "serial port(s):";

    for (const auto &port : ports) {
        qDebug() << "----------------------------------------";
        qDebug() << "Port Name:" << port.portName();
        qDebug() << "Description:" << port.description();
        qDebug() << "Manufacturer:" << port.manufacturer();
        qDebug() << "Serial Number:" << port.serialNumber();
        qDebug() << "Vendor ID:" << QString::number(port.vendorIdentifier(), 16).toUpper();
        qDebug() << "Product ID:" << QString::number(port.productIdentifier(), 16).toUpper();
        qDebug() << "System Location:" << port.systemLocation();

        auto baudRates = port.standardBaudRates();
        qDebug() << "Standard Baud Rates:" << baudRates;
    }
}
```

### Finding a Specific Device by VID/PID

```cpp
#include <QCrossPlatformSerialPortInfo>
#include <QDebug>

QCrossPlatformSerialPortInfo findDevice(quint16 vid, quint16 pid)
{
    auto ports = QCrossPlatformSerialPortInfo::availablePorts();

    for (const auto &port : ports) {
        if (port.vendorIdentifier() == vid && 
            port.productIdentifier() == pid) {
            qDebug() << "Found device:" << port.portName();
            return port;
        }
    }

    qWarning() << "Device not found (VID:" << QString::number(vid, 16) 
               << "PID:" << QString::number(pid, 16) << ")";
    return QCrossPlatformSerialPortInfo(); // Returns null info
}

// Usage example
void connectToFTDI()
{
    // FTDI FT232R default VID/PID
    auto portInfo = findDevice(0x0403, 0x6001);

    if (!portInfo.isNull()) {
        QCrossPlatformSerialPort serial(portInfo);
        // ... configure and open
    }
}
```

## API Reference

### QCrossPlatformSerialPort

The main class for serial port communication.

#### Public Methods

| Method | Description |
|--------|-------------|
| `QCrossPlatformSerialPort(QObject *parent = nullptr)` | Constructs a serial port object |
| `QCrossPlatformSerialPort(const QString &portName, QObject *parent = nullptr)` | Constructs a serial port object with a specific port name |
| `QCrossPlatformSerialPort(const QCrossPlatformSerialPortInfo &portInfo, QObject *parent = nullptr)` | Constructs a serial port object from port info |
| `void setPortName(const QString &name)` | Sets the port name |
| `QString portName() const` | Returns the port name |
| `bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite)` | Opens the serial port |
| `void close()` | Closes the serial port |
| `bool isOpen() const` | Returns true if the port is open |
| `qint64 write(const QByteArray &data)` | Writes data to the port |
| `QByteArray readAll()` | Reads all available data |
| `bool setBaudRate(qint32 baudRate, QCrossPlatformDirections directions = AllDirections)` | Sets the baud rate |
| `qint32 baudRate(QCrossPlatformDirections directions = AllDirections) const` | Returns the current baud rate |
| `bool setDataBits(QCrossPlatformDataBits dataBits)` | Sets the data bits |
| `QCrossPlatformDataBits dataBits() const` | Returns the current data bits |
| `bool setParity(QCrossPlatformParity parity)` | Sets the parity |
| `QCrossPlatformParity parity() const` | Returns the current parity |
| `bool setStopBits(QCrossPlatformStopBits stopBits)` | Sets the stop bits |
| `QCrossPlatformStopBits stopBits() const` | Returns the current stop bits |
| `bool setFlowControl(QCrossPlatformFlowControl flowControl)` | Sets the flow control |
| `QCrossPlatformFlowControl flowControl() const` | Returns the current flow control |
| `bool clear(QCrossPlatformDirections directions = AllDirections)` | Clears the specified buffers |
| `bool flush()` | Flushes the serial port buffers |
| `qint64 bytesAvailable() const` | Returns the number of bytes available for reading |
| `qint64 bytesToWrite() const` | Returns the number of bytes waiting to be written |
| `QCrossPlatformSerialPortError error() const` | Returns the last error |
| `void clearError()` | Clears the error state |

#### Signals

| Signal | Description |
|--------|-------------|
| `void readyRead()` | Emitted when new data is available for reading |
| `void bytesWritten(qint64 bytes)` | Emitted when data has been written to the port |
| `void errorOccurred(QCrossPlatformSerialPortError error)` | Emitted when an error occurs |

### QCrossPlatformSerialPortInfo

Provides information about available serial ports.

#### Public Methods

| Method | Description |
|--------|-------------|
| `QCrossPlatformSerialPortInfo()` | Constructs an empty port info object |
| `QString portName() const` | Returns the port name |
| `QString description() const` | Returns the description string |
| `QString manufacturer() const` | Returns the manufacturer string |
| `QString serialNumber() const` | Returns the serial number string |
| `quint16 productIdentifier() const` | Returns the product ID |
| `quint16 vendorIdentifier() const` | Returns the vendor ID |
| `QString systemLocation() const` | Returns the system location |
| `QList<qint32> standardBaudRates() const` | Returns standard baud rates |
| `static QList<QCrossPlatformSerialPortInfo> availablePorts()` | Returns all available ports |
| `bool isNull() const` | Returns true if the port info is null |
| `void swap(QCrossPlatformSerialPortInfo &other)` | Swaps with another port info |

### Enums

#### QCrossPlatformDirections

| Value | Description |
|-------|-------------|
| `Input` | Input direction |
| `Output` | Output direction |
| `AllDirections` | Both directions |

#### QCrossPlatformDataBits

| Value | Description |
|-------|-------------|
| `Data5` | 5 data bits |
| `Data6` | 6 data bits |
| `Data7` | 7 data bits |
| `Data8` | 8 data bits |

#### QCrossPlatformParity

| Value | Description |
|-------|-------------|
| `NoParity` | No parity |
| `EvenParity` | Even parity |
| `OddParity` | Odd parity |
| `SpaceParity` | Space parity |
| `MarkParity` | Mark parity |

#### QCrossPlatformStopBits

| Value | Description |
|-------|-------------|
| `OneStop` | 1 stop bit |
| `OneAndHalfStop` | 1.5 stop bits |
| `TwoStop` | 2 stop bits |

#### QCrossPlatformFlowControl

| Value | Description |
|-------|-------------|
| `NoFlowControl` | No flow control |
| `HardwareControl` | Hardware (RTS/CTS) flow control |
| `SoftwareControl` | Software (XON/XOFF) flow control |

#### QCrossPlatformSerialPortError

| Value | Description |
|-------|-------------|
| `NoError` | No error occurred |
| `DeviceNotFoundError` | Device not found |
| `PermissionError` | Permission denied |
| `OpenError` | Failed to open device |
| `ParityError` | Parity error detected |
| `FramingError` | Framing error detected |
| `BreakConditionError` | Break condition detected |
| `WriteError` | Write operation failed |
| `ReadError` | Read operation failed |
| `ResourceError` | Resource error |
| `UnsupportedOperationError` | Unsupported operation |
| `UnknownError` | Unknown error |
| `TimeoutError` | Timeout occurred |
| `NotOpenError` | Port not open |

## Platform-Specific Notes

### Desktop Platforms (Windows, Linux, macOS)

- **Implementation:** Uses Qt's `QSerialPort` class
- **Device naming:**
  - Windows: `COM1`, `COM2`, etc.
  - Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`, etc.
  - macOS: `/dev/tty.usbserial-*`, `/dev/cu.usbserial-*`, etc.
- **Permissions:** May require administrator/sudo privileges on some systems
- **Hot-plug:** Device detection may require polling or platform-specific APIs

### Android Platform

- **Implementation:** Uses JNI to interface with USB Serial for Android library
- **Device naming:** Uses USB device path or custom identifier
- **USB Permissions:**
  - Android requires explicit user permission to access USB devices
  - The library handles permission requests automatically
  - First connection to a device will prompt the user for permission
- **Device Detection:**
  - Devices are detected via USB device attachment broadcasts
  - Only USB-to-serial adapters are supported (native UART not available)
- **Supported Chipsets:**
  - FTDI (FT232, FT2232, FT4232, etc.)
  - Silicon Labs CP210x, CP21xx
  - CH34x
  - Prolific PL2303
  - CDC/ACM devices
  - And many others

### Differences Between Implementations

| Feature | Desktop | Android |
|---------|---------|---------|
| Native UART support | ✅ Yes | ❌ No (USB only) |
| Hot-plug detection | ⚠️ Limited | ✅ Full support |
| Permission handling | System-level | User permission dialog |
| Device enumeration | System ports | USB devices only |
| Baud rate flexibility | High | Depends on chipset |

## Contributing

Contributions are welcome! Please follow these guidelines:

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style Guidelines

- Follow Qt coding conventions
- Use meaningful variable and function names
- Add Doxygen-style comments for public APIs
- Test your changes on multiple platforms when possible
- Ensure all existing tests pass

### Reporting Issues

When reporting issues, please include:
- Platform and OS version
- Qt version
- Device information (VID/PID for USB devices)
- Steps to reproduce
- Expected vs actual behavior
- Relevant code snippets

## License

This project is licensed under the GNU Lesser General Public License v2.1 (LGPL-2.1). See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Qt Framework** - The excellent cross-platform application framework
- **USB Serial for Android** - The Android USB serial communication library
- **Qt SerialPort Module** - The desktop serial port implementation

---

**Note:** This library is provided as-is, without warranty of any kind. Use at your own risk.
