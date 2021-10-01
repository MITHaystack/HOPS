
import sys

from PyQt5.QtCore import QSize, Qt, QLine, QPoint
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QMainWindow, QCheckBox, QFrame, QSizePolicy
from PyQt5.QtWidgets import QVBoxLayout, QHBoxLayout, QGridLayout, QTabWidget, QLabel, QLineEdit
from PyQt5.QtGui import QPalette, QColor, QPainter

from parse_alist import ParseAlist

import numpy as np

# parse alist and get baselines, sources, stations, scans
# build new window with checkboxes for all of these (unique)
# option to select all, remove autocorr, etc
# also have snrmin and snrmax options
# different tab: select plot parameters
# collect checkboxes and plot

# picking: open new window with fourfit parameters
# option to show fourfit plot or rerun fourfit for this scan/baseline?

# build plotting options on case
# labels, units, range depending on data type




class SelectionCheckboxGrid(QWidget):

    def __init__(self, checkbox_items, grid_name, orient='col', autocorr=False):
        super().__init__()
        
        self.checkbox_items = checkbox_items
        #self.grid_name = grid_name
        self.num_boxes = len(checkbox_items)
        self.checkboxes = []

        # if we are dealing with baselines, we want to handle the autocorrs
        # with some different methods
        if autocorr:
            first_station = [x[0] for x in self.checkbox_items]
            second_station = [x[1] for x in self.checkbox_items]
            self.auto_idx = []
            for ii in range(self.num_boxes):
                if first_station[ii] == second_station[ii]:
                    self.auto_idx.append(int(ii))

            
        # build a grid of checkboxes

        if self.num_boxes<6:
            cols = 1
        else:
            cols = 6
        rem = self.num_boxes % cols
        if rem==0:
            rows = self.num_boxes // cols
        else:
            rows = self.num_boxes // cols + 1

        self.checkbox_grid = QGridLayout()

        # swap orientation of columns & rows if requested
        if orient=='col':
            positions = [(i, j) for i in range(rows) for j in range(cols)]
        elif orient=='row':
            positions = [(i, j) for i in range(cols) for j in range(rows)]
        else:
            positions = [(i, j) for i in range(rows) for j in range(cols)]
            
        for position, box_name in zip(positions, self.checkbox_items):
            chkbox = QCheckBox(box_name, self)
            chkbox.setCheckable(True)
            self.checkbox_grid.addWidget(chkbox, *position)
            self.checkboxes.append(chkbox)
        
        self.grid_label = QLabel(grid_name)
        self.label_hbox = QHBoxLayout()
        self.label_hbox.addWidget(self.grid_label)
        self.label_hbox.addStretch(1)

        self.separator = QFrame()
        self.separator.setFrameShape(QFrame.HLine)
        self.separator.setLineWidth(1)

        self.selectAllButton = QPushButton("Select All")
        self.selectAllButton.clicked.connect(self.selectAll)
        
        self.deselectAllButton = QPushButton("Deselect All")
        self.deselectAllButton.clicked.connect(self.deselectAll)

        self.button_hbox = QHBoxLayout()
        self.button_hbox.addWidget(self.selectAllButton)
        self.button_hbox.addWidget(self.deselectAllButton)
        

        if autocorr:
            self.selectAutosButton = QPushButton("Select AutoCorrs")
            self.selectAutosButton.clicked.connect(self.selectAutos)
        
            self.deselectAutosButton = QPushButton("Deselect AutoCorrs")
            self.deselectAutosButton.clicked.connect(self.deselectAutos)

            self.button_hbox.addWidget(self.selectAutosButton)
            self.button_hbox.addWidget(self.deselectAutosButton)
            
            
        self.button_hbox.addStretch(1)    

        
    def selectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(True)

            
    def deselectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(False)


    def selectAutos(self):
        for ii in self.auto_idx:
            self.checkboxes[ii].setChecked(True)

    def deselectAutos(self):
        for ii in self.auto_idx:
            self.checkboxes[ii].setChecked(False)



