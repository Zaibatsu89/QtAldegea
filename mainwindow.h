#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem> // Nodig voor items
#include <QBluetoothDeviceInfo> // Nodig voor apparaatinformatie

// Forward declaraties
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class BluetoothScanner; // Forward declaratie van onze scanner klasse
class QLabel; // Forward declaratie voor status label

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slot voor het controleren van permissies
    bool checkAndRequestBluetoothPermissions();

    // Slot voor de scan knop
    void on_scanButton_clicked();

    // Slots om signalen van BluetoothScanner te ontvangen
    void addDeviceToList(const QBluetoothDeviceInfo &deviceInfo);
    void onScanFinished();
    void onScanError(const QString &errorString);
    void updateStatusLabel(const QString &statusMessage);
    void updateScanButtonState(bool isScanning); // Om knop in/uit te schakelen

private:
    Ui::MainWindow *ui;
    BluetoothScanner *m_scanner = nullptr; // Pointer naar onze scanner logica
    QLabel *m_statusLabel = nullptr; // Pointer naar het status label in de UI

    void setupUiConnections(); // Helper functie voor connecties
    void setupStatusLabel();   // Helper functie om status label te initialiseren
    void disableScanButton();
};
#endif // MAINWINDOW_H
