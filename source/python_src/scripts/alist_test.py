#!/usr/bin/env python

import sys

from PyQt5.QtCore import QSize, Qt, QLine, QPoint
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QMainWindow, QCheckBox, QFrame#, QSizePolicy
from PyQt5.QtWidgets import QVBoxLayout, QHBoxLayout, QGridLayout, QTabWidget, QLabel, QLineEdit, QComboBox, QGroupBox, QPlainTextEdit, QTextEdit
from PyQt5.QtGui import QFont

import parse_alist as pa

#import matplotlib
#matplotlib.use('Qt5Agg')

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar

import matplotlib.pyplot
#from matplotlib.lines import Line2D

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

        return [float(self.snrmin_box.text()), float(self.snrmax_box.text())]

        




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

    
    def getUncheckedBoxes(self):

        unchecked_boxes = []        
        for checkbox in self.checkboxes:
            if checkbox.isChecked():
                pass
            else:
                unchecked_boxes.append(checkbox.text())

        return unchecked_boxes

    




    
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
        data_selection_dict['not_stations'] = self.station_checkboxes.getUncheckedBoxes()
        data_selection_dict['qcodes'] = self.qcode_checkboxes.collectCheckedBoxes()
        data_selection_dict['sources'] = self.source_checkboxes.collectCheckedBoxes()        
        data_selection_dict['pols'] = self.pol_checkboxes.collectCheckedBoxes()
        data_selection_dict['snrrange'] = self.param_textbox.collectParamVals()

        
        if len(data_selection_dict['stations'])<2:
            print('WARNING - must choose more than one station!')
        
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
        
        #self.pick_canvas.draw_idle()

        