class DataSelectWindow(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data):
        super().__init__()

        self.alist_data = alist_data
        
        # these need to be public variables (self) so they are callable!
        self.baseline_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.baselines), 'Baselines', autocorr=True)

        self.source_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.sources), 'Sources')

        self.scan_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.scans), 'Scans')

        self.pol_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.pols), 'Polarizations')



        # the stretches here do the vertical centering
        # is there a better way to do this with a groupBox?
        select_vbox = QVBoxLayout()
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.baseline_checkboxes.label_hbox)
        select_vbox.addWidget(self.baseline_checkboxes.separator)
        select_vbox.addLayout(self.baseline_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.baseline_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.scan_checkboxes.label_hbox)
        select_vbox.addWidget(self.scan_checkboxes.separator)
        select_vbox.addLayout(self.scan_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.scan_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.source_checkboxes.label_hbox)
        select_vbox.addWidget(self.source_checkboxes.separator)
        select_vbox.addLayout(self.source_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.source_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.pol_checkboxes.label_hbox)
        select_vbox.addWidget(self.pol_checkboxes.separator)
        select_vbox.addLayout(self.pol_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.pol_checkboxes.button_hbox)
        select_vbox.addStretch(1)


        # add a plot button, connect to a plotting function

        
        
        self.setLayout(select_vbox)



        
        # golden aspect ratio
        wsize = 800
        #self.setFixedSize(QSize(wsize,wsize*0.618))
        self.setMinimumSize(QSize(wsize,wsize*1.2))
        self.setWindowTitle('Alist Data Selection')






### class for initialization tab
### gets alist filename and spawns data selection window
class Init_tab(QWidget):

    def __init__(self, alist_file=None):
        super().__init__()

        # selection window initialization
        self.w = None

        self.alist_file = alist_file
        
        self.initUI()

        self.show()

        
    def initUI(self):

        # Create a text entry box to load the alist
        init_box_label = QLabel('Alist file')
        self.init_afile_box = QLineEdit()
        if self.alist_file is not None:
            self.init_afile_box.setText(self.alist_file)
        #else:
        #    self.init_afile_box.setText()
        self.init_afile_box.setFixedWidth(300)

        # button to load the file; this will run a mess of python parsing code
        self.init_button = QPushButton("Load Afile")
        self.init_button.clicked.connect(self.load_button)
        
        # set up the tab: vertically center an HBox within a VBox
        init_hbox = QHBoxLayout()
        init_hbox.addWidget(init_box_label)
        init_hbox.addWidget(self.init_afile_box)
        init_hbox.addStretch(1)
        init_hbox.addWidget(self.init_button)

        # the stretches here do the vertical centering
        init_vbox = QVBoxLayout()
        init_vbox.addStretch(1)
        init_vbox.addLayout(init_hbox)
        init_vbox.addStretch(1)
        
        self.setLayout(init_vbox)        


    def load_button(self, checked):

        # load the alist file, collect data, open new window with data selection options

        afilename = self.init_afile_box.text()
        
        alist_data = ParseAlist(afilename)

        if self.w is None:
            self.w = DataSelectWindow(alist_data)
        self.w.show()

        



        

class MainWindow(QMainWindow):
    def __init__(self, **kwargs):
        super().__init__()

        if 'alist_file' in kwargs:
            self.alist_file = kwargs['alist_file']
        else:
            self.alist_file = None
            
            
        self.setWindowTitle("HOPS4 Aedit Test GUI")

        # Create a tabs widget in this window
        self.tabs = QTabWidget()
        self.tabs.setTabPosition(QTabWidget.North)
        #self.tabs.setMovable(True)

        self.tab1 = Init_tab(self.alist_file)
        self.tab2 = QWidget()
        self.tab3 = QWidget()
        self.tab4 = QWidget()
        
        # Add tabs, specifying the widget in each tab
        self.tabs.addTab(self.tab1, 'Init')
        self.tabs.addTab(self.tab2, 'Select')
        self.tabs.addTab(self.tab3, 'Plot')
        self.tabs.addTab(self.tab4, 'Save')
        
        self.setCentralWidget(self.tabs)
        
        # golden aspect ratio
        wsize = 800
        #self.setFixedSize(QSize(wsize,wsize*0.618))
        self.setMinimumSize(QSize(wsize,wsize*0.618))


        
    
if __name__ == '__main__':
    app = QApplication(sys.argv)

    # assume command line argument is an alist filename
    if len(sys.argv)>1:
        window = MainWindow(alist_file=sys.argv[1])
    else:
        window = MainWindow()
    
    window.show()

    app.exec()
