import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Read data from csv files
# Analytical results
data_4800 = pd.read_csv("input/data_4800.csv")
data_2400 = pd.read_csv("input/data_2400.csv")
data_1200 = pd.read_csv("input/data_1200.csv")

# Experimental results
data_4800_expr = pd.read_csv("../results/aggregate_trh_4800.csv")
data_2400_expr = pd.read_csv("../results/aggregate_trh_2400.csv")
data_1200_expr = pd.read_csv("../results/aggregate_trh_1200.csv")

# Set plots characeristics (e.g., size, font) here
plt.rcParams["figure.figsize"] = [3, 2]
plt.rc('font', size=6)

# # Draw plots here
plt.plot(data_4800.iloc[:,0], data_4800.iloc[:,2], color = 'black', markerfacecolor = 'None', marker = 'o', linestyle = '--', markersize=4.7, linewidth=1, label = 'T$_{RH}$ = 4800')
plt.plot(data_2400.iloc[:,0], data_2400.iloc[:,2], color = 'dimgrey',markerfacecolor = 'None', marker = 'd',linestyle = '--', markersize=5, linewidth=1, label = 'T$_{RH}$ = 2400')
plt.plot(data_1200.iloc[:,0], data_1200.iloc[:,2], color = 'darkslategray', markerfacecolor = 'None', marker = 's',linestyle = '--', markersize=4.7, linewidth=1, label = 'T$_{RH}$ = 1200')
plt.plot(data_4800.iloc[:,0], data_4800_expr.iloc[:,3], color = 'red', marker = 'x', linestyle = 'None', markersize=2.5, label = 'Time-To-Break (Experiment)')
plt.plot(data_2400.iloc[:,0], data_2400_expr.iloc[:,3], color = 'red', marker = 'x', linestyle = 'None', markersize=2.5)
plt.plot(data_1200.iloc[:,0], data_1200_expr.iloc[:,3], color = 'red', marker = 'x', linestyle = 'None', markersize=2.5)


# Set axis characteristics (label, scale, etc) here
plt.xlabel('Number of Attack Rounds to the Aggressor Row')
plt.ylabel('Time-to-Break (Days)')

plt.yscale('log')

plt.xlim([0,1450])
plt.ylim([0.1e-4,1e4])

plt.tick_params(direction='in', bottom=False)

# Set legends here
plt.legend(loc=(0.54,0.15), ncol=1, fontsize=4.5)

# Set grids
plt.grid(True, linestyle=':')

# Show or save plots here
plt.tight_layout()
# plt.show()
plt.savefig('Figure6.pdf', dpi=600,bbox_inches="tight")