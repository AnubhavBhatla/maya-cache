import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns; sns.set()
import math
from matplotlib.ticker import PercentFormatter
import matplotlib
matplotlib.ticker._mathdefault = lambda x: '\\mathdefault{%s}'%x


#Set Font Size
#plt.rc('font', family='serif')
plt.rc('font', size='12')
plt.rc('axes',edgecolor='black')

df = pd.read_csv("results/base/maya6ways.Spills.stat", delimiter='\s+')

# Add Trials/Spill for Extra-Ways=0
id=len(df)+1
df.at[id, 'ExtraWaysPerSkew'] = 0
df.at[id, 'Trials/Spill'] = 1


fig = plt.figure(figsize=(5.5,2.5))
plt.bar(df['ExtraWaysPerSkew']+9, df['Trials/Spill'], align='center', width = 0.5, color='grey')

ax = plt.gca()

#xticks
plt.xticks(df['ExtraWaysPerSkew']+9)
ax.set_xlim(8.5,15.5)

#axes labels
plt.xlabel('Bucket Capacity')
plt.ylabel('Iterations Per Spill')

#Set Y format & limts
ax.set_yscale('log')
ax.set_ylim(0.2,3*10**10)

#grid
ax.axis('on')
#ax.legend(facecolor="white")
ax.yaxis.grid(color="light-grey")
ax.set_facecolor("white")

#Text
ax.text(14.35, 15, 'no spills observed\n', style='italic',
        bbox={ 'facecolor': 'white','pad': 2}, family='serif',size='8',
        ha='center', va='center')

fig.savefig("Fig7.pdf",bbox_inches='tight')


# ## Bucket Probabilities - Analytical  (Fig-9)

##### PROCESS ANALYTICAL RESULTS ########

Pr_df = pd.read_csv("results/base/maya6ways.BucketProb.stat", delimiter='\s+')
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
   
#print("------------------------------------------------")
#print (Pr_df)
#print("------------------------------------------------")

##### PLOT RESULTS #########

#Set Font Size
#plt.rc('font', family='serif')
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
#legend_properties={'size':14}
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


# ## Frequency of Bucket Spills - Analytical (Fig-10)

##### PROCESS RESULTS ########
# Read Empirical Results
df1 = pd.read_csv("results/base/maya6ways.Spills.stat", delimiter='\s+')
#print (df1)
Spill_df = pd.DataFrame(columns=['Bucket_Capacity','SpillTries_Obs','SpillTries_Est'])
for i in range(0,4):
    Spill_df.at[i,'Bucket_Capacity'] = df1.at[i,'ExtraWaysPerSkew'] + 9
    Spill_df.at[i,'SpillTries_Obs'] = df1.at[i,'Trials/Spill']
    
Spill_df.at[4,'Bucket_Capacity'] = Spill_df.at[3,'Bucket_Capacity'] + 1
Spill_df.at[5,'Bucket_Capacity'] = Spill_df.at[4,'Bucket_Capacity'] + 1

# Calculation of Estimated Spills from Analytical Probabilities.
for i in range(0,6): 
    Spill_df.at[i,'SpillTries_Est'] = 1/Pr_df.at[Spill_df.at[i,'Bucket_Capacity'],'Pr_est(N)']**2
#print(Spill_df)


## Function to Format Time:
def format_time (x):
    digits = int(math.log10(x))+1
    if(digits <= 3):
        return str(int(round(x)))+" ns"
    x_us = x/1000
    digits = int(math.log10(x_us))+1
    if(digits <= 3):
        return str(int(x_us)) + " us"
    x_ms = x_us/1000
    digits = int(math.log10(x_ms))+1
    if(digits <= 3):
        return  str(int(x_ms)) + " ms"
    x_s = x_ms/1000
    digits = int(math.log10(x_s))+1
    if(digits <3):
        return  str(int(x_s))+ " s"
    x_min = x_s/60
    digits = int(math.log10(x_min))+1
    if(digits <3):
        return  str(int(x_min)) + " min"
    x_hr = x_min/60
    digits = int(math.log10(x_hr))+1
    if(digits <3):
        return  str(int((x_hr)))+ " hr"
    x_day = x_hr/24
    digits = int(math.log10(x_day))+1
    if(digits <3):
        return  str(int(x_day))+ " days"
    x_yr = x_day/365
    return  np.format_float_scientific(x_yr,precision=1, exp_digits=2)+ " yrs"    


## Create DataFrame
FreqSAE_df = pd.DataFrame(columns=['Ways in each Skew (Base + Extra)','Installs Per SAE','Time Per SAE'])

