#!/usr/bin/env python

from __future__ import print_function
import sys

from PyQt5.QtCore import QSize, Qt, QLine, QPoint
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QMainWindow, QCheckBox, QFrame#, QSizePolicy
from PyQt5.QtWidgets import QVBoxLayout, QHBoxLayout, QGridLayout, QTabWidget, QLabel, QLineEdit, QComboBox, QGroupBox, QPlainTextEdit, QTextEdit
#from PyQt5.QtGui import QPalette, QColor, QPainter, QWindow

import parse_alist as pa

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar

import matplotlib.pyplot
from matplotlib.lines import Line2D

import numpy as np

# Main Window: load alist file(s) and datadir
# Data Window: tabs

# selection tab selects baselines, sources, etc - summary button?
# plot tab has plotting parameters, plot button -> picking, rerun fourfit with new parameters, cycle through plots
# calc tab does closure -> list available triangles? (from stations from each scan?)

# scans go on a different tab - there are usually a lot of them

# parse alist and get baselines, sources, stations, scans
# build new window with checkboxes for all of these (unique)
# option to select all, remove autocorr, etc
# also have snrmin and snrmax options
# length, frequency

# how is frequency reported in the alist? does 'B32' mean band B, 32 channels?
#

# different tab: select plotting options (axes, limits)

# option to write new afile

# organize plots by baseline? in a list, with a key connection to scroll through them?

# picking: left click, open new window with fourfit plot
# right click, rerun fourfit, open control file and adjust parameters?

# option to show fourfit plot or rerun fourfit for this scan/baseline?

# build plotting options on case
# labels, units, range depending on data type


# this class is an example of a two-box grid
class SelectionParamBoxGrid(QWidget):

    def __init__(self, alist_data, orient='col'):
        super().__init__()

        self.snrmin = np.min(alist_data.records['snr'])
        self.snrmax = np.max(alist_data.records['snr'])
        
        # Create a text entry box
        snrmin_box_label = QLabel('Min SNR ')
        self.snrmin_box = QLineEdit()
        self.snrmin_box.setText(str(self.snrmin))
        self.snrmin_box.setFixedWidth(100)

        snrmax_box_label = QLabel('Max SNR')
        self.snrmax_box = QLineEdit()
        self.snrmax_box.setText(str(self.snrmax))
        self.snrmax_box.setFixedWidth(100)

        # Set up the parameters area
        self.param_label = QLabel('Parameters')
        self.params_hbox = QHBoxLayout()
        self.params_hbox.addWidget(self.param_label)
        self.params_hbox.addStretch(1)

        self.separator = QFrame()
        self.separator.setFrameShape(QFrame.HLine)
        self.separator.setLineWidth(1)

        self.textbox_gridv = QVBoxLayout()

        self.textbox_gridh1 = QHBoxLayout()
        self.textbox_gridh1.addWidget(snrmin_box_label)
        self.textbox_gridh1.addWidget(self.snrmin_box)
        self.textbox_gridh1.addStretch(1)
        
        self.textbox_gridh2 = QHBoxLayout()
        self.textbox_gridh2.addWidget(snrmax_box_label)
        self.textbox_gridh2.addWidget(self.snrmax_box)
        self.textbox_gridh2.addStretch(1)

        self.textbox_gridv.addLayout(self.textbox_gridh1)
        self.textbox_gridv.addLayout(self.textbox_gridh2)
        self.textbox_gridv.addStretch(1)

    def collectParamVals(self):

        snrrange = [self.snrmin_box.text(), self.snrmax_box.text()]






