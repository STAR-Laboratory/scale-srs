import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Read data from csv files
data = pd.read_csv("../simscript/data.csv")

# Set plots characeristics (e.g., size, font) here
plt.rcParams["figure.figsize"] = [12, 3]
plt.rc('font', size=10)
# Set bar width here
bar_width = 0.25

label = data['Benchmark']

data_rrs = data['RRS']
data_srs = data['Scale-SRS']

x_index = np.arange(len(label))

fig, ax = plt.subplots()
ax.set_xticks(x_index)
ax.set_xticklabels(label, ha='right')
ax.tick_params(axis='x',rotation=45)

# Draw plots here
perf_rrs = ax.bar(x_index - (bar_width/2), data_rrs, bar_width, color = 'tan', edgecolor='black', linewidth=2, label = 'RRS')
perf_srs = ax.bar(x_index + (bar_width/2), data_srs, bar_width, color = 'darkgreen', edgecolor='black', linewidth=2, label = 'Scale-SRS')

# plt.xticks(x_index, label)

# Draw hline or vline here
plt.axhline(1,0,1, color='gray', linestyle='solid', linewidth=3)
plt.axvline(29, 0, 1, color='red', linestyle='--', linewidth=3)

# Add texts here
plt.text(32, 1.01, 'GMEAN')

# Set axis characteristics (label, scale, etc) here
plt.ylabel('Normalized Performance')

plt.ylim([0.7,1.05])
plt.xlim([-0.5,37.5])

plt.tick_params(direction='in', bottom=False)



# Set legends here
plt.legend(loc=(0,0.85),ncol=2)

# Set grids
plt.grid(True, linestyle=':')

# Show or save plots here
plt.tight_layout()
# plt.show()
plt.savefig('Figure14.pdf', dpi=600, bbox_inches="tight")