## Ways = 9 + 0
FreqSAE_df.at[0,'Ways in each Skew (Base + Extra)'] = "9 + 0"
FreqSAE_df.at[0,'Installs Per SAE'] = 1

## Ways = "9 + 1" to "9 + 4": Copy from Empirically Observed Balls/Spill_obs in above graph
for i in range(1,5):
    FreqSAE_df.at[i,'Ways in each Skew (Base + Extra)'] = "9 + "+str(i)
    FreqSAE_df.at[i,'Installs Per SAE'] = Spill_df.at[i-1,'SpillTries_Obs']

## Ways = "9 + 5" to "9 + 6": Copy from Analytically Calculated Balls/Spill_obs in above graph
for i in range(5,7):
    FreqSAE_df.at[i,'Ways in each Skew (Base + Extra)'] = "9 + "+str(i)
    FreqSAE_df.at[i,'Installs Per SAE'] = Spill_df.at[i-1,'SpillTries_Est']

## Update the label of the last entry "9 + 6 (default)"
FreqSAE_df.at[6,'Ways in each Skew (Base + Extra)'] = FreqSAE_df.at[6,'Ways in each Skew (Base + Extra)']  + " (default)"

## Calculate Time Per SAE
FreqSAE_df['Time Per SAE'] = FreqSAE_df['Installs Per SAE'] * 1
FreqSAE_df['Time Per SAE'] = FreqSAE_df['Time Per SAE'].apply(lambda x: format_time(x))

## Round numbers
FreqSAE_df.round({'Installs Per SAE': 1}) 
FreqSAE_df['Installs Per SAE'] = FreqSAE_df['Installs Per SAE'].apply(lambda x: np.format_float_scientific(x,precision=0, exp_digits=2))

## Print Table
print("-----------------------------------------------------------------")
print(FreqSAE_df)
print("-----------------------------------------------------------------")


# ## Sensitivity of SAE to LLC-Associativity (Table-4)

######## CALCULATE SAE USING ANALYTICAL MODEL FOR LLC-ASSOCIATIVITY = 8,36 ########


##### LLC-ASSOC = 36 ####
Pr36way_df = pd.read_csv("results/sensitivity/maya12ways.BucketProb.stat", delimiter='\s+')
ways_per_skew = 18

## Setting initial conditions
p_2 = Pr36way_df['Pr_obs(N)'][2]
Pr36way_df.at[3,'Pr_est(N)'] = ways_per_skew/(2+1)*(p_2**2 + 2*p_2 - 2*p_2*(p_2))
p_leqN = p_2 + Pr36way_df.at[3,'Pr_est(N)']

## Bucket-Probabilities Using Eq-5
for N in range(3,23):
    p_N = Pr36way_df.at[N,'Pr_est(N)']
    Pr36way_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2 + 2*p_N - 2*p_N*p_leqN)
    p_leqN = p_leqN + Pr36way_df.at[N+1,'Pr_est(N)']
## Bucket-Probabilities Using Eq-6
for N in range(23,37):
    p_N = Pr36way_df.at[N,'Pr_est(N)']
    Pr36way_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2)   
#print (Pr36way_df)

## Calculate Spills using Eq-7
Spill36way_df = pd.DataFrame(columns=['Bucket_Capacity','SpillTries_Est'])
for i in range(0,6): 
    Spill36way_df.at[i,'Bucket_Capacity'] = i + 1 + ways_per_skew
    Spill36way_df.at[i,'SpillTries_Est'] = 1/Pr36way_df.at[Spill36way_df.at[i,'Bucket_Capacity'],'Pr_est(N)']**2
#print(Spill36way_df)

##### LLC-ASSOC = 8 ####
Pr8way_df = pd.read_csv("results/sensitivity/maya3ways.BucketProb.stat", delimiter='\s+')
ways_per_skew = 4

## Setting initial conditions
p_0 = Pr8way_df['Pr_obs(N)'][0]
Pr8way_df.at[1,'Pr_est(N)'] = ways_per_skew/(0+1)*(p_0**2 + 2*p_0 - 2*p_0*p_0)

p_leqN = p_0+ Pr8way_df.at[1,'Pr_est(N)']

## Bucket-Probabilities Using Eq-5
for N in range(1,6):
    p_N = Pr8way_df.at[N,'Pr_est(N)']
    Pr8way_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2 + 2*p_N - 2*p_N*p_leqN)
    p_leqN = p_leqN + Pr8way_df.at[N+1,'Pr_est(N)']