# For qcode, we want to list all of them, but grey out the ones that aren't present in the data
# also, provide the number of records with each qcode
class QCodeCheckboxGrid(QWidget):

    def __init__(self, qcode_list):
        super().__init__()

        self.existing_qcodes, self.qcode_counts = np.unique(qcode_list,return_counts=True)

        # not sure if A, N, ? are always used, or are redundant?
        self.checkbox_items = ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','N','?']
        self.num_boxes = len(self.checkbox_items)
        self.checkboxes = []

        self.checkbox_grid = QGridLayout()

        positions = [(i, j) for i in range(2) for j in range(10)]
        for position, box_name in zip(positions, self.checkbox_items):
            if box_name in self.existing_qcodes:
                idx = self.existing_qcodes.tolist().index(box_name)
                chkbox = QCheckBox(box_name+' ('+str(self.qcode_counts[idx])+')', self)
                chkbox.setCheckable(True)
                self.checkbox_grid.addWidget(chkbox, *position)
                self.checkboxes.append(chkbox)
            else:
                chkbox = QCheckBox(box_name+' (0)', self)
                chkbox.setCheckable(False)
                chkbox.setDisabled(True)
                self.checkbox_grid.addWidget(chkbox, *position)
                self.checkboxes.append(chkbox)
                
        
        self.grid_label = QLabel('Quality Codes ('+str(len(qcode_list))+' total records)')
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
        self.button_hbox.addStretch(1)
        
        self.selectAll()

        
    def selectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(True)

            
    def deselectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(False)


    def collectCheckedBoxes(self):

        checked_boxes = []        
        for checkbox in self.checkboxes:
            if checkbox.isChecked():
                checked_boxes.append(checkbox.text().split(' ')[0]) # need to strip away the # of records

        return checked_boxes





        
# this class is a generic way to build a grid of checkboxes with title and separator line
# need to add a method to get the checked boxes
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

        self.selectAll()
        if autocorr:
            self.deselectAutos()

        
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

    def collectCheckedBoxes(self):

        checked_boxes = []
        for checkbox in self.checkboxes:
            if checkbox.isChecked():
                checked_boxes.append(checkbox.text())

        return checked_boxes




    
# This class sets up the arrangement of checkboxes, textboxes, etc to select the data from the alist file
# It forms the Selection tab
class SelectionPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data):
        super().__init__()

        self.alist_data = alist_data
        
        # these need to be public variables (self) so they are callable!
        self.param_textbox = SelectionParamBoxGrid(alist_data)
        
        self.baseline_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['baseline']), 'Baselines', autocorr=True)

        self.station_checkboxes = SelectionCheckboxGrid(alist_data.unique_stations, 'Stations')

        #self.frequency_checkboxes = SelectionCheckboxGrid(alist_data.records['frequency_band'], 'Frequencies')
        
        self.qcode_checkboxes = QCodeCheckboxGrid(alist_data.records['qcode'])
        
        self.source_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['source']), 'Sources')

        self.pol_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['pols']), 'Polarizations')



        # the stretches here do the vertical centering
        # is there a better way to do this with a groupBox?
        select_vbox = QVBoxLayout()
        select_vbox.addLayout(self.param_textbox.params_hbox)
        select_vbox.addWidget(self.param_textbox.separator)
        select_vbox.addLayout(self.param_textbox.textbox_gridv)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.baseline_checkboxes.label_hbox)
        select_vbox.addWidget(self.baseline_checkboxes.separator)
        select_vbox.addLayout(self.baseline_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.baseline_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.station_checkboxes.label_hbox)
        select_vbox.addWidget(self.station_checkboxes.separator)
        select_vbox.addLayout(self.station_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.station_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        #select_vbox.addLayout(self.frequency_checkboxes.label_hbox)
        #select_vbox.addWidget(self.frequency_checkboxes.separator)
        #select_vbox.addLayout(self.frequency_checkboxes.checkbox_grid)
        #select_vbox.addLayout(self.frequency_checkboxes.button_hbox)
        #select_vbox.addStretch(1)
        select_vbox.addLayout(self.qcode_checkboxes.label_hbox)
        select_vbox.addWidget(self.qcode_checkboxes.separator)
        select_vbox.addLayout(self.qcode_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.qcode_checkboxes.button_hbox)
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


    def CollectDataSelections(self):

        data_selection_dict = {}
        
        data_selection_dict['baselines'] = self.baseline_checkboxes.collectCheckedBoxes()
        data_selection_dict['stations'] = self.station_checkboxes.collectCheckedBoxes()
        data_selection_dict['qcodes'] = self.qcode_checkboxes.collectCheckedBoxes()
        data_selection_dict['sources'] = self.source_checkboxes.collectCheckedBoxes()        
        data_selection_dict['pols'] = self.pol_checkboxes.collectCheckedBoxes()
        data_selection_dict['snrrange'] = self.param_textbox.collectParamVals()
        
        return data_selection_dict






# This class sets up the arrangement of checkboxes, textboxes, etc to select the data from the alist file
# It forms the Selection tab
class ScanPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data):
        super().__init__()

        self.alist_data = alist_data
        
        # these need to be public variables (self) so they are callable!
        self.scan_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['scan']), 'Scans')

        select_vbox = QVBoxLayout()
        select_vbox.addLayout(self.scan_checkboxes.label_hbox)
        select_vbox.addWidget(self.scan_checkboxes.separator)
        select_vbox.addLayout(self.scan_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.scan_checkboxes.button_hbox)
        select_vbox.addStretch(1)

        self.setLayout(select_vbox)


    def CollectScanSelections(self):

        scan_selection_dict = {}        
        scan_selection_dict['scans'] = self.scan_checkboxes.collectCheckedBoxes()
        return scan_selection_dict

    

