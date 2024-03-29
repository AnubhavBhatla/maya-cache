import os
import pandas as pd
import numpy as np

###-------------------GET MAYA Multicore Results--------------------------###

phrases_1 = []
for i in range (0, 8):
    phrases_1.append(f'CPU {i} cumulative IPC:')

ignored_files = ['605.mcf_s-782B_200M_200M.txt', '605.mcf_s-994B_200M_200M.txt', '605.mcf_s-1644B_200M_200M.txt', '649.fotonik3d_s-10881B_200M_200M.txt']

spec = ['600.perlbench_s-570B','602.gcc_s-734B','602.gcc_s-1850B','602.gcc_s-2226B','603.bwaves_s-891B','603.bwaves_s-1740B','603.bwaves_s-2609B','603.bwaves_s-2931B','605.mcf_s-472B','605.mcf_s-484B','605.mcf_s-665B','605.mcf_s-1152B','605.mcf_s-1536B','605.mcf_s-1554B','607.cactuBSSN_s-2421B','607.cactuBSSN_s-3477B','607.cactuBSSN_s-4004B','619.lbm_s-2676B','619.lbm_s-2677B','619.lbm_s-3766B','619.lbm_s-4268B','620.omnetpp_s-141B','620.omnetpp_s-874B','621.wrf_s-6673B','621.wrf_s-8065B','623.xalancbmk_s-10B','623.xalancbmk_s-165B','623.xalancbmk_s-202B','625.x264_s-20B','627.cam4_s-490B','628.pop2_s-17B','649.fotonik3d_s-1176B','649.fotonik3d_s-7084B','649.fotonik3d_s-8225B','654.roms_s-293B','654.roms_s-294B','654.roms_s-523B','654.roms_s-1007B','654.roms_s-1070B','654.roms_s-1390B','657.xz_s-2302B', '605.mcf_s-782B', '605.mcf_s-994B', '605.mcf_s-1644B', '649.fotonik3d_s-10881B']
gap = ['bc-0','bc-3','bc-5','bc-12','bfs-3','bfs-8','bfs-10','bfs-14','cc-5','cc-6','cc-13','cc-14','pr-3','pr-5','pr-10','pr-14','sssp-3','sssp-5','sssp-10','sssp-14']


def get_number_between_dash_and_B(line):
    # Extract the substring between "-" and "B"
    start_index = line.rfind('-') + 1
    end_index = line.rfind('B', )
    #print(line[start_index:end_index])
    return int(line[start_index:end_index])

def get_first_three_digits(line):
    # Extract the first 3 digits from the string
    return int(line[:3])

def get_number_between_dash_and_underscore(line):
    # Extract the substring between "-" and "_"
    start_index = line.find('-') + 1
    end_index = line.find('_', )
    # print(line[start_index:end_index])
    return int(line[start_index:end_index])

def get_first_two_digits(line):
    # Extract the first 3 digits from the string
    if (line[:2] == 'bc'):
        return 1
    elif (line[:2] == 'bfs'):
        return 2
    elif (line[:2] == 'cc'):
        return 3
    elif (line[:2] == 'pr'):
        return 4
    else:
        return 5


