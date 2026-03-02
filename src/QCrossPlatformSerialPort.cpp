#include "QCrossPlatformSerialPort.hpp"
#include "QCrossPlatformSerialPortInfo.hpp"

// Platform-specific includes
#ifdef Q_OS_ANDROID
    #include <QJniObject>
    #include <QGuiApplication>
    #include <QTimer>
    #include <QMutex>
    #include <QThread>
#else
    #include <QSerialPort>
#endif

#include <QDebug>

/**
 * @brief Private implementation class for QCrossPlatformSerialPort.
 *
 * This class contains all platform-specific implementation details,
 * hidden from users of the public QCrossPlatformSerialPort class.
 */
class QCrossPlatformSerialPortPrivate : public QObject
{
    Q_OBJECT
public:
    QCrossPlatformSerialPort *q;
    QString m_portName;
    QIODevice::OpenMode m_openMode;
    QCrossPlatformDataBits m_dataBits;
    QCrossPlatformParity m_parity;
    QCrossPlatformStopBits m_stopBits;
    QCrossPlatformFlowControl m_flowControl;
    qint32 m_baudRate;
    QCrossPlatformSerialPortError m_error;
    bool m_isOpen;

#ifdef Q_OS_ANDROID
    // Android-specific implementation
    QByteArray m_readBuffer;
    QByteArray m_writeBuffer;
    QMutex m_readMutex;
    QMutex m_writeMutex;
    QTimer *m_readTimer;
    qint64 m_bytesWrittenPending;
    QThread *m_workerThread;  // Worker thread for timer operations
    qint64 m_lastBufferSize;  // Track buffer size to detect new data
#else
    // Desktop: Wrap QSerialPort
    QSerialPort *m_serialPort;
#endif

    QCrossPlatformSerialPortPrivate(QCrossPlatformSerialPort *q_ptr)
        : q(q_ptr)
        , m_openMode(QIODevice::NotOpen)
        , m_dataBits(QCrossPlatformDataBits::Data8)
        , m_parity(QCrossPlatformParity::NoParity)
        , m_stopBits(QCrossPlatformStopBits::OneStop)
        , m_flowControl(QCrossPlatformFlowControl::NoFlowControl)
        , m_baudRate(9600)
        , m_error(QCrossPlatformSerialPortError::NoError)
        , m_isOpen(false)
#ifdef Q_OS_ANDROID
        , m_bytesWrittenPending(0)
        , m_workerThread(nullptr)
        , m_lastBufferSize(0)
#else
        , m_serialPort(nullptr)
#endif
    {
#ifdef Q_OS_ANDROID
        // Create worker thread for timer operations
        m_workerThread = new QThread(q);
        m_readTimer = new QTimer();
        m_readTimer->moveToThread(m_workerThread);
        
        // Start the worker thread
        m_workerThread->start();
        
        // Connect timer timeout to checkForData
        // Since timer is on worker thread and 'this' is on main thread,
        // Qt will automatically use Qt::QueuedConnection
        QObject::connect(m_readTimer, &QTimer::timeout, this, &QCrossPlatformSerialPortPrivate::checkForData);
        
        // Clean up worker thread when timer is destroyed
        QObject::connect(m_readTimer, &QTimer::destroyed, m_workerThread, &QThread::quit);
        QObject::connect(m_workerThread, &QThread::finished, m_workerThread, &QThread::deleteLater);
#else
        m_serialPort = new QSerialPort(q);
        // Connect signals from QSerialPort to our public interface
        QObject::connect(m_serialPort, &QSerialPort::readyRead, q, &QCrossPlatformSerialPort::readyRead);
        QObject::connect(m_serialPort, &QSerialPort::bytesWritten, q, [this](qint64 bytes) {
            emit q->bytesWritten(bytes);
        });
        QObject::connect(m_serialPort, &QSerialPort::errorOccurred, q, [this](QSerialPort::SerialPortError error) {
            m_error = convertError(error);
            emit q->errorOccurred(m_error);
        });
#endif
    }