# class to handle picked fplot
class PickWindow(QWidget):
    def __init__(self, filename, ind):
        super().__init__()

        n = len(ind)
        if not n:
            return
        
        self.pick_fig = matplotlib.figure.Figure()
        self.pick_canvas = FigureCanvas(self.pick_fig)
        self.pick_toolbar = NavigationToolbar(self.pick_canvas, self)

        layout = QVBoxLayout()
        layout.addWidget(self.pick_toolbar)
        layout.addWidget(self.pick_canvas)
        self.setLayout(layout)

        self.pick_fig.clf()
        axs = self.pick_fig.subplots(n, squeeze=False)

        for dataind, pax in zip(ind, axs.flat):
            
            # read in the kludged fourfit plot and plot it
            image = matplotlib.pyplot.imread(filename)        
            pax.imshow(image)
            pax.axis('off')
            pax.margins(0,0)
        
        self.pick_canvas.draw_idle()

        
    

# aedit plot window
class PlotWindow(QWidget):
    def __init__(self, alist_data, x_field, y_field, plot_format_dict):
        super().__init__()

        # Preliminaries
        self.alist_data = alist_data
        self.x_field = x_field
        self.y_field = y_field
        self.plot_format_dict = plot_format_dict
        self.w = None
        self.counter = 0

        self.figure = matplotlib.figure.Figure() # Initialize a figure
        self.canvas = FigureCanvas(self.figure)  # Initialize a canvas in the figure
        self.toolbar = NavigationToolbar(self.canvas, self) # Initialize a toolbar in the figure

        # connect the canvas to the function we want to run on a pick event
        self.canvas.mpl_connect('pick_event', self.pick_action)
        self.canvas.mpl_connect('key_press_event', self.key_event)

        # add this figure to the Qt widget layout
        layout = QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        self.setLayout(layout)

        # these are necessary for key_events
        self.canvas.setFocusPolicy(Qt.ClickFocus)
        self.canvas.setFocus()
        
        # call the routine that does the plotting
        self.alist_plot()


    def alist_plot(self):

        self.figure.clf()
        ax = self.figure.add_subplot(111) # add an Axes class to the figure

        # get the indices of the records to plot
        plot_record_idx = np.where(self.alist_data.record_flags == 0)[0]

        x_data = np.array(self.alist_data.records[self.x_field])[plot_record_idx]
        y_data = np.array(self.alist_data.records[self.y_field])[plot_record_idx]

        self.alist_data.set_record_colors(self.plot_format_dict)
        colors = np.array([self.alist_data.record_color[ii] for ii in plot_record_idx])

        x_label = pa.plot_labels[self.x_field]['label']+' '+pa.plot_labels[self.x_field]['unit']
        y_label = pa.plot_labels[self.y_field]['label']+' '+pa.plot_labels[self.y_field]['unit']

        if self.plot_format_dict['marker_style']['single_style']:
            line = ax.scatter(x_data, y_data, c=colors, marker='o', s=40., alpha=0.5, picker=5.)
            
        else:

            # get an array of the values of the records that we want to use to select marker styles
            data_marker_property = np.array(self.alist_data.records[self.plot_format_dict['marker_style']['style_property']])[plot_record_idx]
            unique_markers = np.unique(data_marker_property)
            #print(unique_markers)
            
            markers = ['s', 'o', 'v', 'P', 'X', 'D', '^', '*', 'x', '+', '>', '<', 'd']
            
            if len(unique_markers) > len(markers):
                print('Too many unique datasets to plot! We have run out of markers.')
                pass

            legend_elements = []
            for ii in range(len(unique_markers)):

                # get the indices
                idx = np.array([j for j,n in enumerate(data_marker_property) if n==unique_markers[ii]])
                
                scatterplot = ax.scatter(x_data[idx], y_data[idx], c=colors[idx], marker=markers[ii], s=40., alpha=0.5, picker=5., label=unique_markers[ii])
                

            # build the legend, set the markers in the legend to black
            ax.legend()
            leg = ax.get_legend()
            for ll in range(len(leg.legendHandles)):
                leg.legendHandles[ll].set_color('black')
        
        ax.set_title('AEDIT Demo plot')

        ax.set_xlabel(x_label)
        ax.set_ylabel(y_label)
        ax.grid(True, which='both', linestyle=':',alpha=0.8)

        self.figure.tight_layout()
        self.canvas.draw_idle()

    
    def pick_action(self, event):

        # call the plot window; this builds the plot
        if self.w is None:
            self.w = PickWindow('KZ_3C279_LR.jpg', event.ind)
        self.w.show()

        return True

    def key_event(self, event):

        print(event.key)
        
        if event.key == 'right':
            self.counter += 1
        elif event.key == 'left':
            self.counter -= 1
        else:
            return

        #counter = counter % num_plots
        print(self.counter)



        

class PlotPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data, SelectionTab, ScanTab):
        super().__init__()

        # window for plotting
        self.w = None

        self.alist_data = alist_data
        self.SelectionTab = SelectionTab
        self.ScanTab = ScanTab

        ####### Build the X-axis formatting panel
        self.xname = QLabel('X Axis')
        self.xname_hbox = QHBoxLayout()
        self.xname_hbox.addWidget(self.xname)
        self.xname_hbox.addStretch(1)

        self.xseparator = QFrame()
        self.xseparator.setFrameShape(QFrame.HLine)
        self.xseparator.setLineWidth(1)

        self.XcomboBox = QComboBox(self)
        self.XcomboBox.setFixedWidth(100)
        #self.XcomboBox.addItems(pa.plot_labels.keys())
        for k in pa.plot_labels.keys():
            self.XcomboBox.addItem(pa.plot_labels[k]['label'])
        self.XcomboBox.setCurrentText('Scan Time')
            
        self.xmenu_hbox = QHBoxLayout()
        xbox_label = QLabel('X-axis data:')
        self.xmenu_hbox.addWidget(xbox_label)
        self.xmenu_hbox.addWidget(self.XcomboBox)
        self.xmenu_hbox.addStretch(1)

        # Create a text entry box for the x-axis min and max
        xmin_box_label = QLabel('xmin ')
        self.xmin_box = QLineEdit()
        self.xmin_box.setFixedWidth(100)

        xmax_box_label = QLabel('xmax')
        self.xmax_box = QLineEdit()
        self.xmax_box.setFixedWidth(100)

        self.xrange_gridv = QVBoxLayout()

        self.xrange_gridh1 = QHBoxLayout()
        self.xrange_gridh1.addWidget(xmin_box_label)
        self.xrange_gridh1.addWidget(self.xmin_box)
        self.xrange_gridh1.addStretch(1)
        
        self.xrange_gridh2 = QHBoxLayout()
        self.xrange_gridh2.addWidget(xmax_box_label)
        self.xrange_gridh2.addWidget(self.xmax_box)
        self.xrange_gridh2.addStretch(1)

        self.xrange_gridv.addLayout(self.xrange_gridh1)
        self.xrange_gridv.addLayout(self.xrange_gridh2)



        ####### Build the Y-axis formatting panel
        self.yname = QLabel('Y Axis')
        self.yname_hbox = QHBoxLayout()
        self.yname_hbox.addWidget(self.yname)
        self.yname_hbox.addStretch(1)

        self.yseparator = QFrame()
        self.yseparator.setFrameShape(QFrame.HLine)
        self.yseparator.setLineWidth(1)
        
        self.YcomboBox = QComboBox(self)
        self.YcomboBox.setFixedWidth(100)
        #self.YcomboBox.addItems(pa.plot_labels.keys())
        for k in pa.plot_labels.keys():
            self.YcomboBox.addItem(pa.plot_labels[k]['label'])
        self.YcomboBox.setCurrentText('SNR')
        
        self.ymenu_hbox = QHBoxLayout()
        ybox_label = QLabel('Y-axis data:')
        self.ymenu_hbox.addWidget(ybox_label)
        self.ymenu_hbox.addWidget(self.YcomboBox)
        self.ymenu_hbox.addStretch(1)

        # Create a text entry box for the y-axis min and max
        ymin_box_label = QLabel('ymin ')
        self.ymin_box = QLineEdit()
        self.ymin_box.setFixedWidth(100)

        ymax_box_label = QLabel('ymax')
        self.ymax_box = QLineEdit()
        self.ymax_box.setFixedWidth(100)

        self.yrange_gridv = QVBoxLayout()

        self.yrange_gridh1 = QHBoxLayout()
        self.yrange_gridh1.addWidget(ymin_box_label)
        self.yrange_gridh1.addWidget(self.ymin_box)
        self.yrange_gridh1.addStretch(1)
        
        self.yrange_gridh2 = QHBoxLayout()
        self.yrange_gridh2.addWidget(ymax_box_label)
        self.yrange_gridh2.addWidget(self.ymax_box)
        self.yrange_gridh2.addStretch(1)

        self.yrange_gridv.addLayout(self.yrange_gridh1)
        self.yrange_gridv.addLayout(self.yrange_gridh2)





        ####### Datapoint style selections and plot sequence
        self.dseparator = QFrame()
        self.dseparator.setFrameShape(QFrame.HLine)
        self.dseparator.setLineWidth(1)

        
        self.markerstyle_chkbox = QCheckBox('Use a single marker style', self)
        self.markerstyle_chkbox.setCheckable(True)
        self.markerstyle_chkbox.setChecked(True)

        # doesn't make sense to have markers based on station since each record has two stations
        self.marker_style = ['Source','QCode','Polarization','Baseline']
        self.MarkercomboBox = QComboBox(self)
        self.MarkercomboBox.setFixedWidth(100)
        self.MarkercomboBox.addItems(self.marker_style)
        self.MarkercomboBox.setEnabled(False)
        
        marker_hbox = QHBoxLayout()
        markerbox_label = QLabel('Marker style according to:')
        markerbox_label.setEnabled(False)
        marker_hbox.addWidget(markerbox_label)
        marker_hbox.addWidget(self.MarkercomboBox)
        marker_hbox.addStretch(1)


        
        # aedit sets the marker color as: SNR<5.5 red, SNR<6.5 orange, SNR>=6.5 green
        # change to blue for colorblindness. plot_quality.c and plot_points.c
        
        # checkbox / combobox for plotting order
        self.pointcolor_chkbox = QCheckBox('Datapoint color from SNR?', self)
        self.pointcolor_chkbox.setCheckable(True)
        self.pointcolor_chkbox.setChecked(True)

        pointqualty_hbox = QHBoxLayout()
        pointquality_label1 = QLabel('SNR<')
        pointquality_label2 = QLabel('SNR<')
        pointquality_label3 = QLabel('SNR>=')
        pointquality_label1.setEnabled(True)
        pointquality_label2.setEnabled(True)
        pointquality_label3.setEnabled(True)
        
        pointquality_label4 = QLabel('red, ')
        pointquality_label5 = QLabel('orange, ')
        pointquality_label6 = QLabel('blue')
        pointquality_label4.setEnabled(True)
        pointquality_label5.setEnabled(True)
        pointquality_label6.setEnabled(True)

        self.badsnr_box = QLineEdit()
        self.badsnr_box.setText('5.5')
        self.badsnr_box.setFixedWidth(40)
        self.badsnr_box.setEnabled(True)

        self.suspectsnr_box = QLineEdit()
        self.suspectsnr_box.setText('6.5')
        self.suspectsnr_box.setFixedWidth(40)
        self.suspectsnr_box.setEnabled(True)
        
        self.goodsnr_box = QLineEdit()
        self.goodsnr_box.setText('6.5')
        self.goodsnr_box.setFixedWidth(40)
        self.goodsnr_box.setEnabled(True)
        
        pointqualty_hbox.addWidget(self.pointcolor_chkbox)
        pointqualty_hbox.addStretch(1)
        pointqualty_hbox.addWidget(pointquality_label1)
        pointqualty_hbox.addWidget(self.badsnr_box)
        pointqualty_hbox.addWidget(pointquality_label4)
        pointqualty_hbox.addWidget(pointquality_label2)
        pointqualty_hbox.addWidget(self.suspectsnr_box)
        pointqualty_hbox.addWidget(pointquality_label5)
        pointqualty_hbox.addWidget(pointquality_label3)
        pointqualty_hbox.addWidget(self.goodsnr_box)
        pointqualty_hbox.addWidget(pointquality_label6)
        pointqualty_hbox.addStretch(1)
        pointqualty_hbox.addStretch(1)
        
        # checkbox / combobox for plotting order
        self.plotall_chkbox = QCheckBox('Plot all data in a single figure', self)
        self.plotall_chkbox.setCheckable(True)
        self.plotall_chkbox.setChecked(True)
        
        self.plot_datasets = ['Baseline','Station','Source','Polarization']
        self.PlotcomboBox = QComboBox(self)
        self.PlotcomboBox.setFixedWidth(100)
        self.PlotcomboBox.addItems(self.plot_datasets)
        self.PlotcomboBox.setEnabled(False)
        
        dataset_hbox = QHBoxLayout()
        plotbox_label = QLabel('Group data into figures by:')
        plotbox_label.setEnabled(False)
        dataset_hbox.addWidget(plotbox_label)
        dataset_hbox.addWidget(self.PlotcomboBox)
        dataset_hbox.addStretch(1)
        
        dataset_vbox = QVBoxLayout()
        dataset_vbox.addLayout(pointqualty_hbox)
        dataset_vbox.addWidget(self.markerstyle_chkbox)
        dataset_vbox.addLayout(marker_hbox)
        dataset_vbox.addWidget(self.plotall_chkbox)
        dataset_vbox.addLayout(dataset_hbox)


        # functions for toggling (greying out) different options
        def toggleFigureBox(state):
            if state > 0:
                self.PlotcomboBox.setEnabled(False)
                plotbox_label.setEnabled(False)
            else:
                self.PlotcomboBox.setEnabled(True)
                plotbox_label.setEnabled(True)

        def toggleMarkerBox(state):
            if state > 0:
                self.MarkercomboBox.setEnabled(False)
                markerbox_label.setEnabled(False)
            else:
                self.MarkercomboBox.setEnabled(True)
                markerbox_label.setEnabled(True)
                
                
        def togglePointQualityBox(state):
            if state > 0:
                self.badsnr_box.setEnabled(True)
                self.suspectsnr_box.setEnabled(True)
                self.goodsnr_box.setEnabled(True)
                pointquality_label1.setEnabled(True)
                pointquality_label2.setEnabled(True)
                pointquality_label3.setEnabled(True)
                pointquality_label4.setEnabled(True)
                pointquality_label5.setEnabled(True)
                pointquality_label6.setEnabled(True)
            else:
                self.badsnr_box.setEnabled(False)
                self.suspectsnr_box.setEnabled(False)
                self.goodsnr_box.setEnabled(False)
                pointquality_label1.setEnabled(False)
                pointquality_label2.setEnabled(False)
                pointquality_label3.setEnabled(False)
                pointquality_label4.setEnabled(False)
                pointquality_label5.setEnabled(False)
                pointquality_label6.setEnabled(False)

                
        self.plotall_chkbox.stateChanged.connect(toggleFigureBox)
        self.markerstyle_chkbox.stateChanged.connect(toggleMarkerBox)
        self.pointcolor_chkbox.stateChanged.connect(togglePointQualityBox)

        

        # button to make the plot
        self.plot_button = QPushButton("Plot Data")
        self.plot_button.clicked.connect(self.PlottingButton)
        plot_hbox = QHBoxLayout()
        plot_hbox.addStretch(1)
        plot_hbox.addWidget(self.plot_button)

        # build the full panel vertically
        # the stretches here do the vertical centering
        plot_vbox = QVBoxLayout()
        #plot_vbox.addStretch(1)
        plot_vbox.addLayout(self.xname_hbox)
        plot_vbox.addWidget(self.xseparator)
        plot_vbox.addLayout(self.xmenu_hbox)
        plot_vbox.addLayout(self.xrange_gridv)
        plot_vbox.addStretch(1)
        plot_vbox.addLayout(self.yname_hbox)
        plot_vbox.addWidget(self.yseparator)
        plot_vbox.addLayout(self.ymenu_hbox)
        plot_vbox.addLayout(self.yrange_gridv)
        plot_vbox.addStretch(1)
        plot_vbox.addWidget(self.dseparator)
        plot_vbox.addLayout(dataset_vbox)
        plot_vbox.addLayout(plot_hbox)
        #plot_vbox.addStretch(1)
        
        self.setLayout(plot_vbox)


        
    def PlottingButton(self, checked):

        #if self.w is not None:
        #    self.w.figure.clf()
        #    self.w.close()
            
        # open the plot window from here

        # get the data selections, pass to the indexing function in the alist class
        data_selection_dict = self.SelectionTab.CollectDataSelections()
        scan_selection_dict = self.ScanTab.CollectScanSelections()

        # get the formatting selections
        plot_format_dict = {}
        plot_format_dict['axis_range'] = {}
        plot_format_dict['axis_range']['xrange'] = [self.xmax_box.text(), self.xmax_box.text()]
        plot_format_dict['axis_range']['yrange'] = [self.ymax_box.text(), self.ymax_box.text()]
        plot_format_dict['point_color'] = {}
        plot_format_dict['point_color']['color_by_SNR'] = self.pointcolor_chkbox.isChecked()
        plot_format_dict['point_color']['bad_threshold'] = float(self.badsnr_box.text())
        plot_format_dict['point_color']['suspect_threshold'] = float(self.suspectsnr_box.text())
        plot_format_dict['point_color']['good_threshold'] = float(self.goodsnr_box.text())
        plot_format_dict['marker_style'] = {}
        plot_format_dict['marker_style']['single_style'] = self.markerstyle_chkbox.isChecked()
        plot_format_dict['marker_style']['style_property'] = pa.plot_label_convert[self.MarkercomboBox.currentText()]
        plot_format_dict['plot_order'] = {}
        plot_format_dict['plot_order']['single_figure'] = self.plotall_chkbox.isChecked()
        plot_format_dict['plot_order']['figure_order_property'] = pa.plot_label_convert[self.PlotcomboBox.currentText()]

        
        # this method returns the indices and updates the flagging, self.alist_data.record_flags
        alist_idx = self.alist_data.get_record_indices(data_selection_dict, scan_selection_dict)
        # note, this is not sorted - should we sort it?

        # get the selected x and y axis data for the plot
        x_field = list(pa.plot_labels.keys())[self.XcomboBox.currentIndex()]
        y_field = list(pa.plot_labels.keys())[self.YcomboBox.currentIndex()]


        # call the plot window; this builds the plot
        #if self.w is None:
        #print('New plot window')
        self.w = PlotWindow(self.alist_data, x_field, y_field, plot_format_dict)
        self.w.show()
        #else:
        #    print(self.w)
        #    return

        
        