# aedit plot window
class PlotWindow(QWidget):
    def __init__(self, alist_data, plot_format_dict):        
        super().__init__()
        
        # Preliminaries
        self.alist_data = alist_data
        self.plot_format_dict = plot_format_dict
        self.x_field = self.plot_format_dict['x_field']
        self.y_field = self.plot_format_dict['y_field']
        self.size_property = self.plot_format_dict['point_size']['pointsize_property']
        #self.figure_order_property = pa.plot_label_convert[self.plot_format_dict['plot_order']['figure_order_property']]
        self.pw = None # window for picked datapoint
        self.fig_counter = 0 # for scrolling through multiple plots, eg by baseline
        
        #matplotlib.style.use('classic')
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


        # get the indices of the records that correspond to the selected properties
        # create a second copy to handle the records for different plots (if only one plot is requested, figure_record_idx won't be modified)
        self.plot_record_idx = np.where(self.alist_data.record_flags == 0)[0]
        self.figure_record_idx = np.copy(self.plot_record_idx)
        
        # call the routine that does the plotting
        if self.plot_format_dict['plot_order']['single_figure']:
            self.alist_plot()

        else:
            # figure out the order of the figures to scroll through
            # plot the first one, the key event will handle the navigation between figures
            self.figure_order_property = pa.plot_label_convert[self.plot_format_dict['plot_order']['figure_order_property']]
            self.figure_order_elements = np.array(self.alist_data.records[self.figure_order_property])[self.plot_record_idx]

            self.unique_figures = np.unique(self.figure_order_elements)
            self.num_figures = len(self.unique_figures)
            
            # update the indices of the records to plot; we will repeat this in the key_event function below
            pidx = np.array(self.alist_data.records['index'])[self.plot_record_idx]
            self.figure_record_idx = np.array([pidx[j] for j,n in enumerate(self.figure_order_elements) if n==self.unique_figures[self.fig_counter]])

            self.alist_plot(figure_ordering=self.unique_figures[0])


    def alist_plot(self, figure_ordering=None):

        self.figure.clf() # this clears the figure, but it does not erase it
        #try:
        #    self.ax.cla()
        #except:
        #    pass
        
        self.ax = self.figure.add_subplot(111) # give the figure an Axes instance

        #print(self.unique_figures[self.fig_counter])
        #print(self.figure_order_elements[self.figure_record_idx])
        #print(np.array(self.alist_data.records[self.figure_order_property])[self.figure_record_idx])

        # get the x- and y-data arrays for the selected alist fields
        x_data = np.array(self.alist_data.records[self.x_field])[self.figure_record_idx]
        y_data = np.array(self.alist_data.records[self.y_field])[self.figure_record_idx]
        self.record_idx = np.array(self.alist_data.records['index'])[self.figure_record_idx] # get the absolute indices of the selected records, for picking

        # work out the marker size
        if self.plot_format_dict['point_size']['pointsize']:
            size_data = np.array(self.alist_data.records[self.size_property])[self.figure_record_idx]

            # scatterplot marker size is in point^2, or area. Scale each property individually so the markersize is sensible
            if self.size_property == 'amplitude':
                s_data = np.sqrt(100*np.abs(size_data))

            elif self.size_property == 'snr':
                s_data = np.sqrt(4*np.abs(size_data))

            elif self.size_property == 'phase':
                s_data = np.sqrt(10*np.abs(size_data))
                
            elif self.size_property == 'sbd':
                s_data = 100*np.sqrt(np.abs(size_data))
                
            elif self.size_property == 'mbd':
                s_data = 10000*np.abs(size_data)
                
            elif self.size_property == 'delay_rate':
                s_data = 50*np.sqrt(np.abs(size_data))
                
            else:
                print('Marker size selection not supported!')
                s_data=20.
                
        else:
            s_data=20.
            
        self.alist_data.set_record_colors(self.plot_format_dict)
        colors = np.array([self.alist_data.record_color[ii] for ii in self.figure_record_idx])

        x_label = pa.plot_labels[self.x_field]['label']+' '+pa.plot_labels[self.x_field]['unit']
        y_label = pa.plot_labels[self.y_field]['label']+' '+pa.plot_labels[self.y_field]['unit']


        self.picking_dict = {} # initialize a dict to keep track of the plotting artists
        if self.plot_format_dict['marker_style']['single_style']:
            idx = np.array([j for j,n in enumerate(x_data)])
            #scatterplot = self.ax.scatter(x_data, y_data, c=colors, marker='o', facecolors='none', s=40., alpha=0.5, picker=5.)
            scatterplot = self.ax.scatter(x_data, y_data, edgecolors=colors, marker='o', facecolors='none', s=s_data, alpha=0.5, picker=5.)
            self.picking_dict[scatterplot] = idx
            
        else:

            # get an array of the values of the records that we want to use to select marker styles
            data_marker_property = self.plot_format_dict['marker_style']['style_property']
            data_marker_elements = np.array(self.alist_data.records[data_marker_property])[self.figure_record_idx]
            unique_markers = np.unique(data_marker_elements)
            #print(unique_markers)

            # can't use non-filled markers if facecolors='none'??
            markers = ['s', 'o', 'v', 'P', 'X', 'D', '^', '*', '>', '<', 'd']
            #markers = ['x', '+', 's', 'o', 'v', 'P', 'X', 'D', '^', '*', '>', '<', 'd']
            
            if len(unique_markers) > len(markers):
                print('Too many unique datasets to plot! We have run out of markers.')
                pass

            legend_elements = []
            for ii in range(len(unique_markers)):

                # if more entries than markers, loop on markers
                if ii>=len(markers):
                    jj = ii%len(markers)
                else:
                    jj=ii
                    
                # get the indices
                idx = np.array([j for j,n in enumerate(data_marker_elements) if n==unique_markers[ii]])
                
                #scatterplot = self.ax.scatter(x_data[idx], y_data[idx], c=colors[idx], marker=markers[ii],
                #                         s=40., facecolors='none', alpha=0.5, picker=5., label=unique_markers[ii])
                scatterplot = self.ax.scatter(x_data[idx], y_data[idx], edgecolors=colors[idx], marker=markers[jj],
                                         s=s_data, facecolors='none', alpha=0.5, picker=5., label=unique_markers[ii])

                # we'll have to do something here to enable the picking
                self.picking_dict[scatterplot] = idx
                
            # build the legend, set the markers in the legend to black
            self.ax.legend()
            leg = self.ax.get_legend()
            for ll in range(len(leg.legendHandles)):
                leg.legendHandles[ll].set_edgecolor('black') # this will effect the datapoints too? only if it's set_edgecolor?
                leg.legendHandles[ll]._sizes = [20.]

        if figure_ordering is not None:
            self.ax.set_title('AEDIT Plot - '+self.figure_order_property+': '+figure_ordering)
        else:
            self.ax.set_title('AEDIT Demo plot')
            
        self.ax.set_xlabel(x_label)
        self.ax.set_ylabel(y_label)
        self.ax.grid(True, which='both', linestyle=':',alpha=0.8)

        self.figure.tight_layout()
        self.canvas.draw()

    
    def pick_action(self, event):

        # work out the record index of the picked data point
        idx = self.picking_dict[event.artist]
        ridx = self.record_idx[idx[event.ind]][0]
        print(ridx, self.alist_data.records['scan'][ridx],
              self.alist_data.records['source'][ridx],
              self.alist_data.records['baseline'][ridx],
              self.alist_data.records['snr'][ridx],
              self.alist_data.records['pols'][ridx],
              self.alist_data.records['qcode'][ridx])
        
        # call the plot window; this builds the plot
        #if self.pw is None:
        #    self.pw = PickWindow('KZ_3C279_LR.jpg', event.ind)
        #self.pw.show()

        return True

    def key_event(self, event):

        # If we only requested a single figure, skip
        if self.plot_format_dict['plot_order']['single_figure']:
            pass

        # otherwise, draw the next plot in the sequence
        else:
            
            # use the key direction to update the figure counter
            if event.key == 'right':
                self.fig_counter += 1
            elif event.key == 'left':
                self.fig_counter -= 1
            else:
                return

            self.fig_counter = self.fig_counter % self.num_figures
        
            # update the indices of the records to plot
            pidx = np.array(self.alist_data.records['index'])[self.plot_record_idx]
            self.figure_record_idx = np.array([pidx[j] for j,n in enumerate(self.figure_order_elements) if n==self.unique_figures[self.fig_counter]])

            self.alist_plot(figure_ordering=self.unique_figures[self.fig_counter])

        return True

        

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
        self.MarkercomboBox.setFixedWidth(110)
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

        # property for data point size
        self.pointsize_chkbox = QCheckBox('Datapoint size from: ', self)
        self.pointsize_chkbox.setCheckable(True)
        self.pointsize_chkbox.setChecked(False)

        pointsize_hbox = QHBoxLayout()

        self.size_datasets = ['Scan time','Amplitude','SNR','Phase','Single-band delay', 'Multi-band delay', 'Delay rate']
        self.SizecomboBox = QComboBox(self)
        self.SizecomboBox.setFixedWidth(130)
        self.SizecomboBox.addItems(self.size_datasets)
        self.SizecomboBox.setEnabled(False)

        pointsize_hbox.addWidget(self.pointsize_chkbox)
        pointsize_hbox.addWidget(self.SizecomboBox)
        pointsize_hbox.addStretch(1)

        
        # checkbox / combobox for plotting order
        self.plotall_chkbox = QCheckBox('Plot all data in a single figure', self)
        self.plotall_chkbox.setCheckable(True)
        self.plotall_chkbox.setChecked(True)
        
        self.plot_datasets = ['Baseline','Source','Polarization','QCode']
        self.PlotcomboBox = QComboBox(self)
        self.PlotcomboBox.setFixedWidth(110)
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
        dataset_vbox.addLayout(pointsize_hbox)
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

        def togglePointSizeBox(state):
            if state > 0:
                self.SizecomboBox.setEnabled(True)
                self.pointsize_chkbox.setEnabled(True)
            else:
                self.SizecomboBox.setEnabled(False)
                #self.pointsize_chkbox.setEnabled(False)

                
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
        self.pointsize_chkbox.stateChanged.connect(togglePointSizeBox)

        

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
        plot_format_dict['x_field'] = pa.plot_label_convert[self.XcomboBox.currentText()]
        plot_format_dict['y_field'] = pa.plot_label_convert[self.YcomboBox.currentText()]
        plot_format_dict['axis_range'] = {}
        plot_format_dict['axis_range']['xrange'] = [self.xmax_box.text(), self.xmax_box.text()]
        plot_format_dict['axis_range']['yrange'] = [self.ymax_box.text(), self.ymax_box.text()]
        plot_format_dict['point_color'] = {}
        plot_format_dict['point_color']['color_by_SNR'] = self.pointcolor_chkbox.isChecked()
        plot_format_dict['point_color']['bad_threshold'] = float(self.badsnr_box.text())
        plot_format_dict['point_color']['suspect_threshold'] = float(self.suspectsnr_box.text())
        plot_format_dict['point_color']['good_threshold'] = float(self.goodsnr_box.text())
        plot_format_dict['point_size'] = {}
        plot_format_dict['point_size']['pointsize'] = self.pointsize_chkbox.isChecked()
        plot_format_dict['point_size']['pointsize_property'] = pa.plot_label_convert[self.SizecomboBox.currentText()]
        plot_format_dict['marker_style'] = {}
        plot_format_dict['marker_style']['single_style'] = self.markerstyle_chkbox.isChecked()
        plot_format_dict['marker_style']['style_property'] = pa.plot_label_convert[self.MarkercomboBox.currentText()]
        plot_format_dict['plot_order'] = {}
        plot_format_dict['plot_order']['single_figure'] = self.plotall_chkbox.isChecked()
        plot_format_dict['plot_order']['figure_order_property'] = self.PlotcomboBox.currentText() # don't convert this one, need it later

        
        # this method returns the indices and updates the flagging, self.alist_data.record_flags
        alist_idx = self.alist_data.get_record_indices(data_selection_dict, scan_selection_dict)
        # note, this is not sorted - should we sort it?

        # get the selected x and y axis data for the plot
        #x_field = list(pa.plot_labels.keys())[self.XcomboBox.currentIndex()]
        #y_field = list(pa.plot_labels.keys())[self.YcomboBox.currentIndex()]
        

        # call the plot window; this builds the plot
        #if self.w is None:
        #print('New plot window')
        self.w = PlotWindow(self.alist_data, plot_format_dict)
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
        self.SelectionTab = SelectionTab
        self.ScanTab = ScanTab

        font = QFont("Courier", 11)
        QApplication.setFont(font, "QPlainTextEdit")
        
        #QtWidgets.QApplication.setFont(font, "MyText")
        
        self.summ_box = QPlainTextEdit()
        #self.summ_box.resize(300,400)
        self.summ_box.setReadOnly(True)

        self.updateButton = QPushButton('Update Summary')
        self.updateButton.clicked.connect(self.UpdateSummary)
        
        updatebutton_hbox = QHBoxLayout()
        updatebutton_hbox.addStretch(1)
        updatebutton_hbox.addWidget(self.updateButton)
        
        summ_vbox = QVBoxLayout()
        summ_vbox.addWidget(self.summ_box)
        summ_vbox.addLayout(updatebutton_hbox)
        
        self.setLayout(summ_vbox)



    def UpdateSummary(self):

        # this method returns the indices and updates the flagging, self.alist_data.record_flags
        data_selection_dict = self.SelectionTab.CollectDataSelections()
        scan_selection_dict = self.ScanTab.CollectScanSelections()

        alist_idx = self.alist_data.get_record_indices(data_selection_dict, scan_selection_dict)

        tot_unflagged_records_str = str(len(alist_idx))
        tot_flagged_records_str = str(len(np.where(self.alist_data.record_flags == 1)[0]))



        # baselines present in the unflagged records
        unique_baselines = np.unique(np.array(self.alist_data.records['baseline'])[alist_idx])
        if len(unique_baselines) > 18:
            baseline_str = ''
            for ii in range(len(unique_baselines)):
                if ii % 18 == 00 and ii>0:
                    baseline_str += '\n        '
                baseline_str += unique_baselines[ii]+' '
        else:
            baseline_str = ' '.join(unique_baselines)

        # sources present in the unflagged records
        unique_sources = np.unique(np.array(self.alist_data.records['source'])[alist_idx])
        if len(unique_sources)>6:
            # break up sources into multiple lines
            source_str = ''
            for ii in range(len(unique_sources)):
                if ii % 6 == 0 and ii>0:
                    source_str += '\n                     '
                source_str += unique_sources[ii]+' '
        else:
            source_str = ' '.join(unique_sources)

        # stations in the unflagged records
        unique_stations = np.unique(np.concatenate(np.array(self.alist_data.records['stations'])[alist_idx]).flat)
        station_str = ''.join(unique_stations)

        # SNR range
        snr_extrema = str(np.round(np.min(np.array(self.alist_data.records['snr'])[alist_idx]),3))+ \
                      '    '+str(np.round(np.max(np.array(self.alist_data.records['snr'])[alist_idx]),2))

        # experiments
        unique_exper = np.unique(np.array(self.alist_data.records['experiment'])[alist_idx])
        exper_str = ''.join(unique_exper)

        # counts for each quality code
        unique_qcodes, qcode_counts = np.unique(np.array(self.alist_data.records['qcode'])[alist_idx],return_counts=True)

        qcodes = ['A','B','C','D','E','F','G','H','0','1','2','3','4','5','6','7','8','9','N','?']

        qcode_str = ''
        qcode_count_str = ''
        for qc in qcodes:
            if qc in unique_qcodes:
                ii = list(unique_qcodes).index(qc)
                qc_str = str(qcode_counts[ii])
                qcode_str += qc + ' '*len(qc_str)
                qcode_count_str += qc_str+' '
            else:
                qcode_str += qc + ' '
                qcode_count_str += '0 '
                

        # earliest and latest scan in the unflagged records
        early_scan_idx = np.argmin(np.array(self.alist_data.records['scan_time'])[alist_idx])
        late_scan_idx = np.argmax(np.array(self.alist_data.records['scan_time'])[alist_idx])
        early_scan = str(np.array(self.alist_data.records['scan_timetag'])[alist_idx][early_scan_idx])
        late_scan = str(np.array(self.alist_data.records['scan_timetag'])[alist_idx][late_scan_idx])

        # frequencies
        unique_freqs = np.unique(np.array(self.alist_data.records['frequency_band'])[alist_idx])
        freq_str = ''.join(unique_freqs)
                
        
        text_string = '\n'
        text_string += '                       SUMMARY OF UNFLAGGED DATA IN MEMORY\n'
        text_string += '                       -----------------------------------\n'
        text_string += '\n'
        text_string += 'Total number of unflagged fringe records = '+tot_unflagged_records_str+'\n'
        text_string += '\n'
        text_string += 'Earliest scan:       '+early_scan+'\n'
        text_string += 'Latest scan:         '+late_scan+'\n'
        #text_string += 'Earliest procdate:   94-050-1648\n'
        #text_string += 'Latest procdate:     94-055-1044\n'
        text_string += 'Stations present:    '+station_str+'\n'
        text_string += 'Baselines present:   '+baseline_str+'\n'
        text_string += 'Experiments present: '+exper_str+'\n'
        text_string += 'Frequencies present: '+freq_str+'\n'
        text_string += 'SNR extrema:         '+snr_extrema+'\n'
        text_string += 'Sources present:     '+source_str+'\n'
        text_string += 'Quality code summary:\n'
        text_string += '        '+qcode_str+'\n'
        text_string += '        '+qcode_count_str+'\n'
        text_string += '\n'
        text_string += 'There are '+tot_flagged_records_str+' flagged records present\n'
        text_string += '\n'

        self.summ_box.setPlainText(text_string)



        


        
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

        # initialize an alist data object
        alist_data = pa.ParseAlist()
        
        #afilename = self.init_afile_box.text()
        afilenames = self.init_afile_box.toPlainText().split('\n')

        for afile in afilenames:
            alist_data.load_alist_data(afile)

            
        if self.w is None:
            self.w = DataWindow(alist_data)
        self.w.show()



        

class MainWindow(QMainWindow):
    def __init__(self, **kwargs):
        super().__init__()

        if 'alist_file' in kwargs:
            self.alist_file = '\n'.join(kwargs['alist_file'])
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
        window = MainWindow(alist_file=sys.argv[1:])
    else:
        window = MainWindow()
    
    window.show()

    app.exec()