    ~QCrossPlatformSerialPortPrivate()
    {
#ifdef Q_OS_ANDROID
        // Stop the timer first
        if (m_readTimer) {
            if (m_readTimer->thread() != QThread::currentThread()) {
                // Timer is on worker thread, stop it using invokeMethod
                QMetaObject::invokeMethod(m_readTimer, &QTimer::stop, Qt::BlockingQueuedConnection);
            } else {
                m_readTimer->stop();
            }
            m_readTimer->deleteLater();
            m_readTimer = nullptr;
        }
        // Stop the worker thread
        if (m_workerThread) {
            if (m_workerThread->isRunning()) {
                m_workerThread->quit();
                if (!m_workerThread->wait(2000)) {
                    qWarning() << "Worker thread did not finish, forcing termination";
                    m_workerThread->terminate();
                    m_workerThread->wait();
                }
            }
            m_workerThread = nullptr;
        }
#else
        if (m_serialPort) {
            m_serialPort->close();
            delete m_serialPort;
        }
#endif
    }

#ifdef Q_OS_ANDROID
public slots:
    void checkForData()
    {
        if (!m_isOpen) {
            return;
        }

        try {
            QJniObject result = QJniObject::callStaticMethod<jstring>(
                "org/qtserial/UsbSerialManager",
                "readData",
                "()Ljava/lang/String;"
            );

            if (result.isValid()) {
                QString data = result.toString();
                if (!data.isEmpty() && data != "No data available or port is not connected.") {
                    // Only hold mutex briefly to append data
                    {
                        QMutexLocker locker(&m_readMutex);
                        m_readBuffer.append(data.toUtf8());
                    }
                    // Emit signal without holding mutex
                    qDebug() << "Read" << data.size() << "bytes from serial port";
                    emit q->readyRead();
                }
            } else {
                qWarning() << "Failed to call readData() from Java - result is invalid";
            }
        } catch (const std::exception &e) {
            qWarning() << "Exception in checkForData():" << e.what();
        }
    }

    void handleBytesWritten(qint64 bytes)
    {
        m_bytesWrittenPending -= bytes;
        if (m_bytesWrittenPending <= 0) {
            m_bytesWrittenPending = 0;
            emit q->bytesWritten(bytes);
        }
    }
#else
    static QSerialPort::Directions convertDirections(QCrossPlatformDirections directions)
    {
        return static_cast<QSerialPort::Directions>(static_cast<int>(directions));
    }

    static QCrossPlatformSerialPortError convertError(QSerialPort::SerialPortError error)
    {
        switch (error) {
            case QSerialPort::NoError:
                return QCrossPlatformSerialPortError::NoError;
            case QSerialPort::DeviceNotFoundError:
                return QCrossPlatformSerialPortError::DeviceNotFoundError;
            case QSerialPort::PermissionError:
                return QCrossPlatformSerialPortError::PermissionError;
            case QSerialPort::OpenError:
                return QCrossPlatformSerialPortError::OpenError;
            case QSerialPort::WriteError:
                return QCrossPlatformSerialPortError::WriteError;
            case QSerialPort::ReadError:
                return QCrossPlatformSerialPortError::ReadError;
            case QSerialPort::ResourceError:
                return QCrossPlatformSerialPortError::ResourceError;
            case QSerialPort::UnsupportedOperationError:
                return QCrossPlatformSerialPortError::UnsupportedOperationError;
            case QSerialPort::UnknownError:
                return QCrossPlatformSerialPortError::UnknownError;
            case QSerialPort::TimeoutError:
                return QCrossPlatformSerialPortError::TimeoutError;
            case QSerialPort::NotOpenError:
                return QCrossPlatformSerialPortError::NotOpenError;
            default:
                return QCrossPlatformSerialPortError::UnknownError;
        }
    }

    static QSerialPort::DataBits convertDataBits(QCrossPlatformDataBits dataBits)
    {
        return static_cast<QSerialPort::DataBits>(static_cast<int>(dataBits));
    }

    static QSerialPort::Parity convertParity(QCrossPlatformParity parity)
    {
        return static_cast<QSerialPort::Parity>(static_cast<int>(parity));
    }

    static QSerialPort::StopBits convertStopBits(QCrossPlatformStopBits stopBits)
    {
        return static_cast<QSerialPort::StopBits>(static_cast<int>(stopBits));
    }