path_spec = "maya/original_results/maya_8core_12MB_spec"
path_gap = "maya/original_results/maya_8core_12MB_gap"
ipc_maya_multicore = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                for phrase in phrases_1:
                    count = 0
                    with open(lookup_dir+file, 'r') as f:
                        for line_number, line in enumerate(f, start=1):
                            # Check if the phrase is present in the line
                            if (phrase in line):

                                if(count == 1):
                                    ipc_List.append(float(line.split()[4]))
                                count += 1
                    f.close()
                
                ipc_maya_multicore[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
df_ipc_maya_multicore = pd.DataFrame(ipc_maya_multicore)
print(df_ipc_maya_multicore.transpose().to_string())



###-----------------------------------MAYA Single Core-------------------------------------###

path_spec = "maya/original_results/maya_1core_12MB_8slices_spec"
path_gap = "maya/original_results/maya_1core_12MB_8slices_gap"
ipc_maya_singlecore = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                for phrase in phrases_1:
                    
                    with open(lookup_dir+file, 'r') as f:
                        for line_number, line in enumerate(f, start=1):
                            # Check if the phrase is present in the line
                            if (phrase in line):
                                ipc_List.append(float(line.split()[4]))
                               
                                
                    f.close()
                
                ipc_maya_singlecore[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
    
df_ipc_maya_singlecore = pd.DataFrame(ipc_maya_singlecore, index = ['Single Core IPC'])
print(df_ipc_maya_singlecore.transpose().to_string())

weighted_speedup_maya = pd.concat([df_ipc_maya_multicore, df_ipc_maya_singlecore])
raw_ipc = df_ipc_maya_multicore.sum(axis=0)
raw_ipc_column = pd.DataFrame([raw_ipc], columns=weighted_speedup_maya.columns, index=['Raw IPC'])
weighted_speedup_maya = pd.concat([weighted_speedup_maya, raw_ipc_column])
weighted_speedup_value = weighted_speedup_maya.iloc[9].div(weighted_speedup_maya.iloc[8])
weighted_speedup_column = pd.DataFrame([weighted_speedup_value], columns=weighted_speedup_maya.columns, index=['Weighted speedup'])
weighted_speedup_maya = pd.concat([weighted_speedup_maya, weighted_speedup_column])

print(weighted_speedup_maya.transpose().to_string())
weighted_speedup_maya.transpose().to_csv("Homogeneous_weighted_speedup_MAYA.csv")


###-------------------GET MIRAGE Multicore Results--------------------------###

path_spec = "mirage/original_results/mirage_8core_16MB_spec"
path_gap = "mirage/original_results/mirage_8core_16MB_gap"
ipc_maya_multicore = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                for phrase in phrases_1:
                    count = 0
                    with open(lookup_dir+file, 'r') as f:
                        for line_number, line in enumerate(f, start=1):
                            # Check if the phrase is present in the line
                            if (phrase in line):

                                if(count == 1):
                                    ipc_List.append(float(line.split()[4]))
                                count += 1
                    f.close()
                
                ipc_maya_multicore[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
df_ipc_maya_multicore = pd.DataFrame(ipc_maya_multicore)
print(df_ipc_maya_multicore.transpose().to_string())



###-----------------------------------MIRAGE Single Core-------------------------------------###

path_spec = "mirage/original_results/mirage_1core_16MB_8slices_spec"
path_gap = "mirage/original_results/mirage_1core_16MB_8slices_gap"
ipc_maya_singlecore = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                for phrase in phrases_1:
                    
                    with open(lookup_dir+file, 'r') as f:
                        for line_number, line in enumerate(f, start=1):
                            # Check if the phrase is present in the line
                            if (phrase in line):
                                ipc_List.append(float(line.split()[4]))
                               
                                
                    f.close()
                
                ipc_maya_singlecore[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
    
df_ipc_maya_singlecore = pd.DataFrame(ipc_maya_singlecore, index = ['Single Core IPC'])
print(df_ipc_maya_singlecore.transpose().to_string())

weighted_speedup_maya = pd.concat([df_ipc_maya_multicore, df_ipc_maya_singlecore])
raw_ipc = df_ipc_maya_multicore.sum(axis=0)
raw_ipc_column = pd.DataFrame([raw_ipc], columns=weighted_speedup_maya.columns, index=['Raw IPC'])
weighted_speedup_maya = pd.concat([weighted_speedup_maya, raw_ipc_column])
weighted_speedup_value = weighted_speedup_maya.iloc[9].div(weighted_speedup_maya.iloc[8])
weighted_speedup_column = pd.DataFrame([weighted_speedup_value], columns=weighted_speedup_maya.columns, index=['Weighted speedup'])
weighted_speedup_maya = pd.concat([weighted_speedup_maya, weighted_speedup_column])

print(weighted_speedup_maya.transpose().to_string())
weighted_speedup_maya.transpose().to_csv("Homogeneous_weighted_speedup_MIRAGE.csv")




###-------------------GET BASELINE Multicore Results--------------------------###

path_spec = "mirage/original_results/baseline_8core_16MB_spec"
path_gap = "mirage/original_results/baseline_8core_16MB_gap"
ipc_maya_multicore = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                for phrase in phrases_1:
                    count = 0
                    with open(lookup_dir+file, 'r') as f:
                        for line_number, line in enumerate(f, start=1):
                            # Check if the phrase is present in the line
                            if (phrase in line):

                                if(count == 1):
                                    ipc_List.append(float(line.split()[4]))
                                count += 1
                    f.close()
                
                ipc_maya_multicore[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
df_ipc_maya_multicore = pd.DataFrame(ipc_maya_multicore)
print(df_ipc_maya_multicore.transpose().to_string())



###-----------------------------------BASELINE Single Core-------------------------------------###

path_spec = "mirage/original_results/baseline_1core_16MB_8slices_spec"
path_gap = "mirage/original_results/baseline_1core_16MB_8slices_gap"
ipc_maya_singlecore = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                for phrase in phrases_1:
                    
                    with open(lookup_dir+file, 'r') as f:
                        for line_number, line in enumerate(f, start=1):
                            # Check if the phrase is present in the line
                            if (phrase in line):
                                ipc_List.append(float(line.split()[4]))
                               
                                
                    f.close()
                
                ipc_maya_singlecore[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
    
df_ipc_maya_singlecore = pd.DataFrame(ipc_maya_singlecore, index = ['Single Core IPC'])
print(df_ipc_maya_singlecore.transpose().to_string())

weighted_speedup_maya = pd.concat([df_ipc_maya_multicore, df_ipc_maya_singlecore])
raw_ipc = df_ipc_maya_multicore.sum(axis=0)
raw_ipc_column = pd.DataFrame([raw_ipc], columns=weighted_speedup_maya.columns, index=['Raw IPC'])
weighted_speedup_maya = pd.concat([weighted_speedup_maya, raw_ipc_column])
weighted_speedup_value = weighted_speedup_maya.iloc[9].div(weighted_speedup_maya.iloc[8])
weighted_speedup_column = pd.DataFrame([weighted_speedup_value], columns=weighted_speedup_maya.columns, index=['Weighted speedup'])
weighted_speedup_maya = pd.concat([weighted_speedup_maya, weighted_speedup_column])

print(weighted_speedup_maya.transpose().to_string())
weighted_speedup_maya.transpose().to_csv("Homogeneous_weighted_speedup_BASELINE.csv")



###---------------------Get Deadblocks--------------------------------------------###############


path_spec = "mirage/original_results/baseline_1core_2MB_spec"
path_gap = "mirage/original_results/baseline_1core_2MB_gap"
base_deadblocks = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                
                count = 0
                with open(lookup_dir+file, 'r') as f:
                    for line_number, line in enumerate(f, start=1):
                        # Check if the phrase is present in the line
                        if ("Deadblock percentage" in line):
                            ipc_List.append(float(line.split()[6]))

                f.close()
                
                base_deadblocks[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
df_base_deadblocks = pd.DataFrame(base_deadblocks, index = ['Baseline'])
print(df_base_deadblocks.transpose().to_string())



###-----------------------------Get Mirage Deadblocks-----------------------------###

path_spec = "mirage/original_results/mirage_1core_2MB_spec"
path_gap = "mirage/original_results/mirage_1core_2MB_gap"
mirage_deadblocks = {}
List_spec = os.listdir(path_spec)
List_gap = os.listdir(path_gap)

List_spec.sort()
List_gap.sort()


List_mod = List_spec.copy()
for files in List_mod:
    if(str(files[:-14]) in spec):
        pass
    else:
        List_spec.remove(files)
print(len(List_spec))


List_mod = List_gap.copy()
for files in List_mod:
    if(str(files[:-14]) in gap):
        pass
    else:
        List_gap.remove(files)
print(len(List_gap))


sorted_data = sorted(List_spec, key=get_number_between_dash_and_B)
sorted_list_spec = sorted(sorted_data, key=get_first_three_digits)

sorted_data = sorted(List_gap, key=get_number_between_dash_and_underscore)
sorted_list_gap = sorted(sorted_data, key=get_first_two_digits)


cwd = os.getcwd()
lookup_dir = cwd + "/" +path_spec + "/" 
for lists in [sorted_list_spec, sorted_list_gap]:

    for file in lists:
        # os.chdir(cwd + "/" + path)
        ipc_List = []
        try:
            if (file in ignored_files):
                pass
            else:
                
                count = 0
                with open(lookup_dir+file, 'r') as f:
                    for line_number, line in enumerate(f, start=1):
                        # Check if the phrase is present in the line
                        if ("Deadblock percentage" in line):
                            ipc_List.append(float(line.split()[6]))

                f.close()
                
                mirage_deadblocks[file[:file.rfind('_200M_200', )]] = ipc_List


        # Handle exceptions if the file doesn't exist or there's an error in reading it
        except FileNotFoundError:
            print("File not found.")
        except IOError:
            print("Error while reading the file.")
    lookup_dir = cwd + "/" +path_gap + "/" 
# print(ipc)
df_mirage_deadblocks = pd.DataFrame(mirage_deadblocks, index=["Mirage"])


df_deadblocks = pd.concat([df_base_deadblocks, df_mirage_deadblocks])
print(df_deadblocks.transpose().to_string())
df_deadblocks.transpose().to_csv("Deadblocks.csv")


