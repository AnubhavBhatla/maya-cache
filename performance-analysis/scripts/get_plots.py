import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns; sns.set()
import math
from matplotlib.ticker import PercentFormatter
import matplotlib.cm as cm
from scipy.stats import gmean


baseline_data = pd.read_csv("Homogeneous_weighted_speedup_BASELINE.csv")
mirage_data = pd.read_csv("Homogeneous_weighted_speedup_MIRAGE.csv")
maya_data = pd.read_csv("Homogeneous_weighted_speedup_MAYA.csv")
deadblocks_data = pd.read_csv("Deadblocks.csv")

maya_data.columns.values[0] = 'Benchmarks'
mirage_data.columns.values[0] = 'Benchmarks'
baseline_data.columns.values[0] = 'Benchmarks'
deadblocks_data.columns.values[0] = 'Benchmarks'

gap_list = ['bc', 'bfs', 'cc', 'pr', 'sssp']

maya_norm = maya_data["Weighted speedup"]/baseline_data["Weighted speedup"]
mirage_norm = mirage_data["Weighted speedup"]/baseline_data["Weighted speedup"]

comp_maya_mirage = pd.DataFrame([maya_data['Benchmarks'],maya_norm, mirage_norm], index=['Benchmarks', 'Maya', 'Mirage']).transpose()
#print(comp_maya_mirage.to_string())

last_bench = 'na'

gmean_comp = {"Benchmarks" : [],
              'Mirage' : [],
              "Maya" : []
              }

avg_deadblock = {"Benchmarks" : [],
              'Baseline' : [],
              "Mirage" : []
              }

count = 0
gmean_list = [[], []]
for i in comp_maya_mirage['Benchmarks']:
    
    start_index = i.find('.') + 1 
    end_index = i.find('_' ) if i.find('_') > 1 else i.find('-')
    bench = i[start_index:end_index]
    
    if bench != last_bench:
        gmean_comp['Benchmarks'].append(bench)
        
        
        if count > 0:
            
            gmean_comp['Maya'].append(gmean(gmean_list[0]))
            gmean_comp['Mirage'].append(gmean(gmean_list[1]))
            gmean_list = [[], []]
        gmean_list[0].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Maya'].iloc[0])
        gmean_list[1].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Mirage'].iloc[0])

        count += 1
        
        
    else:
        gmean_list[0].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Maya'].iloc[0])
        gmean_list[1].append(comp_maya_mirage.loc[comp_maya_mirage['Benchmarks'] == i, 'Mirage'].iloc[0])
    last_bench = bench

gmean_comp['Maya'].append(gmean(gmean_list[0]))
gmean_comp['Mirage'].append(gmean(gmean_list[1]))

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
        mean_list[0].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Baseline'].iloc[0])
        mean_list[1].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Mirage'].iloc[0])

        count += 1
        
        
    else:
        mean_list[0].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Baseline'].iloc[0])
        mean_list[1].append(deadblocks_data.loc[deadblocks_data['Benchmarks'] == i, 'Mirage'].iloc[0])
    last_bench = bench

avg_deadblock['Baseline'].append(np.mean(mean_list[0]))
avg_deadblock['Mirage'].append(np.mean(mean_list[1]))

avg_deadblock['Benchmarks'].append("AVERAGE")
avg_deadblock['Mirage'].append(np.mean(avg_deadblock['Mirage']))
avg_deadblock['Baseline'].append(np.mean(avg_deadblock['Baseline']))




# Melt the DataFrame to have a single value column and a 'variable' column
df_melted = pd.melt(pd.DataFrame(gmean_comp), id_vars=['Benchmarks'], var_name='Variable', value_name='Value')

# Plot side-by-side bar plot
plt.figure(figsize=(16, 7))
# sns.set_style('white')
# sns.set_palette('gray_r')
# sns.barplot(data=df_melted, x='Benchmarks', y='Value', hue='Variable')

width = 0.3       

# Plotting
cmap = cm.get_cmap('Greys', 100)
ind = np.arange(len(avg_deadblock['Benchmarks']))
_ = plt.bar(ind , avg_deadblock['Baseline'] , width, label='Baseline', color = cmap(40), edgecolor = "black")
_ = plt.bar(ind + width, avg_deadblock['Mirage'], width, label='Mirage', color = cmap(75), edgecolor = "black")
plt.xticks(ind + width / 2, avg_deadblock['Benchmarks'])
# plt.title('Deadblockks')
# plt.xlabel('Benchmarks')
plt.ylabel('% of dead blocks', fontsize = 15)
# plt.tight_layout()
plt.legend()
plt.xticks(rotation=45, horizontalalignment='right')
plt.grid(axis='x')

plt.savefig('Fig1.pdf', format='pdf')


plt.clf()
ind = np.arange(len(gmean_comp['Benchmarks']))
_ = plt.bar(ind + width, gmean_comp['Maya'] , width, label='Maya', color = cmap(75), edgecolor = "black")
_ = plt.bar(ind , gmean_comp['Mirage'], width, label='Mirage', color = cmap(40), edgecolor = "black")
plt.xticks(ind + width / 2, gmean_comp['Benchmarks'])
# plt.title('Performance')
# plt.xlabel('Benchmarks')
plt.ylabel('Normaized Performance', fontsize = 15)
# plt.tight_layout()
plt.ylim(.87, 1.2)
plt.legend()
plt.xticks(rotation=45, horizontalalignment='right')
plt.grid(axis='x')

plt.savefig('Fig10.pdf', format='pdf')

#plt.show()
