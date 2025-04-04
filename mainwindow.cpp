#include "mainwindow.h"
#include "ui_mainwindow.h" // Correct pad gegenereerd door Qt/CMake
#include "drivers/bluetoothscanner.h" // Include onze scanner klasse

#include <QDebug>
#include <QLabel> // Nodig voor status label
#include <QCoreApplication> // Voor checkPermission/requestPermissions
#include <QBluetoothPermission> // Specifiek voor Bluetooth permissie types
#include <QLocationPermission> // Nodig voor locatie op oudere Android
#include <QOperatingSystemVersion> // Om Android versie te checken
#include <QFuture> // Voor asynchroon resultaat van requestPermissions
#include <QPermission>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scanner(new BluetoothScanner(this)) // Maak scanner aan (parent = this voor auto-cleanup)
{
    ui->setupUi(this);
    setupStatusLabel(); // Initialiseer status label
    setupUiConnections(); // Zet alle signaal/slot connecties op

    // Voeg geen dummy item meer toe bij start, wacht op scan
    updateStatusLabel("Klaar om te scannen.");
}

MainWindow::~MainWindow()
{
    // m_scanner wordt automatisch verwijderd omdat MainWindow de parent is
    delete ui;
}

void MainWindow::setupStatusLabel()
{
    // Creëer het status label (alternatief: definieer het in mainwindow.ui)
    m_statusLabel = new QLabel("Initialiseren...", this);
    ui->statusbar->addWidget(m_statusLabel); // Voeg toe aan de statusbalk
}

void MainWindow::setupUiConnections()
{
    // Verbind de scan knop
    connect(ui->scanButton, &QPushButton::clicked, this, &MainWindow::on_scanButton_clicked);

    // Verbind signalen van de BluetoothScanner met slots in MainWindow
    connect(m_scanner, &BluetoothScanner::deviceDiscovered, this, &MainWindow::addDeviceToList);
    connect(m_scanner, &BluetoothScanner::scanFinished, this, &MainWindow::onScanFinished);
    connect(m_scanner, &BluetoothScanner::scanError, this, &MainWindow::onScanError);
    connect(m_scanner, &BluetoothScanner::statusChanged, this, &MainWindow::updateStatusLabel);
    connect(m_scanner, &BluetoothScanner::statusChanged, this, &MainWindow::updateStatusLabel);
    connect(m_scanner, &BluetoothScanner::scanningChanged, this, &MainWindow::updateScanButtonState);
    connect(m_scanner, &BluetoothScanner::scanningUnavailable, this, &MainWindow::disableScanButton);
}

// Functie om permissies te checken en aan te vragen (kan als private member in MainWindow)
bool MainWindow::checkAndRequestBluetoothPermissions()
{
#if defined(Q_OS_ANDROID) // Alleen uitvoeren op Android
    QList<QPermission> permissionsToRequest;

    // --- Check Android Versie ---
    const QOperatingSystemVersion currentVersion = QOperatingSystemVersion::current();

    if (currentVersion >= QOperatingSystemVersion::Android12) {
        // Android 12 (API 31) en hoger: BLUETOOTH_SCAN nodig
        QBluetoothPermission scanPermission;
        scanPermission.setCommunicationModes(QBluetoothPermission::Access); // Scannen valt onder Access
        if (QCoreApplication::instance()->checkPermission(scanPermission) != Qt::PermissionStatus::Granted) {
            permissionsToRequest << scanPermission;
        }

        // Optioneel, als je ook gaat verbinden: BLUETOOTH_CONNECT
        // QBluetoothPermission connectPermission;
        // connectPermission.setCommunicationModes(QBluetoothPermission::Connection);
        // if (QCoreApplication::checkPermission(connectPermission) != Qt::PermissionStatus::Granted) {
        //     permissionsToRequest << connectPermission;
        // }

    } else {
        // Android 6 (API 23) t/m Android 11 (API 30): Locatie nodig voor scannen
        // FINE is meestal nodig voor BLE scanning
        QLocationPermission preciselocationPermission = QLocationPermission();
        preciselocationPermission.setAccuracy(QLocationPermission::Precise);
        if (QCoreApplication::instance()->checkPermission(preciselocationPermission) != Qt::PermissionStatus::Granted) {
            permissionsToRequest << preciselocationPermission;
        }
        // Eventueel ook COARSE checken/toevoegen als fallback of vereiste
        // QLocationPermission coarseLocationPermission = QLocationPermission();
        // preciselocationPermission.setAccuracy(QLocationPermission::Approximate);
        // if (QCoreApplication::checkPermission(coarseLocationPermission) != Qt::PermissionStatus::Granted) {
        //    permissionsToRequest << coarseLocationPermission;
        // }
    }

    // --- Permissies aanvragen indien nodig ---
    if (!permissionsToRequest.isEmpty()) {
        qDebug() << "Aanvragen van missende permissies:" << permissionsToRequest;
        updateStatusLabel("Wachten op permissies..."); // Feedback aan gebruiker

        // Asynchroon permissies aanvragen
        foreach (QPermission permissionToRequest, permissionsToRequest) {
            QCoreApplication::instance()->requestPermission(permissionToRequest, this, [this](const QPermission &resultPermission)
            {
                if (resultPermission.status() == Qt::PermissionStatus::Granted) {
                    qDebug() << "Alle benodigde permissies verkregen.";
                    updateStatusLabel("Permissies OK. Starten scan...");
                    // Start de scan NU pas, nadat permissies zijn verleend
                    ui->fireplaceListWidget->clear();
                    m_scanner->startScan();
                } else {
                    qDebug() << "Niet alle permissies zijn verleend door de gebruiker.";
                    updateStatusLabel("Fout: Benodigde permissies niet verleend.");
                    // Scan kan niet starten, knop mogelijk weer inschakelen?
                    updateScanButtonState(false); // Schakel knop weer in, scan mislukt
                }

            });
        }

        return false; // Geef aan dat we wachten op asynchroon resultaat, start scan nog niet direct
    } else {
        qDebug() << "Alle benodigde permissies zijn al aanwezig.";
        return true; // Alle permissies zijn al OK, scan kan direct starten
    }

#else // Niet op Android
    return true; // Geen runtime permissies nodig op andere platformen
#endif
}