    static QSerialPort::FlowControl convertFlowControl(QCrossPlatformFlowControl flowControl)
    {
        return static_cast<QSerialPort::FlowControl>(static_cast<int>(flowControl));
    }

    static QCrossPlatformDataBits convertDataBits(QSerialPort::DataBits dataBits)
    {
        return static_cast<QCrossPlatformDataBits>(static_cast<int>(dataBits));
    }

    static QCrossPlatformParity convertParity(QSerialPort::Parity parity)
    {
        return static_cast<QCrossPlatformParity>(static_cast<int>(parity));
    }

    static QCrossPlatformStopBits convertStopBits(QSerialPort::StopBits stopBits)
    {
        return static_cast<QCrossPlatformStopBits>(static_cast<int>(stopBits));
    }

    static QCrossPlatformFlowControl convertFlowControl(QSerialPort::FlowControl flowControl)
    {
        return static_cast<QCrossPlatformFlowControl>(static_cast<int>(flowControl));
    }
#endif
};

QCrossPlatformSerialPort::QCrossPlatformSerialPort(QObject *parent)
    : QObject(parent)
    , d(new QCrossPlatformSerialPortPrivate(this))
{
}

QCrossPlatformSerialPort::QCrossPlatformSerialPort(const QString &portName, QObject *parent)
    : QObject(parent)
    , d(new QCrossPlatformSerialPortPrivate(this))
{
    d->m_portName = portName;
}

QCrossPlatformSerialPort::QCrossPlatformSerialPort(const QCrossPlatformSerialPortInfo &portInfo, QObject *parent)
    : QObject(parent)
    , d(new QCrossPlatformSerialPortPrivate(this))
{
    d->m_portName = portInfo.portName();
}

QCrossPlatformSerialPort::~QCrossPlatformSerialPort()
{
    close();
    delete d;
}

void QCrossPlatformSerialPort::setPortName(const QString &name)
{
    d->m_portName = name;
}

QString QCrossPlatformSerialPort::portName() const
{
    return d->m_portName;
}

bool QCrossPlatformSerialPort::open(QIODevice::OpenMode mode)
{
    if (d->m_isOpen) {
        d->m_error = QCrossPlatformSerialPortError::OpenError;
        emit errorOccurred(d->m_error);
        return false;
    }

#ifdef Q_OS_ANDROID
    // Android implementation using JNI
    QJniObject activity = QJniObject::callStaticObjectMethod("android/app/ActivityThread", "currentApplication", "()Landroid/app/Application;");
    
    // Initialize UsbSerialManager first
    QJniObject::callStaticMethod<void>(
        "org/qtserial/UsbSerialManager",
        "init",
        "(Landroid/content/Context;)V",
        activity.object<jobject>()
    );
    
    // Connect to device
    jboolean result = QJniObject::callStaticMethod<jboolean>(
        "org/qtserial/UsbSerialManager",
        "connectToDevice",
        "(Landroid/content/Context;)Z",
        activity.object<jobject>()
    );

    if (result) {
        d->m_isOpen = true;
        d->m_openMode = mode;
        // Start timer on worker thread using QMetaObject::invokeMethod
        QMetaObject::invokeMethod(d->m_readTimer, [timer = d->m_readTimer]() {
            timer->start(50); // Check for data every 50ms
        }, Qt::QueuedConnection);
        qDebug() << "Serial port opened successfully on Android";
        return true;
    } else {
        d->m_error = QCrossPlatformSerialPortError::OpenError;
        emit errorOccurred(d->m_error);
        qWarning() << "Failed to open serial port on Android";
        return false;
    }
#else
    // Desktop implementation using QSerialPort
    d->m_serialPort->setPortName(d->m_portName);

    // Apply configured settings before opening
    d->m_serialPort->setBaudRate(d->m_baudRate);
    d->m_serialPort->setDataBits(QCrossPlatformSerialPortPrivate::convertDataBits(d->m_dataBits));
    d->m_serialPort->setParity(QCrossPlatformSerialPortPrivate::convertParity(d->m_parity));
    d->m_serialPort->setStopBits(QCrossPlatformSerialPortPrivate::convertStopBits(d->m_stopBits));
    d->m_serialPort->setFlowControl(QCrossPlatformSerialPortPrivate::convertFlowControl(d->m_flowControl));

    if (d->m_serialPort->open(mode)) {
        d->m_isOpen = true;
        d->m_openMode = mode;
        d->m_error = QCrossPlatformSerialPortError::NoError;
        return true;
    } else {
        d->m_error = QCrossPlatformSerialPortPrivate::convertError(d->m_serialPort->error());
        emit errorOccurred(d->m_error);
        return false;
    }
#endif
}

