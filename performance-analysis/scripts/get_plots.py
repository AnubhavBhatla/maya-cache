import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import math
from matplotlib.ticker import PercentFormatter
import matplotlib.cm as cm
from scipy.stats import gmean



# try:
baseline_data = pd.read_csv("Homogeneous_weighted_speedup_BASELINE.csv")
mirage_data = pd.read_csv("Homogeneous_weighted_speedup_MIRAGE.csv")
maya_data = pd.read_csv("Homogeneous_weighted_speedup_MAYA.csv")


maya_data.columns.values[0] = 'Benchmarks'
mirage_data.columns.values[0] = 'Benchmarks'
baseline_data.columns.values[0] = 'Benchmarks'


gap_list = ['bc', 'bfs', 'cc', 'pr', 'sssp']

maya_norm = maya_data["Weighted speedup"]/baseline_data["Weighted speedup"]
mirage_norm = mirage_data["Weighted speedup"]/baseline_data["Weighted speedup"]

comp_maya_mirage = pd.DataFrame([maya_data['Benchmarks'],maya_norm, mirage_norm], index=['Benchmarks', 'Maya', 'Mirage']).transpose()
#print(comp_maya_mirage.to_string())
comp_maya_mirage.replace([np.inf, -np.inf],np.nan, inplace=True)
comp_maya_mirage.dropna(how="any", inplace=True)
comp_maya_mirage.to_csv("comp_maya_mirage.csv")
last_bench = 'na'

gmean_comp = {"Benchmarks" : [],
            'Mirage' : [],
            "Maya" : []
            }


count = 0
gmean_list = [[], []]
for i in comp_maya_mirage['Benchmarks']:
    
    start_index = i.find('.') + 1 
    end_index = i.find('_' ) if i.find('_') > 1 else i.find('-')
    bench = i[start_index:end_index]
    
    if bench != last_bench:
        # gmean_comp['Benchmarks'].append(bench)
        
        
        
            # last_bench = bench
        
        if(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Maya'].iloc[0] == 0 or comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Mirage'].iloc[0] == 0):
            # last_bench = bench
            
            pass
        else:
            if count > 0:
            
                gmean_comp['Maya'].append(gmean(gmean_list[0]))
                gmean_comp['Mirage'].append(gmean(gmean_list[1]))
                gmean_list = [[], []]
            gmean_comp['Benchmarks'].append(bench)
            gmean_list[0].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Maya'].iloc[0])
            gmean_list[1].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Mirage'].iloc[0])
            last_bench = bench

        count += 1
        
        
    else:
        if(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Maya'].iloc[0] == 0 or comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Mirage'].iloc[0] == 0):
            pass
        else:
            gmean_list[0].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Maya'].iloc[0])
            gmean_list[1].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Mirage'].iloc[0])
        last_bench = bench
    # last_bench = bench

gmean_comp['Maya'].append(gmean(gmean_list[0]))
gmean_comp['Mirage'].append(gmean(gmean_list[1]))
# print(len(gmean_comp['Benchmarks']))
# print((gmean_comp['Benchmarks']))
# print(len(gmean_comp['Mirage']))
# print((gmean_comp['Mirage']))

# print(len(gmean_comp['Maya']))
# print((gmean_comp['Maya']))

df_gmean_comp = pd.DataFrame(gmean_comp)




gmean_comp['Benchmarks'].append("Geomean-SPEC")
gmean_comp['Maya'].append(gmean(df_gmean_comp[~df_gmean_comp['Benchmarks'].isin( gap_list)][ 'Maya']))
gmean_comp['Mirage'].append(gmean(df_gmean_comp[~df_gmean_comp['Benchmarks'].isin( gap_list)][ 'Mirage']))

gmean_comp['Benchmarks'].append("Geomean-GAP")
gmean_comp['Maya'].append(gmean(df_gmean_comp[df_gmean_comp['Benchmarks'].isin( gap_list)][ 'Maya']))
gmean_comp['Mirage'].append(gmean(df_gmean_comp[df_gmean_comp['Benchmarks'].isin( gap_list)][ 'Mirage']))

gmean_comp['Benchmarks'].append("Geomean-ALL")
gmean_comp['Maya'].append(gmean(df_gmean_comp['Maya']))
gmean_comp['Mirage'].append(gmean(df_gmean_comp['Mirage']))



df_gmean_comp = pd.DataFrame(gmean_comp)
df_gmean_comp.to_csv("gmean_comp.csv")
# except:
#     print("Multicore Simulation not Finished")
#     pass


