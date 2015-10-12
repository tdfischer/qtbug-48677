#include <QCoreApplication>
#include <iostream>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QDebug>
#include <memory>

#include <unistd.h>

class SerialApp : public QCoreApplication
{
Q_OBJECT
public:
  SerialApp(int &argc, char** argv)
    : QCoreApplication(argc, argv) 
  {
    QTimer::singleShot(0, this, &SerialApp::setupSerial);
    std::cout << "Init..." << std::endl;
  }

private slots:
  void setupSerial() {
    std::cout << "Setting up serial..." << std::endl;

    m_ctlPort.reset(new QSerialPort("\\\\.\\COM7"));

    QObject::connect(m_ctlPort.get(), &QSerialPort::readyRead, this, &SerialApp::doCtlRead);

    m_ctlPort->setBaudRate(QSerialPort::Baud115200);
    m_ctlPort->setDataBits(QSerialPort::Data8);
    m_ctlPort->setParity(QSerialPort::NoParity);
    m_ctlPort->setStopBits(QSerialPort::OneStop);

    bool opened = m_ctlPort->open(QIODevice::ReadWrite);

    if (!opened) {
      std::cout << "Could not open port: " << m_ctlPort->errorString().toStdString() << std::endl;
      quit();
    }

    m_ctlPort->setDataTerminalReady(true);
    m_ctlPort->setRequestToSend(true);

    QTimer::singleShot(0, this, &SerialApp::poke);
  }

  void poke() {
    std::cout << "poking" << std::endl;
    m_ctlPort->write("{sr:n}\n");
  }

  void doCtlRead() {
    QByteArray readBytes(m_ctlPort->readAll());
    std::cout << "read:" << readBytes.toStdString() << std::endl;
    sleep(1);
    m_readBuf += readBytes;
    QStringList tokens = m_readBuf.split("\n");
    if (tokens.length() > 1) {
      m_readBuf = tokens[1];
      QTimer::singleShot(500, this, &SerialApp::poke);
      std::cout << "Read full:" << tokens[0].toStdString() << std::endl;
    }
  }

private:
  std::unique_ptr<QSerialPort> m_ctlPort;
  QString m_readBuf;
};

int main(int argc, char** argv)
{
  SerialApp app(argc, argv);

  std::cout << "Start!" << std::endl;
  return app.exec();
}

#include "moc_main.cpp"
