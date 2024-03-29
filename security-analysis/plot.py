import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns; sns.set()
import math
from matplotlib.ticker import PercentFormatter
import matplotlib
import matplotlib.cm as cm
matplotlib.ticker._mathdefault = lambda x: '\\mathdefault{%s}'%x

####### Figure - 7 #########

#Set Font Size
plt.rc('font', size='12')
plt.rc('axes',edgecolor='black')

df = pd.read_csv("results/maya6ways.Spills.stat", delimiter='\s+')

# Add Trials/Spill for Extra-Ways=0
id=len(df)+1
df.at[id, 'ExtraWaysPerSkew'] = 0
df.at[id, 'Trials/Spill'] = 1


cmap = cm.get_cmap('Greys', 100)
fig = plt.figure(figsize=(5.5,2.5))
plt.bar(df['ExtraWaysPerSkew']+9, df['Trials/Spill'], align='center', width = 0.5, color=cmap(40), edgecolor = "black")

ax = plt.gca()

#xticks
plt.xticks(df['ExtraWaysPerSkew']+9)
ax.set_xlim(8.5,15.5)

#axes labels
plt.xlabel('Bucket Capacity')
plt.ylabel('Iterations Per Spill')

#Set Y format & limts
ax.set_yscale('log')
ax.set_ylim(0.2,9*10**8)

#grid
ax.axis('on')
ax.yaxis.grid(color="grey")
ax.set_facecolor("white")

#Text
ax.text(14.35, 15, 'no spills observed\n', style='italic',
        bbox={ 'facecolor': 'white','pad': 2}, family='serif',size='8',
        ha='center', va='center')

fig.savefig("Fig7.pdf",bbox_inches='tight')


####### Figure - 8 #########

Pr_df = pd.read_csv("results/maya6ways.BucketProb.stat", delimiter='\s+')
p_0 = Pr_df['Pr_obs(N)'][0]
Pr_df.at[1,'Pr_est(N)'] = 9*(p_0**2 + 2*p_0 - 2*p_0*p_0)

p_leqN = p_0 + Pr_df.at[1,'Pr_est(N)']

## Using Eq-5
for N in range(1,12):
    p_N = Pr_df.at[N,'Pr_est(N)']
    Pr_df.at[N+1,'Pr_est(N)'] = 9/(N+1)*(p_N**2 + 2*p_N - 2*p_N*p_leqN)
    p_leqN = p_leqN + Pr_df.at[N+1,'Pr_est(N)']
## Using Eq-6
for N in range(12,16):
    p_N = Pr_df.at[N,'Pr_est(N)']
    Pr_df.at[N+1,'Pr_est(N)'] = 9/(N+1)*(p_N**2)


#Set Font Size
plt.rc('font', size='12')
plt.rc('axes',edgecolor='black')

fig = plt.figure(figsize=(5.5,2.5))
ax1 = plt.gca()

#Plot
ax1 = Pr_df.set_index('BallsInBucket-N')['Pr_est(N)'].plot(label='$Estimated$',color='r',linestyle="--",linewidth=2, marker="o", markersize=6.5)
ax1 = Pr_df.set_index('BallsInBucket-N')['Pr_obs(N)'].plot(ax=ax1,label='$Experimental$',color='b', marker="*", markersize=10, linewidth=0)

#Get Axes
ax1 = plt.gca()

#legend
ax1.legend(facecolor="white")


#xticks
ax1.set_xlim(-1,17)

#axes labels
plt.xlabel('Number of Balls (N) in a Bucket')
plt.ylabel('Pr(Bucket with N balls)')

#Set Y format & limts
ax1.set_yscale('log')
ax1.set_ylim(10**-36,100)
ax1.set_yticks([1,10**-5,10**-10,10**-15,10**-20,10**-25,10**-30,10**-35])

#grid
ax1.axis('on')
ax1.yaxis.grid(color="grey")
ax1.set_facecolor("white")

fig.savefig("Fig8.pdf",bbox_inches='tight',dpi=300)

