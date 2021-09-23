from road_traffic_controller_design import *
from PyQt5.QtWidgets import QApplication, QMainWindow, QButtonGroup
import sys
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
from PyQt5.QtGui import QIcon
import R_T_Controller_Volume_Settings as RTC_volume
# import fix_qt_import_error


class Road_Controller(Ui_MainWindow, QMainWindow):

    portList = []
    portListDescription = ['Выберите устройство']

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.comboBox.setToolTip("Список СОМ-портов")
        self.refreshCOMbutton.setToolTip("Обновить список СОМ-портов")
        self.connectButton.setToolTip("Подключиться к роботу")
        self.ejectButton.setToolTip("Отключить соединение")
        self.start_btn.setToolTip("Выполнить движение")
        self.answer_btn.setToolTip("Показать светодиодную разметку")
        self.finish_btn.setToolTip("Вернуться в исходное положение")
        # self.exit_action.triggered.connect(exit)
        self.settings_action.triggered.connect(self.volume_settings)

        self.start_btn.clicked.connect(self.start_moving)
        self.answer_btn.clicked.connect(self.show_answer)
        self.finish_btn.clicked.connect(self.finish_move)

        # Position buttons group
        self.pos_button_group = QButtonGroup()
        self.pos_button_group.addButton(self.posit_1_btn)
        self.pos_button_group.addButton(self.posit_2_btn)
        self.pos_button_group.addButton(self.posit_3_btn)
        self.pos_button_group.addButton(self.posit_4_btn)
        # self.pos_button_group.buttonClicked.connect(self.position_radiobtn_func)

        # Hand buttons group
        self.hand_button_group = QButtonGroup()
        self.hand_button_group.addButton(self.hand_down_btn)
        self.hand_button_group.addButton(self.hand_front_btn)
        self.hand_button_group.addButton(self.hand_up_btn)

        # Serial connections
        self.serial_init()
        self.serial.readyRead.connect(self.on_read)
        self.refreshCOMbutton.clicked.connect(self.refresh_COM)
        self.connectButton.clicked.connect(self.on_open)
        self.ejectButton.clicked.connect(self.on_close)

    volume = 9
    def volume_settings(self):
        volume_window = RtcVolume(self)
        volume_window.spinBox.setValue(self.volume)
        # print()
        if volume_window.exec_():
            self.volume = volume_window.spinBox.value()
            print(self.volume)
            self.serial_send('4'+str(self.volume))


    def on_read(self):
        rx = self.serial.readLine()
        rxs = str(rx, 'utf-8').strip()
        data = rxs.split(' ')
        print(data)

    def serial_init(self):
        # open the serial port
        self.serial = QSerialPort(self)
        self.serial.setBaudRate(115200)

        ports = QSerialPortInfo().availablePorts()
        for port in ports:
            self.portList.append(port.portName())
            self.portListDescription.append(port.description())
        print(self.portList)
        print(self.portListDescription)
        self.comboBox.addItems(self.portList)

    def on_close(self):
        self.serial.close()
        self.connectLabel.setText('Нет подключения')

    def on_open(self):
        self.serial.setPortName(self.comboBox.currentText())
        answer = self.serial.open(QIODevice.ReadWrite)
        print('connected to', self.comboBox.currentText())
        print('answer =', answer)
        if answer:
            self.connectLabel.setText('Подключено')

    def refresh_COM(self):
        ports = QSerialPortInfo().availablePorts()
        # ports.clear()
        self.portList.clear()
        self.portListDescription.clear()
        for port in ports:
            self.portList.append(port.portName())
            self.portListDescription.append(port.description())
        print(self.portList)
        print(self.portListDescription)
        self.comboBox.addItems(self.portListDescription)
        self.comboBox.clear()
        self.comboBox.addItems(self.portList)

    def serial_send(self, data):
        txs = ''
        for val in data:
            txs += str(val)
            txs += ','
        txs = txs[:-1]
        txs += ';'
        self.serial.write(txs.encode())
        self.serial.waitForBytesWritten(10)
        print("serial send", txs)

    def start_moving(self):
        # pos =
        pos = - (self.pos_button_group.checkedId() + 1)
        hand = - (self.hand_button_group.checkedId() + 1)

        print(pos, hand)
        # print(self.hand_button_group.checkedId())
        self.serial_send('1'+str(pos)+str(hand))

    def show_answer(self):
        pos = - (self.pos_button_group.checkedId() + 1)
        hand = - (self.hand_button_group.checkedId() + 1)

        self.serial_send('2'+str(pos)+str(hand))

    def finish_move(self):
        self.serial_send('310')

    def position_radiobtn_func(self, btn):
        # print(btn, btn.text)
        # self.position = btn
        # print(self.pos_button_group.checkedId())
        pass


class RtcVolume(RTC_volume.Ui_Dialog, QtWidgets.QDialog):
    def __init__(self, parent=None):
        super().__init__()
        self.setupUi(self)
        self.setMaximumWidth(self.width())
        self.setMaximumHeight(self.height())
        self.setMinimumWidth(self.width())
        self.setMinimumHeight(self.height())


def main():
    app = QApplication(sys.argv)  # Новый экземпляр QApplication
    app.setStyle('Fusion')
    window = Road_Controller()  # Создаём объект класса ExampleApp
    window.setWindowTitle("Регулировщик")
    window.setMaximumWidth(window.width())
    window.setMaximumHeight(window.height())
    window.setMinimumWidth(window.width())
    window.setMinimumHeight(window.height())
    # window.setWindowIcon(QIcon('src/zarnitza64g.ico'))
    window.show()  # Показываем окно
    app.exec_()  # и запускаем приложение


if __name__ == '__main__':
    main()