# Class for the Summary panel
class SummPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data, SelectionTab, ScanTab):
        super().__init__()

        self.alist_data = alist_data

        self.summ_box = QTextEdit()
        #self.summ_box.resize(300,400)

        text_string = '\n'
        text_string += 'Hello here is some text.\n'
        text_string += '\n'
        text_string += 'SUMMARY OF UNFLAGGED DATA IN MEMORY\n-----------------------------------\n'
        text_string += '\n'
        text_string += 'Total number of unflagged fringe records = 6754\n'
        text_string += '\n'
        text_string += 'Earliest scan:       94-015-183000\n'
        text_string += 'Latest scan:         94-016-181505\n'
        text_string += 'Earliest procdate:   94-050-1648\n'
        text_string += 'Latest procdate:     94-055-1044\n'
        text_string += 'Stations present:    DAKLETV\n'
        text_string += 'Baselines present:   DA DK DL DE AK AL AE KL KE LE TE AT AV TV EV KT KV DV LV DT LT\n'
        text_string += 'Frequencies present: XS\n'
        text_string += 'SNR extrema:         0.000  1069.\n'
        text_string += 'Experiments present: 2498\n'
        text_string += 'Sources present:     0048-097 0059+581 0119+041 0229+131 0454-234 0458-020\n'
        text_string += '        0528+134 0537-441 0552+398 0727-115 0735+178 0804+499 0820+560\n'
        text_string += '        0823+033 0919-260 0954+658 0955+476 1034-293 1044+719 1053+815\n'
        text_string += '        1104-445 1128+385 1219+044 1308+326 1334-127 1357+769 1424-418\n'
        text_string += '        1606+106 1622-253 1726+455 1739+522 1741-038 1749+096 1921-293\n'
        text_string += '        2145+067 2234+282 2255-282 4C39.25 NRAO512 OJ287 OK290\n'
        text_string += 'Quality code summary:\n'
        text_string += '        A B C D   E F  0  1 2  3 4  5  6   7   8   9    ?\n'
        text_string += '        0 2 0 137 3 93 88 0 46 5 18 60 144 211 851 5096 0\n'
        text_string += '\n'
        text_string += 'There are 0 flagged records present\n'
        text_string += '\n'

        self.summ_box.setPlainText(text_string)
        self.summ_box.setReadOnly(True)

        self.updateButton = QPushButton('Update Summary')

        updatebutton_hbox = QHBoxLayout()
        updatebutton_hbox.addStretch(1)
        updatebutton_hbox.addWidget(self.updateButton)
        
        summ_vbox = QVBoxLayout()
        summ_vbox.addWidget(self.summ_box)
        summ_vbox.addLayout(updatebutton_hbox)
        #summ_vbox.addWidget(self.xseparator)
        #plot_vbox.addLayout(self.xmenu_hbox)
        #plot_vbox.addLayout(self.xrange_gridv)
        #plot_vbox.addStretch(1)
        
        self.setLayout(summ_vbox)

        


        