## Bucket-Probabilities Using Eq-6
for N in range(6,16):
    p_N = Pr8way_df.at[N,'Pr_est(N)']
    Pr8way_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2)   
#print (Pr8way_df)

## Calculate Spills using Eq-7
Spill8way_df = pd.DataFrame(columns=['Bucket_Capacity','SpillTries_Est'])
for i in range(0,6): 
    Spill8way_df.at[i,'Bucket_Capacity'] = i + 1 + ways_per_skew
    Spill8way_df.at[i,'SpillTries_Est'] = 1/Pr8way_df.at[Spill8way_df.at[i,'Bucket_Capacity'],'Pr_est(N)']**2
#print(Spill8way_df)

######## CREATE TABLE FOR LLC-ASSOC = 8,18,36 ########

## Create DataFrame
SensFreqSAE_df = pd.DataFrame(columns=['Over-Provisioning','LLC-Assoc 8-ways','LLC-Assoc 18-ways','LLC-Assoc 36-ways'])
#1 extra way/skew
extra_ways_per_skew = [4,5,6]
for i in range(len(extra_ways_per_skew)):
    SensFreqSAE_df.at[i,'Over-Provisioning'] = str(extra_ways_per_skew[i])+' extra way/skew'
    SensFreqSAE_df.at[i,'LLC-Assoc 8-ways'] = Spill8way_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    SensFreqSAE_df.at[i,'LLC-Assoc 18-ways'] = Spill_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    SensFreqSAE_df.at[i,'LLC-Assoc 36-ways'] = Spill36way_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    
## Format Table:
SensFreqSAE_df['LLC-Assoc 8-ways'] = SensFreqSAE_df['LLC-Assoc 8-ways'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))
SensFreqSAE_df['LLC-Assoc 18-ways'] = SensFreqSAE_df['LLC-Assoc 18-ways'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))
SensFreqSAE_df['LLC-Assoc 36-ways'] = SensFreqSAE_df['LLC-Assoc 36-ways'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))

print("-------------------------------------------------------------------------------------")
with pd.option_context('expand_frame_repr', False):
    print (SensFreqSAE_df)
print("-------------------------------------------------------------------------------------")

# In[6]:


######## CALCULATE SAE USING ANALYTICAL MODEL FOR REUSE = 1,5,7 ########

##### REUSE = 1 ####
Pr1reuse_df = pd.read_csv("results/sensitivity/maya6ways.Reuse1.BucketProb.stat", delimiter='\s+')
ways_per_skew = 7

## Setting initial conditions
p_2 = Pr1reuse_df['Pr_obs(N)'][2]
Pr1reuse_df.at[3,'Pr_est(N)'] = ways_per_skew/(2+1)*(p_2**2 + 2*p_2 - 2*p_2*(p_2))
p_leqN = p_2 + Pr1reuse_df.at[3,'Pr_est(N)']

## Bucket-Probabilities Using Eq-5
for N in range(3,12):
    p_N = Pr1reuse_df.at[N,'Pr_est(N)']
    Pr1reuse_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2 + 2*p_N - 2*p_N*p_leqN)
    p_leqN = p_leqN + Pr1reuse_df.at[N+1,'Pr_est(N)']
## Bucket-Probabilities Using Eq-6
for N in range(12,14):
    p_N = Pr1reuse_df.at[N,'Pr_est(N)']
    Pr1reuse_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2)   
#print (Pr1reuse_df)

## Calculate Spills using Eq-7
Spill1reuse_df = pd.DataFrame(columns=['Bucket_Capacity','SpillTries_Est'])
for i in range(0,6): 
    Spill1reuse_df.at[i,'Bucket_Capacity'] = i + 1 + ways_per_skew
    Spill1reuse_df.at[i,'SpillTries_Est'] = 1/Pr1reuse_df.at[Spill1reuse_df.at[i,'Bucket_Capacity'],'Pr_est(N)']**2
#print(Spill1reuse_df)

##### REUSE = 5 ####
Pr5reuse_df = pd.read_csv("results/sensitivity/maya6ways.Reuse5.BucketProb.stat", delimiter='\s+')
ways_per_skew = 11

## Setting initial conditions
p_2 = Pr5reuse_df['Pr_obs(N)'][2]
Pr5reuse_df.at[3,'Pr_est(N)'] = ways_per_skew/(2+1)*(p_2**2 + 2*p_2 - 2*p_2*(p_2))
p_leqN = p_2 + Pr5reuse_df.at[3,'Pr_est(N)']

