#include "QCrossPlatformSerialPortInfo.hpp"

// Platform-specific includes
#ifdef Q_OS_ANDROID
    #include <QJniObject>
    #include <QGuiApplication>
    #include <QJsonDocument>
    #include <QJsonArray>
    #include <QJsonObject>
#else
    #include <QSerialPortInfo>
#endif

#include <QDebug>

/**
 * @brief Private implementation class for QCrossPlatformSerialPortInfo.
 *
 * This class contains all platform-specific implementation details,
 * hidden from users of the public QCrossPlatformSerialPortInfo class.
 */
class QCrossPlatformSerialPortInfoPrivate
{
public:
    QString m_portName;
    QString m_description;
    QString m_manufacturer;
    QString m_serialNumber;
    quint16 m_productIdentifier;
    quint16 m_vendorIdentifier;
    QString m_systemLocation;
    bool m_isNull;

#ifdef Q_OS_ANDROID
    // Android-specific: No additional data needed
#else
    // Desktop: We may wrap a QSerialPortInfo object
    QSerialPortInfo m_serialPortInfo;
#endif

    QCrossPlatformSerialPortInfoPrivate()
        : m_productIdentifier(0)
        , m_vendorIdentifier(0)
        , m_isNull(true)
    {
    }

    QCrossPlatformSerialPortInfoPrivate(const QString &portName,
                                        const QString &description,
                                        const QString &manufacturer,
                                        const QString &serialNumber,
                                        quint16 productIdentifier,
                                        quint16 vendorIdentifier,
                                        const QString &systemLocation)
        : m_portName(portName)
        , m_description(description)
        , m_manufacturer(manufacturer)
        , m_serialNumber(serialNumber)
        , m_productIdentifier(productIdentifier)
        , m_vendorIdentifier(vendorIdentifier)
        , m_systemLocation(systemLocation)
        , m_isNull(false)
    {
    }

#ifndef Q_OS_ANDROID
    // Desktop constructor from QSerialPortInfo
    explicit QCrossPlatformSerialPortInfoPrivate(const QSerialPortInfo &info)
        : m_portName(info.portName())
        , m_description(info.description())
        , m_manufacturer(info.manufacturer())
        , m_serialNumber(info.serialNumber())
        , m_productIdentifier(info.productIdentifier())
        , m_vendorIdentifier(info.vendorIdentifier())
        , m_systemLocation(info.systemLocation())
        , m_isNull(info.isNull())
        , m_serialPortInfo(info)
    {
    }
#endif
};

QCrossPlatformSerialPortInfo::QCrossPlatformSerialPortInfo()
    : d(new QCrossPlatformSerialPortInfoPrivate())
{
}

QCrossPlatformSerialPortInfo::QCrossPlatformSerialPortInfo(const QCrossPlatformSerialPortInfo &other)
    : d(new QCrossPlatformSerialPortInfoPrivate())
{
    *d = *other.d;
}

// Private constructor for internal use
QCrossPlatformSerialPortInfo::QCrossPlatformSerialPortInfo(const QString &portName,
                                                             const QString &description,
                                                             const QString &manufacturer,
                                                             const QString &serialNumber,
                                                             quint16 productIdentifier,
                                                             quint16 vendorIdentifier,
                                                             const QString &systemLocation)
    : d(new QCrossPlatformSerialPortInfoPrivate(portName, description, manufacturer,
                                               serialNumber, productIdentifier,
                                               vendorIdentifier, systemLocation))
{
}

#ifndef Q_OS_ANDROID
// Desktop private constructor from QSerialPortInfo
QCrossPlatformSerialPortInfo::QCrossPlatformSerialPortInfo(const QSerialPortInfo &info)
    : d(new QCrossPlatformSerialPortInfoPrivate(info))
{
}
#endif

