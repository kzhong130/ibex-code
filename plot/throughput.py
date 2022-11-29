import matplotlib.pyplot as plt
import numpy as np

def scale(vec, s):
    for i in range(len(vec)):
        vec[i] = vec[i] * s

fig, ax1 = plt.subplots(figsize=(8.0,4.0))
fontsize=20
dotsize=7.0
dotsize_50=7.0

font = { 'family': 'Arial',
'weight' : 'normal',
'size'   : fontsize,
}

ax1.set_xlabel('Throughput (requests/min)', font)
ax1.set_ylabel('Response time (sec)', font)

# x_group_14 = [30,60, 80, 100, 102, 104, 99, 98]*2
# y_group_14 = [0.823437,0.820387, 0.825128, 0.830367, 0.830394, 1.07985, 2.77594, 3.27458]

# x_baseline_5 = [15, 20, 19]
# y_baseline_5 = [3.720, 4.829, 20.871]

x_14_50 = [40,80,105,110,115,109,108]
y_14_50 = [0.728216,0.752344,0.752807,0.824917,0.867612,2.79326,4.38712]

x_15_50 = [40,80,100,102,104,105,99,97]
y_15_50 = [0.78299,0.820669,0.795633,0.856837,0.884747,0.919158,1.22475,3.68042]

x_16_50 = [40,60,70,80,85,90,90,87]
y_16_50 = [0.879943,0.897737,0.881385,0.87913,0.882796,0.977653,1.04798,2.50277]

x_baseline_50 = [40,80,100,160,200,210,215,220,225,229,224]
y_baseline_50 = [0.414312,0.410287,0.417921,0.450895,0.4582,0.480908,0.509908,0.507866,0.685618,0.961494,3.35477]

x_14_99 = x_14_50
y_14_99 = [0.883195,0.952327,0.941494,1.04447,1.26522,5.90265,8.12179]

x_15_99 = x_15_50
y_15_99 = [0.940426,1.05425,0.982442,1.26124,1.28278,1.11637,6.78924,9.73897]

x_16_99 = x_16_50
y_16_99 = [1.0707,1.1017,1.18112,1.06388,1.28069,1.59536,5.76922,9.19464]

x_baseline_99 = x_baseline_50
y_baseline_99 = [0.570843,0.600802,0.722449,0.725692,0.756075,0.790426,0.911764,0.897504,1.41953,1.35321,4.85071]

# scale(y_15_50, 1000)
# scale(y_14_50, 1000)
# scale(y_16_50, 1000)
# scale(y_baseline_50, 1000)
# scale(y_15_99, 1000)
# scale(y_14_99, 1000)
# scale(y_16_99, 1000)
# scale(y_baseline_99, 1000)

# x_ock_5 = [25, 50, 60, 70, 80, 82, 85]
# y_ock_5 = [1.23, 1.250, 1.274, 1.667, 5.653, 10.26, 20.195]

# x_ours_1 = [60, 90, 120, 140, 180, 209]
# y_ours_1 = [0.712, 0.744, 0.807, 2.244, 11.179, 23.917]

# x_ours_5 = [60, 90, 120, 116]
# y_ours_5 = [1.039, 0.948, 5.520, 21.030]

my_line_width = 3


ax1.plot(x_14_50, y_14_50, color='lightslategrey',
          linestyle='dashed', linewidth=my_line_width,
          marker='v', markerfacecolor='white', markersize=dotsize_50,
          label='Group size: $2^{14}$')
# ax1.plot(x_14_99, y_14_99, color='lightslategrey',
#           linestyle='dashed', linewidth=my_line_width,
#           marker='D', markerfacecolor='white', markersize=dotsize,
#           label='Group size: $2^{14}$ - 99p')
ax1.plot(x_15_50, y_15_50, color='rosybrown',
          linestyle='dashed', linewidth=my_line_width,
          marker='D', markerfacecolor='white', markersize=dotsize_50,
          label='Group size: $2^{15}$')
# ax1.plot(x_15_99, y_15_99, color='darkred',
#           linestyle='dashed', linewidth=my_line_width,
#           marker='o', markerfacecolor='white', markersize=dotsize,
#           label='Group size: $2^{15}$ - 99p')
ax1.plot(x_16_50, y_16_50, color='orange',
          linestyle='dashed', linewidth=my_line_width,
          marker='o', markerfacecolor='white', markersize=dotsize_50,
          label='Group size: $2^{16}$')
# ax1.plot(x_16_99, y_16_99, color='lightslategrey',
#           linestyle='dashed', linewidth=my_line_width,
#           marker='D', markerfacecolor='white', markersize=dotsize,
#           label='Group size: $2^{16}$ - 99p')
ax1.plot(x_baseline_50, y_baseline_50, color='peru',
          linestyle='dashed', linewidth=my_line_width,
          marker='^', markerfacecolor='white', markersize=dotsize_50,
          label='Baseline (non-private)')
# ax1.plot(x_baseline_99, y_baseline_99, color='orange',
#           linestyle='dashed', linewidth=my_line_width,
#           marker='s', markerfacecolor='white', markersize=dotsize,
#           label='Baseline (non-private) - 99p')

# Function x**(1/2)
def forward(x):
    return x**(1/2)


def inverse(x):
    return x**2

# ax1.set_yscale('function', functions=(forward, inverse))
ax1.set_yscale('log')
# plt.xlim(xmin=0, xmax = 600)
# plt.ylim(ymin=0,ymax = 150)

ax1.spines['bottom'].set_linewidth(2)
ax1.spines['left'].set_linewidth(2)

ax1.spines['top'].set_visible(False)
ax1.spines['right'].set_visible(False)

plt.rcParams['font.family'] = 'serif'

ax1.set_yticks([1,3])
ax1.set_yticklabels(["${1}$","3"], fontsize=70)
# ax1.set_ylim(0, 4)

ax1.legend(ncol=1, loc='upper center', framealpha=0.0, bbox_to_anchor=(0.67
, 1.0), prop={'family':'Arial', 'weight': 'bold', 'size': 18})
# ax1.legend(ncol=1, loc='upper center', bbox_to_anchor=(0.3, 1.03), framealpha=0.0, prop={'family':'Arial', 'weight': 'bold', 'size': 10})
# plt.grid(True, 'major', 'y', ls='--', lw=.5, c='k', alpha=.3)


plt.tick_params(labelsize=fontsize)
labels = ax1.get_xticklabels() + ax1.get_yticklabels()
# print labels
[label.set_fontname('Arial') for label in labels]

fig.tight_layout()
fig.savefig("throughput-eval.eps", format='eps', bbox_inches='tight')