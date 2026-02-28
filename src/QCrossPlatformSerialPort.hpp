#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QIODevice>
#include <qglobal.h>

// Forward declarations for Pimpl pattern
class QCrossPlatformSerialPortPrivate;

// Forward declaration for port info
class QCrossPlatformSerialPortInfo;

#ifndef Q_OS_ANDROID
// Forward declaration for desktop
class QSerialPort;
#endif

/**
 * @brief Enum for serial port directions.
 */
enum class QCrossPlatformDirections {
    AllDirections = 0x03,
    Input = 0x01,
    Output = 0x02
};

/**
 * @brief Enum for serial port data bits.
 */
enum class QCrossPlatformDataBits {
    Data5 = 5,
    Data6 = 6,
    Data7 = 7,
    Data8 = 8
};

/**
 * @brief Enum for serial port parity.
 */
enum class QCrossPlatformParity {
    NoParity = 0,
    EvenParity = 2,
    OddParity = 3,
    SpaceParity = 4,
    MarkParity = 5
};

/**
 * @brief Enum for serial port stop bits.
 */
enum class QCrossPlatformStopBits {
    OneStop = 1,
    OneAndHalfStop = 3,
    TwoStop = 2
};

/**
 * @brief Enum for serial port flow control.
 */
enum class QCrossPlatformFlowControl {
    NoFlowControl = 0,
    HardwareControl = 1,
    SoftwareControl = 2
};

/**
 * @brief Enum for serial port errors.
 */
enum class QCrossPlatformSerialPortError {
    NoError = 0,
    DeviceNotFoundError = 1,
    PermissionError = 2,
    OpenError = 3,
    ParityError = 4,
    FramingError = 5,
    BreakConditionError = 6,
    WriteError = 7,
    ReadError = 8,
    ResourceError = 9,
    UnsupportedOperationError = 10,
    UnknownError = 11,
    TimeoutError = 12,
    NotOpenError = 13
};

/**
 * @brief The QCrossPlatformSerialPort class provides a cross-platform serial port interface.
 *
 * This class provides a cross-platform interface similar to QSerialPort.
 * Platform-specific implementation details are hidden using the Pimpl pattern.
 * On Android, it uses JNI to call Java UsbSerialManager methods directly.
 * On Desktop, it wraps QSerialPort.
 *
 * This class contains NO QML-related code - no Q_PROPERTY, no Q_INVOKABLE.
 * It provides a clean C++ API for serial port operations.
 */
class QCrossPlatformSerialPort : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a QCrossPlatformSerialPort with the given parent.
     * @param parent The parent QObject
     */
    explicit QCrossPlatformSerialPort(QObject *parent = nullptr);

    /**
     * @brief Constructs a QCrossPlatformSerialPort for the given port name.
     * @param portName The name of the serial port
     * @param parent The parent QObject
     */
    explicit QCrossPlatformSerialPort(const QString &portName, QObject *parent = nullptr);

    /**
     * @brief Constructs a QCrossPlatformSerialPort for the given port info.
     * @param portInfo The serial port info
     * @param parent The parent QObject
     */
    explicit QCrossPlatformSerialPort(const QCrossPlatformSerialPortInfo &portInfo, QObject *parent = nullptr);

    /**
     * @brief Destroys the QCrossPlatformSerialPort object.
     */
    ~QCrossPlatformSerialPort();

    /**
     * @brief Sets the name of the serial port.
     * @param name The port name
     */
    void setPortName(const QString &name);

    /**
     * @brief Returns the name of the serial port.
     * @return Port name as QString
     */
    QString portName() const;

    /**
     * @brief Opens the serial port with the specified mode.
     * @param mode The open mode (ReadWrite, ReadOnly, WriteOnly)
     * @return true if successful, false otherwise
     */
    bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);

    /**
     * @brief Closes the serial port.
     */
    void close();

    /**
     * @brief Returns true if the serial port is open.
     * @return true if open, false otherwise
     */
    bool isOpen() const;

    /**
     * @brief Writes data to the serial port.
     * @param data The data to write
     * @return Number of bytes written, or -1 on error
     */
    qint64 write(const QByteArray &data);

    /**
     * @brief Reads all available data from the serial port.
     * @return All available data as QByteArray
     */
    QByteArray readAll();

    /**
     * @brief Sets the baud rate of the serial port.
     * @param baudRate The baud rate to set
     * @param directions The directions to apply (default: AllDirections)
     * @return true if successful, false otherwise
     */
    bool setBaudRate(qint32 baudRate, QCrossPlatformDirections directions = QCrossPlatformDirections::AllDirections);

    /**
     * @brief Returns the current baud rate.
     * @return The baud rate, or -1 on error
     */
    qint32 baudRate(QCrossPlatformDirections directions = QCrossPlatformDirections::AllDirections) const;

    /**
     * @brief Sets the data bits of the serial port.
     * @param dataBits The data bits to set
     * @return true if successful, false otherwise
     */
    bool setDataBits(QCrossPlatformDataBits dataBits);

    /**
     * @brief Returns the current data bits.
     * @return The data bits
     */
    QCrossPlatformDataBits dataBits() const;

    /**
     * @brief Sets the parity of the serial port.
     * @param parity The parity to set
     * @return true if successful, false otherwise
     */
    bool setParity(QCrossPlatformParity parity);

    /**
     * @brief Returns the current parity.
     * @return The parity
     */
    QCrossPlatformParity parity() const;

    /**
     * @brief Sets the stop bits of the serial port.
     * @param stopBits The stop bits to set
     * @return true if successful, false otherwise
     */
    bool setStopBits(QCrossPlatformStopBits stopBits);

    /**
     * @brief Returns the current stop bits.
     * @return The stop bits
     */
    QCrossPlatformStopBits stopBits() const;

    /**
     * @brief Sets the flow control of the serial port.
     * @param flowControl The flow control to set
     * @return true if successful, false otherwise
     */
    bool setFlowControl(QCrossPlatformFlowControl flowControl);

    /**
     * @brief Returns the current flow control.
     * @return The flow control
     */
    QCrossPlatformFlowControl flowControl() const;

    /**
     * @brief Clears the specified buffers.
     * @param directions The buffers to clear
     * @return true if successful, false otherwise
     */
    bool clear(QCrossPlatformDirections directions = QCrossPlatformDirections::AllDirections);

    /**
     * @brief Flushes the serial port buffers.
     * @return true if successful, false otherwise
     */
    bool flush();

    /**
     * @brief Returns the number of bytes available for reading.
     * @return Number of available bytes
     */
    qint64 bytesAvailable() const;

    /**
     * @brief Returns the number of bytes waiting to be written.
     * @return Number of bytes waiting to be written
     */
    qint64 bytesToWrite() const;

    /**
     * @brief Returns the last error that occurred.
     * @return The last error
     */
    QCrossPlatformSerialPortError error() const;

    /**
     * @brief Clears the error state.
     */
    void clearError();

signals:
    /**
     * @brief This signal is emitted when new data is available for reading.
     */
    void readyRead();

    /**
     * @brief This signal is emitted when data has been written to the serial port.
     * @param bytes The number of bytes written
     */
    void bytesWritten(qint64 bytes);

    /**
     * @brief This signal is emitted when an error occurs.
     * @param error The error that occurred
     */
    void errorOccurred(QCrossPlatformSerialPortError error);

private:
    // Pimpl pattern - implementation details hidden
    QCrossPlatformSerialPortPrivate *d;
};