QCrossPlatformSerialPortInfo &QCrossPlatformSerialPortInfo::operator=(const QCrossPlatformSerialPortInfo &other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

QCrossPlatformSerialPortInfo::~QCrossPlatformSerialPortInfo()
{
    delete d;
}

QString QCrossPlatformSerialPortInfo::portName() const
{
    return d->m_portName;
}

QString QCrossPlatformSerialPortInfo::description() const
{
    return d->m_description;
}

QString QCrossPlatformSerialPortInfo::manufacturer() const
{
    return d->m_manufacturer;
}

QString QCrossPlatformSerialPortInfo::serialNumber() const
{
    return d->m_serialNumber;
}

quint16 QCrossPlatformSerialPortInfo::productIdentifier() const
{
    return d->m_productIdentifier;
}

quint16 QCrossPlatformSerialPortInfo::vendorIdentifier() const
{
    return d->m_vendorIdentifier;
}

QString QCrossPlatformSerialPortInfo::systemLocation() const
{
    return d->m_systemLocation;
}

QList<qint32> QCrossPlatformSerialPortInfo::standardBaudRates() const
{
    // Return standard baud rates commonly supported by serial devices
    static const QList<qint32> standardRates = {
        1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
        230400, 460800, 921600
    };
    return standardRates;
}

bool QCrossPlatformSerialPortInfo::isNull() const
{
    return d->m_isNull;
}

void QCrossPlatformSerialPortInfo::swap(QCrossPlatformSerialPortInfo &other)
{
    std::swap(d, other.d);
}

#ifdef Q_OS_ANDROID
/**
 * @brief Android implementation - parses device info from JSON string.
 */
static QCrossPlatformSerialPortInfo fromDeviceInfoString(const QString &deviceInfoString)
{
    // Parse device info JSON string from Java JNI call
    // Expected JSON format: {"portName":"...","description":"...","manufacturer":"...","serialNumber":"...","productId":...,"vendorId":...,"systemLocation":"..."}

    QJsonDocument doc = QJsonDocument::fromJson(deviceInfoString.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid device info JSON format:" << deviceInfoString;
        return QCrossPlatformSerialPortInfo();
    }

    QJsonObject obj = doc.object();

    QString portName = obj.value("portName").toString();
    QString description = obj.value("description").toString();
    QString manufacturer = obj.value("manufacturer").toString();
    QString serialNumber = obj.value("serialNumber").toString();
    quint16 productId = static_cast<quint16>(obj.value("productId").toInt(0));
    quint16 vendorId = static_cast<quint16>(obj.value("vendorId").toInt(0));
    QString systemLocation = obj.value("systemLocation").toString();

    return QCrossPlatformSerialPortInfo(portName, description, manufacturer, serialNumber,
                                         productId, vendorId, systemLocation);
}
#endif

QList<QCrossPlatformSerialPortInfo> QCrossPlatformSerialPortInfo::availablePorts()
{
    QList<QCrossPlatformSerialPortInfo> ports;

#ifdef Q_OS_ANDROID
    // Android implementation: Use JNI to query USB serial devices
    qInfo() << "Querying available USB serial devices on Android...";
    
    try {
        // Initialize UsbManager first
        QJniObject activity = QJniObject::callStaticObjectMethod("android/app/ActivityThread", "currentApplication", "()Landroid/app/Application;");
        if (!activity.isValid()) {
            qWarning() << "Failed to get Android activity context";
            return ports;
        }
        
        // Call Java method JniUsbSerial.availableDevicesInfo() which returns a JSONArray
        QJniObject result = QJniObject::callStaticMethod<jobject>(
            "org/qtserial/JniUsbSerial",
            "availableDevicesInfo",
            "()Lorg/json/JSONArray;"
        );

        if (result.isValid()) {
            // Convert Java JSONArray to a JSON string
            QString jsonString = result.toString();
            qInfo() << "Received JSON from Java:" << jsonString;

            // Parse JSON string
            QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
            if (doc.isArray()) {
                QJsonArray devicesArray = doc.array();

                for (const QJsonValue &deviceValue : devicesArray) {
                    if (deviceValue.isObject()) {
                        QJsonObject deviceObj = deviceValue.toObject();

                        QString portName = deviceObj.value("portName").toString();
                        QString description = deviceObj.value("description").toString();
                        QString manufacturer = deviceObj.value("manufacturer").toString();
                        QString serialNumber = deviceObj.value("serialNumber").toString();
                        quint16 productId = static_cast<quint16>(deviceObj.value("productId").toInt(0));
                        quint16 vendorId = static_cast<quint16>(deviceObj.value("vendorId").toInt(0));
                        QString systemLocation = deviceObj.value("systemLocation").toString();

                        qInfo() << "Found device:" << portName << description;
                        ports.append(QCrossPlatformSerialPortInfo(portName, description, manufacturer,
                                                                  serialNumber, productId, vendorId,
                                                                  systemLocation));
                    }
                }

                qInfo() << "Found" << ports.size() << "USB serial devices on Android";
            } else {
                qWarning() << "Invalid JSON format returned from Java: expected array, got:" << jsonString;
            }
        } else {
            qWarning() << "Failed to call Java method availableDevicesInfo() - result is invalid";
        }
    } catch (const std::exception &e) {
        qWarning() << "Exception in availablePorts():" << e.what();
    }

#else
    // Desktop implementation: Use QSerialPortInfo directly
    QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : serialPortInfos) {
        ports.append(QCrossPlatformSerialPortInfo(info));
    }
    qInfo() << "Found" << ports.size() << "serial ports on desktop";
#endif

    return ports;
}