# Class for the Data window
class DataWindow(QWidget):

    def __init__(self, alist_file, **kwargs):
        super().__init__()

        self.container = QVBoxLayout(self)
        
        self.alist_file = alist_file
    
        self.setWindowTitle('Alist Data')

        # Create a tabs widget in this window
        self.tabs = QTabWidget()
        self.tabs.setTabPosition(QTabWidget.North)
        self.tabs.setMovable(True)

        self.SelectionTab = SelectionPanel(self.alist_file)
        self.ScanTab = ScanPanel(self.alist_file)
        self.PlotTab = PlotPanel(self.alist_file, self.SelectionTab, self.ScanTab)
        self.SummTab = SummPanel(self.alist_file, self.SelectionTab, self.ScanTab)
        
        self.tab1 = self.SelectionTab
        self.tab2 = self.ScanTab
        self.tab3 = self.SummTab # summary panel
        self.tab4 = self.PlotTab # plot panel
        self.tab5 = QWidget() # calc panel
        self.tab6 = QWidget() # save panel
        
        # Add tabs, specifying the widget in each tab
        self.tabs.addTab(self.tab1, 'Select')
        self.tabs.addTab(self.tab2, 'Scans')
        self.tabs.addTab(self.tab3, 'Summ')
        self.tabs.addTab(self.tab4, 'Plot')
        self.tabs.addTab(self.tab5, 'Calc')
        self.tabs.addTab(self.tab6, 'Save')
        
        self.container.addWidget(self.tabs)

        # golden aspect ratio
        wsize = 800
        #self.setFixedSize(QSize(wsize,wsize*0.618))
        self.setMinimumSize(QSize(wsize,wsize*1.2))



        



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
        init_box_label = QLabel('Alist file(s):')
        self.init_afile_box = QPlainTextEdit()#QLineEdit()
        if self.alist_file is not None:
            self.init_afile_box.insertPlainText(self.alist_file)
            #self.init_afile_box.setText(self.alist_file)
        #else:
        #    self.init_afile_box.setText()
        #self.init_afile_box.setFixedWidth(300)
        self.init_afile_box.setMinimumWidth(300)
        self.init_afile_box.setMinimumHeight(200)
        
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

        #afilename = self.init_afile_box.text()
        afilename = self.init_afile_box.toPlainText()
                
        alist_data = pa.ParseAlist(afilename)

        if self.w is None:
            self.w = DataWindow(alist_data)
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
        #self.tab2 = QWidget()
        #self.tab3 = QWidget()
        #self.tab4 = QWidget()
        
        # Add tabs, specifying the widget in each tab
        self.tabs.addTab(self.tab1, 'Init')
        #self.tabs.addTab(self.tab2, 'Select')
        #self.tabs.addTab(self.tab3, 'Plot')
        #self.tabs.addTab(self.tab4, 'Save')
        
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
