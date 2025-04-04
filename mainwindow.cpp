#include "mainwindow.h"
#include "ui_mainwindow.h" // Correct pad gegenereerd door Qt/CMake
#include "bluetoothscanner.h" // Include onze scanner klasse

#include <QDebug>
#include <QLabel> // Nodig voor status label

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
    connect(m_scanner, &BluetoothScanner::deviceFound, this, &MainWindow::addDeviceToList);
    connect(m_scanner, &BluetoothScanner::scanFinished, this, &MainWindow::onScanFinished);
    connect(m_scanner, &BluetoothScanner::scanError, this, &MainWindow::onScanError);
    connect(m_scanner, &BluetoothScanner::statusChanged, this, &MainWindow::updateStatusLabel);
    connect(m_scanner, &BluetoothScanner::statusChanged, this, &MainWindow::updateStatusLabel);
    connect(m_scanner, &BluetoothScanner::scanningChanged, this, &MainWindow::updateScanButtonState);
    connect(m_scanner, &BluetoothScanner::scanningUnavailable, this, &MainWindow::disableScanButton);
}

void MainWindow::on_scanButton_clicked()
{
    qDebug() << "Scan knop geklikt - Start Bluetooth scan via BluetoothScanner";
    ui->fireplaceListWidget->clear(); // Maak lijst leeg voor nieuwe resultaten
    m_scanner->startScan(); // Start de scan via de scanner klasse
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