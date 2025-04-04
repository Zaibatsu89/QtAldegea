#include "bluetoothscanner.h"
#include <QDebug>

BluetoothScanner::BluetoothScanner(QObject *parent) : QObject(parent)
{
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    if (!m_discoveryAgent) { // Check ook of new gefaald is
        emit statusChanged("Error: Bluetooth is niet beschikbaar op dit apparaat.");
        emit scanError("Bluetooth is niet beschikbaar.");
        qWarning() << "Bluetooth is niet beschikbaar op dit apparaat.";
        delete m_discoveryAgent; // Voorkom verdere problemen
        m_discoveryAgent = nullptr;
        emit scanningUnavailable(); // Signal to disable scanning button in UI
        return;
    }

    // Verbind de signalen van de agent met onze interne slots
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothScanner::deviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BluetoothScanner::scanFinished);
    // Gebruik de nieuwe connect-syntax voor overloaded signalen
    connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::errorOccurred),
            this, &BluetoothScanner::handleScanError);

    emit statusChanged("Klaar om te scannen.");
}

BluetoothScanner::~BluetoothScanner()
{
    // discoveryAgent wordt automatisch verwijderd door de QObject destructor
    // als het niet als is verwijderd in de constructor
}

bool BluetoothScanner::isScanning() const
{
    return m_discoveryAgent && m_discoveryAgent->isActive();
}

void BluetoothScanner::startScan()
{
    if (!m_discoveryAgent) {
        emit statusChanged("Error: Kan niet scannen, Bluetooth niet beschikbaar.");
        emit scanError("Bluetooth niet beschikbaar.");
        return;
    }

    if (m_scanning) {
        qDebug() << "Scan already in progress, ignoring start request.";
        return; // Al aan het scannen
    }

    m_deviceList.clear(); // Reset lijst voor nieuwe scan
    emit statusChanged("Scannen naar apparaten...");
    m_scanning = true;
    emit scanningChanged(true); // Laat UI weten dat scan start
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod); // Start scan (of gebruik ClassicMethod)

    // Optioneel: Voeg een timeout toe voor het geval finished() nooit komt
    // QTimer::singleShot(30000, this, &BluetoothScanner::stopScan); // Stop na 30s
}

void BluetoothScanner::stopScan()
{
     if (!m_discoveryAgent || !m_scanning) {
        return;
    }
    m_discoveryAgent->stop();
    // handleScanFinished wordt normaal gesproken aangeroepen via het 'finished' signaal
}

void BluetoothScanner::handleDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo)
{
    // Filter eventueel ongewenste apparaten (bv. zonder naam of niet Low Energy)
    if (deviceInfo.isValid() && !deviceInfo.name().trimmed().isEmpty() && (deviceInfo.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
    {
        // Optioneel: voorkom dubbele signalen voor hetzelfde apparaat in één scan
        QString identifier = deviceInfo.address().toString();
        if (!m_deviceList.contains(identifier)) {
             m_deviceList.append(identifier);
              qDebug() << "Apparaat gevonden:" << deviceInfo.name() << "(" << deviceInfo.address().toString() << ")";
              emit deviceDiscovered(deviceInfo); // Stuur door naar MainWindow
        }
    }
}

void BluetoothScanner::handleScanFinished()
{
    if (!m_scanning) return; // Voorkom afhandeling als we niet actief waren

    qDebug() << "Scan voltooid.";
    m_scanning = false;
    emit scanningChanged(false); // Laat UI weten dat scan stopt
    emit statusChanged("Scan voltooid.");
    emit scanFinished(); // Stuur door naar MainWindow
}

void BluetoothScanner::handleScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString errorMessage;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::NoError:
        // Dit zou niet moeten gebeuren via dit signaal
        return;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        errorMessage = "I/O Fout tijdens scannen.";
        break;
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorMessage = "Bluetooth staat uit.";
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        errorMessage = "Ongeldige Bluetooth adapter.";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        errorMessage = "Bluetooth wordt niet ondersteund op dit platform.";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnknownError:
    default:
        errorMessage = "Onbekende fout tijdens scannen.";
        break;
    }
    qWarning() << "Scan Error:" << errorMessage;
    m_scanning = false;
    emit scanningChanged(false); // Laat UI weten dat scan stopt
    emit statusChanged("Fout: " + errorMessage);
    emit scanError(errorMessage); // Stuur door naar MainWindow
}
