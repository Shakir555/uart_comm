#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QIcon>
#include <QFont>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>

void qt_window(QWidget& wd)
{
    wd.setFixedSize(500, 550);
    wd.setWindowIcon(QIcon("/home/shakir-salam/Documents/proj/emp_ms/icon.png"));
    wd.setWindowTitle("Serial Port");

    QVBoxLayout* layout = new QVBoxLayout(&wd);

    QLabel* portLabel = new QLabel("Select Serial Port:", &wd);
    QComboBox* portSelector = new QComboBox(&wd);
    QPushButton* connectButton = new QPushButton("Connect", &wd);
    QTextEdit* textEdit = new QTextEdit(&wd);
    QPushButton* sendButton = new QPushButton("Send Data", &wd);
    QTextEdit* receivedDataEdit = new QTextEdit(&wd);

    layout->addWidget(portLabel);
    layout->addWidget(portSelector);
    layout->addWidget(connectButton);
    layout->addWidget(textEdit);
    layout->addWidget(sendButton);
    layout->addWidget(receivedDataEdit);

    // Create serial port object
    QSerialPort* serialPort = new QSerialPort(&wd);

    // Populate available ports
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        QString portDescription = QString("%1 (%2)").arg(info.portName(), info.description());
        portSelector->addItem(portDescription, info.portName());
        qDebug() << "Port:" << info.portName()
                 << " Description:" << info.description()
                 << " Manufacturer:" << info.manufacturer();
    }

    // Connect to selected serial port on button click
    QObject::connect(connectButton, &QPushButton::clicked, [=]() mutable {
        if (serialPort->isOpen())
            serialPort->close(); // Close if already open

        QString selectedPortName = portSelector->currentData().toString();
        if (selectedPortName.isEmpty()) {
            qDebug() << "No port selected!";
            return;
        }

        serialPort->setPortName(selectedPortName);
        serialPort->setBaudRate(QSerialPort::Baud115200);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (serialPort->open(QIODevice::ReadWrite)) {
            qDebug() << "Serial port opened: " << selectedPortName;
        } else {
            qDebug() << "Failed to open serial port!";
        }
    });

    // Send button handler
    QObject::connect(sendButton, &QPushButton::clicked, [=]() {
        if (serialPort->isOpen()) {
            QString data = textEdit->toPlainText();
            if (!data.isEmpty()) {
                qint64 bytesWritten = serialPort->write(data.toUtf8());
                if (bytesWritten == -1) {
                    qDebug() << "Error writing to serial port!";
                } else {
                    qDebug() << "Sent data: " << data;
                }
            } else {
                qDebug() << "No data to send!";
            }
        } else {
            qDebug() << "Serial port is not open!";
        }
    });

    // Handle incoming data
    QObject::connect(serialPort, &QSerialPort::readyRead, [=]() {
        QByteArray receivedData = serialPort->readAll();
        if (!receivedData.isEmpty()) {
            QString receivedString = QString::fromUtf8(receivedData);
            qDebug() << "Received data: " << receivedString;
            receivedDataEdit->append(receivedString);
        } else {
            qDebug() << "ReadyRead signal received, but no data.";
        }
    });
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QWidget wd;
    qt_window(wd);
    wd.show();
    return application.exec();
}
