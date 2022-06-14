import os

from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QAction, QPushButton, QScrollArea, QWidget, QVBoxLayout, \
    QCheckBox, QComboBox
import sys
import serial
import time
import matplotlib.pyplot as plt
import csv
import pickle
import winsound
from PyQt5.QtCore import Qt, QSize
import glob


class ReadLine:
    def __init__(self, s):
        self.buf = bytearray()
        self.s = s

    def readline(self):
        i = self.buf.find(b"\n")
        if i >= 0:
            r = self.buf[:i + 1]
            self.buf = self.buf[i + 1:]
            return r
        while True:
            i = max(1, min(2048, self.s.in_waiting))
            data = self.s.read(i)
            i = data.find(b"\n")
            if i >= 0:
                r = self.buf + data[:i + 1]
                self.buf[0:] = data[i + 1:]
                return r
            else:
                self.buf.extend(data)


class MyWindow(QMainWindow):
    def __init__(self):
        super(MyWindow,self).__init__()

        self.candidate_no = 49

        self.base_paths = []
        for filename in glob.iglob("./src/data_collection/data" + '**/**/*_hand', recursive=True):
            self.base_paths.append(filename)
        self.base_path = self.base_paths[0]

        self.initUI()
        self.data = []
        self.chosen_hand = "right_hand"
        self.count = 0
        self.accept_data = True

    def button_clicked(self):
        index = self.sender().text()
        print(f"a button was clicked: {index}")

        plt.plot(self.unpickled[int(index)])
        plt.xlabel('Time')
        plt.ylabel('Potentiometer Reading')
        plt.title('Potentiometer Reading vs. Time')
        plt.show()

    def update_file(self):
        print("UPDATING FILE")
        with open(self.path, "wb") as file:
            for data, checkbox in zip(self.unpickled, self.checkboxes):
                if checkbox.isChecked():
                    pickle.dump(data, file)

    def move_file(self):
        print("MOVING FILE")
        with open(self.path, "wb") as file:
            for data, checkbox in zip(self.unpickled, self.checkboxes):
                if checkbox.isChecked():
                    pickle.dump(data, file)

        with open(self.path_move, "ab") as file:
            print("MOVING TO: ", self.path_move)
            for data, checkbox in zip(self.unpickled, self.checkboxes):
                if not checkbox.isChecked():
                    pickle.dump(data, file)

        print("FILE UPDATED")

    def selectionchange(self, i):
        self.base_path = self.base_paths[i]
        print(self.base_paths[i])
        print(self.base_path)
        self.clearUI()

    def clearUI(self):
        for i in reversed(range(self.vbox.count())):
            self.vbox.itemAt(i).widget().setParent(None)

        print("cleared UI")
        self.addUI()

    def checkProblematicFiles(self):
        print("REVIEWING ALL PATHS")
        for base_path in self.base_paths:
            print(base_path)
            path_checking = f"{base_path}/candidate_{self.candidate_no}.pickle"
            with open(path_checking, 'rb') as f:
                unpickled = []
                while True:
                    try:
                        unpickled.append(pickle.load(f))
                    except EOFError:
                        break
            file_len = len(unpickled)
            if file_len != 5:
                print(f"PROBLEMATIC FILE: {file_len}: {path_checking}")

        print("DONE REVIEWING PATHS")

    def addUI(self):
        print("adding UI")

        self.path = f"{self.base_path}/candidate_{self.candidate_no}.pickle"
        # self.path_move = f"{self.base_path}/candidate_00.pickle"
        self.base_path_move = f"./src/data_collection\data/clockwise/left_hand"
        self.path_move = f"{self.base_path}/candidate_{self.candidate_no}.pickle"
        # self.path_move = f"{self.base_path_move}/candidate_{self.candidate_no + 1}.pickle"


        print(self.path)
        self.vbox.addWidget(self.cb)
        self.vbox.addWidget(self.b)
        self.vbox.addWidget(self.b2)
        self.vbox.addWidget(self.b3)
        self.buttons = []
        self.checkboxes = []
        index = 0
        print("added buttons")

        # with open(self.path_move, 'ab+') as f:
        #     pass
        with open(self.path, 'rb') as f:
            self.unpickled = []
            while True:
                try:
                    self.unpickled.append(pickle.load(f))
                    button = QPushButton(self)
                    button.setText(f"{index}")
                    button.clicked.connect(self.button_clicked)
                    # grid.addWidget(self.swipe_right_button)
                    self.buttons.append(button)

                    checkbox = QCheckBox(f"{index}")
                    checkbox.setChecked(True)
                    self.checkboxes.append(checkbox)

                    index += 1
                except EOFError:
                    break

        print(len(self.unpickled))

        for button, checkbox in zip(self.buttons, self.checkboxes):
            self.vbox.addWidget(button)
            self.vbox.addWidget(checkbox, 1)
            # self.vbox.addWidget(button, stretch=10, alignment=Qt.AlignLeft)
            # self.vbox.addWidget(checkbox, alignment=Qt.AlignRight)

    def initUI(self):
        quit = QAction("Quit", self)
        quit.triggered.connect(self.closeEvent)

        print("here")
        # for filename in glob.iglob("./src/data_collection/data" + '**/**/**.pickle', recursive=True):
        #     print(filename)

        self.cb = QComboBox()
        self.cb.addItems(self.base_paths)
        self.cb.currentIndexChanged.connect(self.selectionchange)




        w = QtWidgets.QWidget()
        self.scroll = QScrollArea()  # Scroll Area which contains the widgets, set as the centralWidget
        self.widget = QWidget()  # Widget that contains the collection of Vertical Box
        self.vbox = QVBoxLayout()
        # self.setCentralWidget(w)
        # grid = QtWidgets.QGridLayout(w)




        self.b = QPushButton(self)
        self.b.setText("UPDATE FILE")
        self.b.clicked.connect(self.update_file)


        self.b2 = QPushButton(self)
        self.b2.setText("MOVE FILE")
        self.b2.clicked.connect(self.move_file)

        self.b3 = QPushButton(self)
        self.b3.setText("Find Problematic Files")
        self.b3.clicked.connect(self.checkProblematicFiles)


        self.widget.setLayout(self.vbox)

        # Scroll Area Properties
        self.scroll.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scroll.setWidgetResizable(True)
        self.scroll.setWidget(self.widget)

        self.setCentralWidget(self.scroll)

        self.setGeometry(100, 100, 1000, 900)
        self.setWindowTitle('Scroll Area Demonstration')
        self.show()

        self.addUI()

        return

    def closeEvent(self, event):
        event.accept()








def window():
    app = QApplication(sys.argv)
    win = MyWindow()
    win.show()
    sys.exit(app.exec_())

window()