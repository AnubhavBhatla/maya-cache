# Melt the DataFrame to have a single value column and a 'variable' column
# df_melted = pd.melt(comp_maya_mirage, id_vars=['Benchmarks'], var_name='Variable', value_name='Value')

# # Plot side-by-side bar plot
# plt.figure(figsize=(10, 6))
# sns.barplot(data=df_melted, x='Benchmarks', y='Value', hue='Variable')
# plt.title('Side-by-Side Bar Plot')
# plt.xlabel('Categories')
# plt.ylabel('Values')
# plt.tight_layout()

# plt.show()