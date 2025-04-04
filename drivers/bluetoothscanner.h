#ifndef BLUETOOTHSCANNER_H
#define BLUETOOTHSCANNER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QStringList> // Voor apparaatlijst

// Klasse verantwoordelijk voor het scannen van Bluetooth-apparaten
class BluetoothScanner : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothScanner(QObject *parent = nullptr);
    ~BluetoothScanner();

    bool isScanning() const; // Controleer of er momenteel wordt gescand

public slots:
    void startScan(); // Start het scannen naar Bluetooth-apparaten
    void stopScan();  // Stop het scannen naar Bluetooth-apparaten

signals:
    void deviceDiscovered(const QBluetoothDeviceInfo &device); // Signaal voor ontdekte apparaten
    void scanFinished(); // Signaal wanneer het scannen is voltooid
    void scanError(const QString &error); // Signaal voor fouten tijdens het scannen
    void statusChanged(const QString &status); // Signaal voor statuswijzigingen
    void scanningChanged(bool scanning); // Signaal voor wijzigingen in de scanstatus
    void scanningUnavailable(); // Signal to indicate bluetooth is unavailable


private slots:
    // Interne slots om signalen van de discovery agent te verwerken
    void handleDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void handleScanFinished();
    void handleScanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent; // Agent voor het ontdekken van Bluetooth-apparaten
    QStringList m_deviceList; // Lijst van ontdekte apparaten
    bool m_scanning; // Huidige scanstatus
};

#endif // BLUETOOTHSCANNER_H