## Bucket-Probabilities Using Eq-5
for N in range(3,16):
    p_N = Pr5reuse_df.at[N,'Pr_est(N)']
    Pr5reuse_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2 + 2*p_N - 2*p_N*p_leqN)
    p_leqN = p_leqN + Pr5reuse_df.at[N+1,'Pr_est(N)']
## Bucket-Probabilities Using Eq-6
for N in range(16,18):
    p_N = Pr5reuse_df.at[N,'Pr_est(N)']
    Pr5reuse_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2)   
#print (Pr5reuse_df)

## Calculate Spills using Eq-7
Spill5reuse_df = pd.DataFrame(columns=['Bucket_Capacity','SpillTries_Est'])
for i in range(0,6): 
    Spill5reuse_df.at[i,'Bucket_Capacity'] = i + 1 + ways_per_skew
    Spill5reuse_df.at[i,'SpillTries_Est'] = 1/Pr5reuse_df.at[Spill5reuse_df.at[i,'Bucket_Capacity'],'Pr_est(N)']**2
#print(Spill5reuse_df)

##### REUSE = 7 ####
Pr7reuse_df = pd.read_csv("results/sensitivity/maya6ways.Reuse7.BucketProb.stat", delimiter='\s+')
ways_per_skew = 13

## Setting initial conditions
p_2 = Pr7reuse_df['Pr_obs(N)'][2]
Pr7reuse_df.at[3,'Pr_est(N)'] = ways_per_skew/(2+1)*(p_2**2 + 2*p_2 - 2*p_2*(p_2))
p_leqN = p_2 + Pr7reuse_df.at[3,'Pr_est(N)']

## Bucket-Probabilities Using Eq-5
for N in range(3,18):
    p_N = Pr7reuse_df.at[N,'Pr_est(N)']
    Pr7reuse_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2 + 2*p_N - 2*p_N*p_leqN)
    p_leqN = p_leqN + Pr7reuse_df.at[N+1,'Pr_est(N)']
## Bucket-Probabilities Using Eq-6
for N in range(18,20):
    p_N = Pr7reuse_df.at[N,'Pr_est(N)']
    Pr7reuse_df.at[N+1,'Pr_est(N)'] = ways_per_skew/(N+1)*(p_N**2)   
#print (Pr7reuse_df)

## Calculate Spills using Eq-7
Spill7reuse_df = pd.DataFrame(columns=['Bucket_Capacity','SpillTries_Est'])
for i in range(0,6): 
    Spill7reuse_df.at[i,'Bucket_Capacity'] = i + 1 + ways_per_skew
    Spill7reuse_df.at[i,'SpillTries_Est'] = 1/Pr7reuse_df.at[Spill7reuse_df.at[i,'Bucket_Capacity'],'Pr_est(N)']**2
#print(Spill7reuse_df)

######## CREATE TABLE FOR REUSE = 1,3,5,7 ########

## Create DataFrame
SensFreqSAE_df = pd.DataFrame(columns=['Over-Provisioning','Reuse 1-way','Reuse 3-ways','Reuse 5-ways', 'Reuse 7-ways'])

extra_ways_per_skew = [5,6]
for i in range(len(extra_ways_per_skew)):
    SensFreqSAE_df.at[i,'Over-Provisioning'] = str(extra_ways_per_skew[i])+' extra way/skew'
    SensFreqSAE_df.at[i,'Reuse 1-way'] = Spill1reuse_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    SensFreqSAE_df.at[i,'Reuse 3-ways'] = Spill_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    SensFreqSAE_df.at[i,'Reuse 5-ways'] = Spill5reuse_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    SensFreqSAE_df.at[i,'Reuse 7-ways'] = Spill7reuse_df.at[extra_ways_per_skew[i]-1,'SpillTries_Est']
    
## Format Table:
SensFreqSAE_df['Reuse 1-way'] = SensFreqSAE_df['Reuse 1-way'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))
SensFreqSAE_df['Reuse 3-ways'] = SensFreqSAE_df['Reuse 3-ways'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))
SensFreqSAE_df['Reuse 5-ways'] = SensFreqSAE_df['Reuse 5-ways'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))
SensFreqSAE_df['Reuse 7-ways'] = SensFreqSAE_df['Reuse 7-ways'].apply(
    lambda x: (np.format_float_scientific(x,precision=1, exp_digits=2)+ ' ('+format_time(x)+')'))

print("----------------------------------------------------------------------------------------------------------------")
with pd.option_context('expand_frame_repr', False):
    print (SensFreqSAE_df)
print("----------------------------------------------------------------------------------------------------------------")