void MainWindow::on_scanButton_clicked()
{
    qDebug() << "Scan knop geklikt - Start Bluetooth scan via BluetoothScanner";

    if (checkAndRequestBluetoothPermissions()) {
        // Als alle permissies al OK waren (synchrone pad)
        qDebug() << "Permissies waren al OK, directe start van scan.";
        ui->fireplaceListWidget->clear(); // Maak lijst leeg voor nieuwe resultaten
        m_scanner->startScan(); // Start de scan via de scanner klasse
    } else {
        // Permissies worden asynchroon aangevraagd.
        // De scan wordt gestart in de lambda callback van checkAndRequestBluetoothPermissions
        // als de gebruiker toestemming geeft.
        qDebug() << "Wachten op resultaat van permissieaanvraag...";
        // De scan knop wordt hier meestal al uitgeschakeld door updateScanButtonState(true)
        // vanuit de startScan aanroep in de lambda, of je kunt het hier expliciet doen.
        // updateScanButtonState(true); // Knop uitschakelen tijdens wachten
    }
}

void MainWindow::addDeviceToList(const QBluetoothDeviceInfo &deviceInfo)
{
    // Maak de tekst voor het lijstitem duidelijker
    QString itemName = deviceInfo.name();
    if (itemName.trimmed().isEmpty()) {
        itemName = "[Naamloos apparaat]";
    }
    QString itemText = QString("%1 (%2)").arg(itemName, deviceInfo.address().toString());

    QListWidgetItem *item = new QListWidgetItem(itemText, ui->fireplaceListWidget);

    // Voeg tooltip toe voor toegankelijkheid en extra info
    QString toolTip = QString("Naam: %1\nAdres: %2\nType: %3")
                          .arg(itemName,
                               deviceInfo.address().toString(),
                               (deviceInfo.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) ? "BLE" : "Classic");
    item->setToolTip(toolTip);

    // Optioneel: bewaar de deviceInfo of adres in het item voor latere interactie
    item->setData(Qt::UserRole, QVariant::fromValue(deviceInfo.address()));

    ui->fireplaceListWidget->addItem(item);
}

void MainWindow::onScanFinished()
{
    // Scan is klaar (normaal of zonder resultaten)
    // De statusbalk wordt bijgewerkt via het statusChanged signaal
    if (ui->fireplaceListWidget->count() == 0) {
         updateStatusLabel("Scan voltooid. Geen haarden gevonden.");
    } else {
         updateStatusLabel(QString("Scan voltooid. %1 haard(en) gevonden.").arg(ui->fireplaceListWidget->count()));
    }
     // De knop wordt weer ingeschakeld via het scanningChanged signaal
}

void MainWindow::onScanError(const QString &errorString)
{
    // Er is een fout opgetreden tijdens het scannen
    // Statusbalk en knop worden al bijgewerkt via statusChanged/scanningChanged signalen
    qDebug() << "Scan Fout ontvangen in MainWindow:" << errorString;
    // Hier zou je eventueel een dialoogvenster kunnen tonen
}

void MainWindow::updateStatusLabel(const QString &statusMessage)
{
    if (m_statusLabel) {
        m_statusLabel->setText(statusMessage);
    }
}

void MainWindow::updateScanButtonState(bool isScanning)
{
     ui->scanButton->setEnabled(!isScanning); // Schakel knop uit tijdens scan
}

void MainWindow::disableScanButton()
{
    ui->scanButton->setEnabled(false);
}
