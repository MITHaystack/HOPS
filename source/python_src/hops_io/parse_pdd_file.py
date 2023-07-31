
import os, sys


def parse_PDD(filename):
    '''
    Function to parse the output of fourfit data dump file.

    Parameters
    ----------

    filename : str
        Path to the PDD file to parse

    Returns
    -------

    plot_dict : dict
        Dictionary with key/value pairs for fields in the PDD file.


    '''
    


    # 
    # f = open(filename)
    # lines = f.readlines()
    # f.close()

    # initialize the dictionary we'll build of the dataypes in the PDD file
    pdd_dict = {}

    with open(filename, 'r', errors='replace') as f:
        lines = f.readlines()

        # datatypes that require an extra line skip
        skip2 = ['SEG_AMP', 'SEG_PHS', 'WEIGHTS', 'PHASOR-ABS', 'PHASOR-ARG', 'MEAN_AP']
        
        ii=0
        while ii<len(lines):
            
            line_split = lines[ii].split(None)

            print(line_split)
            # if the line is empty, skip it
            if len(line_split)<2:
                ii+=1
                continue

            # annoying special cases
            if line_split[1]=='RootScanBaseline':
                pdd_dict['RootScanBaseline'] = line_split[2]+' '+line_split[3]+' '+line_split[4]
                ii+=1
                continue
            if line_split[1]=='CorrVers':
                #pdd_dict['CorrVers'] = line_split[2]+' '+line_split[3]+' '+line_split[4]+' '+line_split[5]+' '+line_split[6]
                ii+=1
                continue
            
            # Parse the lines: 
            #  - if the line is len==3, we'll assume it is of the form ['#', KEY, VALUE], and assign appropriately
            #  - if the line is len==4 and has curly brackets AND the subsequent line has no #, we'll build an array
            # TODO: add some sanity checks to make sure the arrays have the expected number of elements

            # We need to check if the next line is a new field
            next_line = lines[ii+1].split(None)

            if line_split[0] == '#' and len(line_split)==3:
                ii+=1
                val = line_split[2]
                pdd_dict[line_split[1]] = val

            elif line_split[0] == '#' and len(line_split)>=4 and '{' in line_split[3] and next_line[0] != '#':

                # initialize a list to store the array of values
                arr = []
                #print(line_split[1])

                # some datatypes require skipping two lines, this is very annoying
                if line_split[1] in skip2:
                    increment = 2                
                else:
                    increment = 1

                jj=increment
                next_line = lines[ii+jj].split(None)
                while next_line[0] != '#':
                    arr.extend([float(nn) for nn in next_line])
                    jj+=increment
                    next_line = lines[ii+jj].split(None)
                    if len(next_line)<2:
                        break
                    if increment==2:
                        # need to test that we aren't skipping a line that starts a new field
                        test_line = lines[ii+jj-1].split(None)
                        if len(test_line)<1 or test_line[0] == '#':
                            break

                # add the list of values to the dictionary
                pdd_dict[line_split[1]] = arr
                ii+=jj-1
                
            elif line_split[1] == 'eof':
                break
            else:
                ii+=1

    return pdd_dict
            
