import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns; sns.set()
import math
from matplotlib.ticker import PercentFormatter

from scipy.stats import gmean


baseline_data = pd.read_csv("Homogeneous_weighted_speedup_BASELINE.csv")
mirage_data = pd.read_csv("Homogeneous_weighted_speedup_MIRAGE.csv")
maya_data = pd.read_csv("Homogeneous_weighted_speedup_MAYA.csv")

maya_data.columns.values[0] = 'Benchmarks'
mirage_data.columns.values[0] = 'Benchmarks'
baseline_data.columns.values[0] = 'Benchmarks'






maya_norm = maya_data["Weighted speedup"]/baseline_data["Weighted speedup"]
mirage_norm = mirage_data["Weighted speedup"]/baseline_data["Weighted speedup"]

comp_maya_mirage = pd.DataFrame([maya_data['Benchmarks'],maya_norm, mirage_norm], index=['Benchmarks', 'Maya', 'Mirage']).transpose()
print(comp_maya_mirage.to_string())

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
df_gmean_comp.to_csv("gmean_comp.csv")
print(df_gmean_comp)

# Melt the DataFrame to have a single value column and a 'variable' column
df_melted = pd.melt(pd.DataFrame(gmean_comp), id_vars=['Benchmarks'], var_name='Variable', value_name='Value')

# Plot side-by-side bar plot
plt.figure(figsize=(10, 6))
# sns.set_style('white')
# sns.set_palette('gray_r')
# sns.barplot(data=df_melted, x='Benchmarks', y='Value', hue='Variable')

width = 0.3       

# Plotting
ind = np.arange(len(df_gmean_comp))
plt.bar(ind + width, gmean_comp['Maya'] , width, label='Maya')
plt.bar(ind , gmean_comp['Mirage'], width, label='Mirage')
plt.xticks(ind + width / 2, gmean_comp['Benchmarks'])
plt.title('Side-by-Side Bar Plot')
plt.xlabel('Benchmarks')
plt.ylabel('Values')
plt.tight_layout()
plt.ylim(.87, 1.2)
plt.xticks(rotation=45)
plt.grid(axis='x')

plt.show()