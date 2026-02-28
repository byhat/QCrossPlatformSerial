#pragma once

#include <QString>
#include <QList>
#include <qglobal.h>

// Forward declarations for Pimpl pattern
class QCrossPlatformSerialPortInfoPrivate;

#ifndef Q_OS_ANDROID
// Forward declaration for desktop private constructor
class QSerialPortInfo;
#endif

/**
 * @brief The QCrossPlatformSerialPortInfo class provides information about serial ports.
 *
 * This class provides a cross-platform interface similar to QSerialPortInfo.
 * Platform-specific implementation details are hidden using Pimpl pattern.
 * On Android, it uses JNI to query USB serial devices.
 * On Desktop, it wraps QSerialPortInfo.
 *
 * This class contains NO QML-related code - no Q_PROPERTY, no Q_INVOKABLE, no signals.
 */
class QCrossPlatformSerialPortInfo
{
public:
    /**
     * @brief Constructs an empty QCrossPlatformSerialPortInfo object.
     */
    QCrossPlatformSerialPortInfo();

    /**
     * @brief Copy constructor.
     * @param other The QCrossPlatformSerialPortInfo to copy
     */
    QCrossPlatformSerialPortInfo(const QCrossPlatformSerialPortInfo &other);

    /**
     * @brief Assignment operator.
     * @param other The QCrossPlatformSerialPortInfo to assign from
     * @return Reference to this object
     */
    QCrossPlatformSerialPortInfo &operator=(const QCrossPlatformSerialPortInfo &other);

    /**
     * @brief Destroys the QCrossPlatformSerialPortInfo object.
     */
    ~QCrossPlatformSerialPortInfo();

    /**
     * @brief Returns the name of the port.
     * @return Port name as QString
     */
    QString portName() const;

    /**
     * @brief Returns the description string of the port.
     * @return Description as QString
     */
    QString description() const;

    /**
     * @brief Returns the manufacturer string of the port.
     * @return Manufacturer name as QString
     */
    QString manufacturer() const;

    /**
     * @brief Returns the serial number string of the port.
     * @return Serial number as QString
     */
    QString serialNumber() const;

    /**
     * @brief Returns the 16-bit product number of the port.
     * @return Product identifier as quint16
     */
    quint16 productIdentifier() const;

    /**
     * @brief Returns the 16-bit vendor number of the port.
     * @return Vendor identifier as quint16
     */
    quint16 vendorIdentifier() const;

    /**
     * @brief Returns the system location of the port.
     * @return System location as QString
     */
    QString systemLocation() const;

    /**
     * @brief Returns a list of available standard baud rates.
     * @return List of standard baud rates as QList<qint32>
     */
    QList<qint32> standardBaudRates() const;

    /**
     * @brief Returns a list of available serial ports on the system.
     *
     * This static method queries the platform for available serial devices.
     * On Android, it uses JNI to query USB serial devices.
     * On Desktop, it uses QSerialPortInfo.
     *
     * @return List of available QCrossPlatformSerialPortInfo objects
     */
    static QList<QCrossPlatformSerialPortInfo> availablePorts();

    /**
     * @brief Returns true if the port info is null (empty).
     * @return true if null, false otherwise
     */
    bool isNull() const;

    /**
     * @brief Swaps this serial port info with other.
     * @param other The QCrossPlatformSerialPortInfo to swap with
     */
    void swap(QCrossPlatformSerialPortInfo &other);

    // Internal constructor for creating from device info (used by availablePorts())
    // This is public for internal use but not intended for external API
    QCrossPlatformSerialPortInfo(const QString &portName,
                                 const QString &description,
                                 const QString &manufacturer,
                                 const QString &serialNumber,
                                 quint16 productIdentifier,
                                 quint16 vendorIdentifier,
                                 const QString &systemLocation);

#ifndef Q_OS_ANDROID
    // Desktop internal constructor from QSerialPortInfo
    explicit QCrossPlatformSerialPortInfo(const QSerialPortInfo &info);
#endif

private:
    // Pimpl pattern - implementation details hidden
    QCrossPlatformSerialPortInfoPrivate *d;
};
