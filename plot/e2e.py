import os
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt

width = 0.7
linewidth = 5
font_size = 45
mpl.rc('hatch', linewidth=2)
font_name = 'Arial'
l_font = {'fontname': font_name}
s_font = {'family': font_name, 'size' : font_size}
bar_font = {'family': font_name, 'size' : 40}
bar_patterns = ('/', '\\', '/', '\\/', '-', '|')
colors = ((48/255., 115/255., 175/255.), (224/255., 23/255., 27/255.), (227/255., 161/255., 4/255.), (0., 0., 0.), (141/255., 69/255., 153/255.), (154/255., 205/255., 50/255.))

figure, axes = plt.subplots(figsize = (20, 10))
labels = np.array([3, 6, 9])
ratios = ['5K','10K','20K']
ratio_search_mean = [1,2,4]
ratio_search_std = None

time_14 = [784.787,1561.46,3113.21]
time_15 = [1700.15,3366.03,6714.41]
time_16 = [3581.62,7095.1,14157.8]

for i in range(len(time_14)):
    time_14[i] = time_14[i]/60
    time_15[i] = time_15[i]/60
    time_16[i] = time_16[i]/60

axes.bar(labels-width, time_14, width, yerr=ratio_search_std, hatch=bar_patterns[1], color='white', linewidth=linewidth, edgecolor=colors[0], capsize=8, label="Group size: 2$^{14}$")

axes.bar(labels, time_15, width, yerr=ratio_search_std, hatch=bar_patterns[2], color='white', linewidth=linewidth, edgecolor=colors[1], capsize=8, label="Group size: 2$^{15}$")

axes.bar(labels+width, time_16, width, yerr=ratio_search_std, hatch=bar_patterns[3], color='white', linewidth=linewidth, edgecolor=colors[2], capsize=3, label="Group size: 2$^{16}$")

axes.set_xticks(labels)
axes.set_xticklabels(ratios, fontsize=font_size)
axes.tick_params(axis='y', labelsize=font_size) 
axes.set_xlabel("Number of aggregation reports", **l_font, fontsize=font_size)
axes.set_ylabel("Time (minute)", **l_font, fontsize=font_size)

for tick in axes.get_xticklabels():
    tick.set_fontname(font_name)
for tick in axes.get_yticklabels():
    tick.set_fontname(font_name)

plt.legend(prop = bar_font)
# plt.savefig(os.path.join(directory, "indexer-ratio.pdf"), bbox_inches='tight', pad_inches=0.1)
plt.savefig("e2e-eval.eps", format='eps', bbox_inches='tight', pad_inches=0.1)