void QCrossPlatformSerialPort::close()
{
    if (!d->m_isOpen) {
        return;
    }

#ifdef Q_OS_ANDROID
    // Stop timer on worker thread using QMetaObject::invokeMethod
    QMetaObject::invokeMethod(d->m_readTimer, [timer = d->m_readTimer]() {
        timer->stop();
    }, Qt::QueuedConnection);
    // Note: We don't close USB connection on Android as it may be shared
    // Just mark as closed
    d->m_isOpen = false;
    d->m_openMode = QIODevice::NotOpen;
#else
    d->m_serialPort->close();
    d->m_isOpen = false;
    d->m_openMode = QIODevice::NotOpen;
#endif
}

bool QCrossPlatformSerialPort::isOpen() const
{
    return d->m_isOpen;
}

qint64 QCrossPlatformSerialPort::write(const QByteArray &data)
{
    if (!d->m_isOpen) {
        d->m_error = QCrossPlatformSerialPortError::NotOpenError;
        emit errorOccurred(d->m_error);
        return -1;
    }

#ifdef Q_OS_ANDROID
    QMutexLocker locker(&d->m_writeMutex);
    QJniObject javaData = QJniObject::fromString(QString::fromUtf8(data));
    // Use wrapper method that doesn't require Context parameter
    jboolean result = QJniObject::callStaticMethod<jboolean>(
        "org/qtserial/UsbSerialManager",
        "sendData",
        "(Ljava/lang/String;)Z",
        javaData.object<jstring>()
    );

    if (result) {
        d->m_bytesWrittenPending += data.size();
        // Emit bytesWritten asynchronously
        QTimer::singleShot(10, this, [this, size = data.size()]() {
            emit bytesWritten(size);
        });
        qDebug() << "Wrote" << data.size() << "bytes to serial port";
        return data.size();
    } else {
        d->m_error = QCrossPlatformSerialPortError::WriteError;
        emit errorOccurred(d->m_error);
        qWarning() << "Failed to write to serial port";
        return -1;
    }
#else
    qint64 bytesWritten = d->m_serialPort->write(data);
    if (bytesWritten < 0) {
        d->m_error = QCrossPlatformSerialPortPrivate::convertError(d->m_serialPort->error());
        emit errorOccurred(d->m_error);
    }
    return bytesWritten;
#endif
}

QByteArray QCrossPlatformSerialPort::readAll()
{
#ifdef Q_OS_ANDROID
    QMutexLocker locker(&d->m_readMutex);
    QByteArray data = d->m_readBuffer;
    d->m_readBuffer.clear();
    return data;
#else
    return d->m_serialPort->readAll();
#endif
}

bool QCrossPlatformSerialPort::setBaudRate(qint32 baudRate, QCrossPlatformDirections directions)
{
    d->m_baudRate = baudRate;

#ifndef Q_OS_ANDROID
    return d->m_serialPort->setBaudRate(baudRate, QCrossPlatformSerialPortPrivate::convertDirections(directions));
#else
    // On Android, baud rate is typically handled by USB-to-serial adapter
    // We store it for reference but actual configuration may be limited
    return true;
#endif
}

qint32 QCrossPlatformSerialPort::baudRate(QCrossPlatformDirections directions) const
{
#ifndef Q_OS_ANDROID
    return d->m_serialPort->baudRate(QCrossPlatformSerialPortPrivate::convertDirections(directions));
#else
    return d->m_baudRate;
#endif
}

bool QCrossPlatformSerialPort::setDataBits(QCrossPlatformDataBits dataBits)
{
    d->m_dataBits = dataBits;

#ifndef Q_OS_ANDROID
    return d->m_serialPort->setDataBits(QCrossPlatformSerialPortPrivate::convertDataBits(dataBits));
#else
    // On Android, data bits are typically handled by USB-to-serial adapter
    return true;
#endif
}