try:
    deadblocks_data = pd.read_csv("Deadblocks.csv")
    deadblocks_data.columns.values[0] = 'Benchmarks'
    avg_deadblock = {"Benchmarks" : [],
                'Baseline' : [],
                "Mirage" : []
                }
    last_bench = 'na'
    count = 0
    mean_list = [[], []]
    for i in deadblocks_data['Benchmarks']:
        
        start_index = i.find('.') + 1 
        end_index = i.find('_' ) if i.find('_') > 1 else i.find('-')
        bench = i[start_index:end_index]
        
        if bench != last_bench:
            avg_deadblock['Benchmarks'].append(bench)
            
            
            if count > 0:
                
                avg_deadblock['Baseline'].append(np.mean(mean_list[0]))
                avg_deadblock['Mirage'].append(np.mean(mean_list[1]))
                mean_list = [[], []]
            if(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Baseline'].iloc[0] == 0 or deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Mirage'].iloc[0] == 0 ):
                pass
            else:
                mean_list[0].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Baseline'].iloc[0])
                mean_list[1].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Mirage'].iloc[0])

            count += 1
            
            
        else:
            if(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Baseline'].iloc[0] == 0 or deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Mirage'].iloc[0] == 0 ):
                pass
            else:
                mean_list[0].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Baseline'].iloc[0])
                mean_list[1].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Mirage'].iloc[0])
        last_bench = bench

    avg_deadblock['Baseline'].append(np.mean(mean_list[0]))
    avg_deadblock['Mirage'].append(np.mean(mean_list[1]))

    avg_deadblock['Benchmarks'].append("AVERAGE")
    avg_deadblock['Mirage'].append(np.mean(avg_deadblock['Mirage']))
    avg_deadblock['Baseline'].append(np.mean(avg_deadblock['Baseline']))

except:
    print("Deadblock simulations not finished")
    pass



# Melt the DataFrame to have a single value column and a 'variable' column
# df_melted = pd.melt(pd.DataFrame(gmean_comp), id_vars=['Benchmarks'], var_name='Variable', value_name='Value')

# Plot side-by-side bar plot
plt.figure(figsize=(16, 7))
# sns.set_style('white')
# sns.set_palette('gray_r')
# sns.barplot(data=df_melted, x='Benchmarks', y='Value', hue='Variable')

width = 0.3       
cmap = cm.get_cmap('Greys', 100)
# Plotting
try:
    
    ind = np.arange(len(avg_deadblock['Benchmarks']))
    _ = plt.bar(ind , avg_deadblock['Baseline'] , width, label='Baseline', color = cmap(40), edgecolor = "black")
    _ = plt.bar(ind + width, avg_deadblock['Mirage'], width, label='Mirage', color = cmap(75), edgecolor = "black")
    plt.xticks(ind + width / 2, avg_deadblock['Benchmarks'])
    # plt.title('Deadblockks')
    # plt.xlabel('Benchmarks')
    plt.ylabel('% of dead blocks', fontsize = 15)
    # plt.tight_layout()
    plt.ylim(0, 115)
    plt.legend(loc='upper left', ncol=2, fontsize = 15)
    plt.xticks(rotation=45, horizontalalignment='right')
    plt.grid(axis='x')
    plt.subplots_adjust(bottom=0.15)
    plt.savefig('Fig1.pdf', format='pdf')
except:
    pass


try:
    plt.clf()
    ind = np.arange(len(gmean_comp['Benchmarks']))
    _ = plt.bar(ind , gmean_comp['Mirage'], width, label='Mirage', color = cmap(40), edgecolor = "black")
    _ = plt.bar(ind + width, gmean_comp['Maya'] , width, label='Maya', color = cmap(75), edgecolor = "black")
    plt.xticks(ind + width / 2, gmean_comp['Benchmarks'])
    # plt.title('Performance')
    # plt.xlabel('Benchmarks')
    plt.ylabel('Normaized Performance', fontsize = 15)
    # plt.tight_layout()
    plt.ylim(.9, 1.15)
    plt.legend(loc='upper left', ncol=2, fontsize = 15)
    plt.xticks(rotation=45, horizontalalignment='right')
    plt.grid(axis='x')
    plt.subplots_adjust(bottom=0.16)
    plt.savefig('Fig10.pdf', format='pdf')
except:
    pass


#plt.show()