QCrossPlatformDataBits QCrossPlatformSerialPort::dataBits() const
{
#ifndef Q_OS_ANDROID
    return QCrossPlatformSerialPortPrivate::convertDataBits(d->m_serialPort->dataBits());
#else
    return d->m_dataBits;
#endif
}

bool QCrossPlatformSerialPort::setParity(QCrossPlatformParity parity)
{
    d->m_parity = parity;

#ifndef Q_OS_ANDROID
    return d->m_serialPort->setParity(QCrossPlatformSerialPortPrivate::convertParity(parity));
#else
    // On Android, parity is typically handled by USB-to-serial adapter
    return true;
#endif
}

QCrossPlatformParity QCrossPlatformSerialPort::parity() const
{
#ifndef Q_OS_ANDROID
    return QCrossPlatformSerialPortPrivate::convertParity(d->m_serialPort->parity());
#else
    return d->m_parity;
#endif
}

bool QCrossPlatformSerialPort::setStopBits(QCrossPlatformStopBits stopBits)
{
    d->m_stopBits = stopBits;

#ifndef Q_OS_ANDROID
    return d->m_serialPort->setStopBits(QCrossPlatformSerialPortPrivate::convertStopBits(stopBits));
#else
    // On Android, stop bits are typically handled by USB-to-serial adapter
    return true;
#endif
}

QCrossPlatformStopBits QCrossPlatformSerialPort::stopBits() const
{
#ifndef Q_OS_ANDROID
    return QCrossPlatformSerialPortPrivate::convertStopBits(d->m_serialPort->stopBits());
#else
    return d->m_stopBits;
#endif
}

bool QCrossPlatformSerialPort::setFlowControl(QCrossPlatformFlowControl flowControl)
{
    d->m_flowControl = flowControl;

#ifndef Q_OS_ANDROID
    return d->m_serialPort->setFlowControl(QCrossPlatformSerialPortPrivate::convertFlowControl(flowControl));
#else
    // On Android, flow control is typically handled by USB-to-serial adapter
    return true;
#endif
}

QCrossPlatformFlowControl QCrossPlatformSerialPort::flowControl() const
{
#ifndef Q_OS_ANDROID
    return QCrossPlatformSerialPortPrivate::convertFlowControl(d->m_serialPort->flowControl());
#else
    return d->m_flowControl;
#endif
}

bool QCrossPlatformSerialPort::clear(QCrossPlatformDirections directions)
{
#ifndef Q_OS_ANDROID
    return d->m_serialPort->clear(QCrossPlatformSerialPortPrivate::convertDirections(directions));
#else
    if (static_cast<int>(directions) & static_cast<int>(QCrossPlatformDirections::Input)) {
        QMutexLocker locker(&d->m_readMutex);
        d->m_readBuffer.clear();
    }
    if (static_cast<int>(directions) & static_cast<int>(QCrossPlatformDirections::Output)) {
        QMutexLocker locker(&d->m_writeMutex);
        d->m_writeBuffer.clear();
    }
    return true;
#endif
}

bool QCrossPlatformSerialPort::flush()
{
#ifndef Q_OS_ANDROID
    return d->m_serialPort->flush();
#else
    // On Android, flushing is handled by Java layer
    return true;
#endif
}

qint64 QCrossPlatformSerialPort::bytesAvailable() const
{
#ifndef Q_OS_ANDROID
    return d->m_serialPort->bytesAvailable();
#else
    QMutexLocker locker(&d->m_readMutex);
    return d->m_readBuffer.size();
#endif
}

qint64 QCrossPlatformSerialPort::bytesToWrite() const
{
#ifndef Q_OS_ANDROID
    return d->m_serialPort->bytesToWrite();
#else
    return d->m_bytesWrittenPending;
#endif
}

QCrossPlatformSerialPortError QCrossPlatformSerialPort::error() const
{
    return d->m_error;
}

void QCrossPlatformSerialPort::clearError()
{
    d->m_error = QCrossPlatformSerialPortError::NoError;
}

#include "QCrossPlatformSerialPort.moc"
