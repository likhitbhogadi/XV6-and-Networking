import matplotlib.pyplot as plt
import numpy as np

# Data parsing
data = """
pid : 5, ticks : 666, queue : 0 
pid : 6, ticks : 667, queue : 0 
pid : 7, ticks : 668, queue : 0 
pid : 8, ticks : 669, queue : 0 
pid : 9, ticks : 670, queue : 0 
pid : 5, ticks : 671, queue : 1 
pid : 5, ticks : 672, queue : 1 
pid : 5, ticks : 673, queue : 1 
pid : 5, ticks : 674, queue : 1 
pid : 6, ticks : 675, queue : 1 
pid : 6, ticks : 676, queue : 1 
pid : 6, ticks : 677, queue : 1 
pid : 6, ticks : 678, queue : 1 
pid : 7, ticks : 679, queue : 1 
pid : 7, ticks : 680, queue : 1 
pid : 7, ticks : 681, queue : 1 
pid : 7, ticks : 682, queue : 1 
pid : 8, ticks : 683, queue : 1 
pid : 8, ticks : 684, queue : 1 
pid : 8, ticks : 685, queue : 1 
pid : 8, ticks : 686, queue : 1 
pid : 9, ticks : 687, queue : 1 
pid : 9, ticks : 688, queue : 1 
pid : 9, ticks : 689, queue : 1 
pid : 9, ticks : 690, queue : 1 
pid : 5, ticks : 691, queue : 2 
pid : 5, ticks : 692, queue : 2 
pid : 5, ticks : 693, queue : 2 
pid : 5, ticks : 694, queue : 2 
pid : 5, ticks : 695, queue : 2 
pid : 5, ticks : 696, queue : 2 
pid : 5, ticks : 697, queue : 2 
pid : 5, ticks : 698, queue : 2 
pid : 6, ticks : 699, queue : 2 
pid : 6, ticks : 700, queue : 2 
pid : 6, ticks : 701, queue : 2 
pid : 6, ticks : 702, queue : 2 
pid : 6, ticks : 703, queue : 2 
pid : 6, ticks : 704, queue : 2 
pid : 6, ticks : 705, queue : 2 
pid : 6, ticks : 706, queue : 2 
pid : 7, ticks : 707, queue : 2 
pid : 7, ticks : 708, queue : 2 
pid : 7, ticks : 709, queue : 2 
pid : 7, ticks : 710, queue : 2 
pid : 7, ticks : 711, queue : 2 
pid : 7, ticks : 712, queue : 2 
pid : 7, ticks : 713, queue : 2 
pid : 7, ticks : 714, queue : 2 
pid : 5, ticks : 715, queue : 0 
pid : 6, ticks : 716, queue : 0 
pid : 7, ticks : 717, queue : 0 
pid : 8, ticks : 718, queue : 0 
pid : 9, ticks : 719, queue : 0 
pid : 5, ticks : 720, queue : 1 
pid : 5, ticks : 721, queue : 1 
pid : 5, ticks : 722, queue : 1 
pid : 5, ticks : 723, queue : 1 
pid : 6, ticks : 724, queue : 1 
pid : 6, ticks : 725, queue : 1 
pid : 6, ticks : 726, queue : 1 
pid : 6, ticks : 727, queue : 1 
pid : 7, ticks : 728, queue : 1 
pid : 7, ticks : 729, queue : 1 
pid : 7, ticks : 730, queue : 1 
pid : 7, ticks : 731, queue : 1 
pid : 8, ticks : 732, queue : 1 
pid : 8, ticks : 733, queue : 1 
pid : 8, ticks : 734, queue : 1 
pid : 8, ticks : 735, queue : 1 
pid : 9, ticks : 736, queue : 1 
pid : 9, ticks : 737, queue : 1 
pid : 9, ticks : 738, queue : 1 
pid : 9, ticks : 739, queue : 1 
pid : 5, ticks : 740, queue : 2 
pid : 5, ticks : 741, queue : 2 
pid : 5, ticks : 742, queue : 2 
pid : 5, ticks : 743, queue : 2 
pid : 5, ticks : 744, queue : 2 
pid : 6, ticks : 745, queue : 2 
pid : 6, ticks : 746, queue : 2 
pid : 6, ticks : 747, queue : 2 
pid : 6, ticks : 748, queue : 2 
pid : 7, ticks : 749, queue : 2 
pid : 7, ticks : 750, queue : 2 
pid : 7, ticks : 751, queue : 2 
pid : 7, ticks : 752, queue : 2 
pid : 8, ticks : 753, queue : 2 
pid : 8, ticks : 754, queue : 2 
pid : 8, ticks : 755, queue : 2 
pid : 8, ticks : 756, queue : 2 
pid : 8, ticks : 757, queue : 2 
pid : 8, ticks : 758, queue : 2 
pid : 8, ticks : 759, queue : 2 
pid : 8, ticks : 760, queue : 2 
pid : 9, ticks : 761, queue : 2 
pid : 9, ticks : 762, queue : 2 
pid : 9, ticks : 763, queue : 2 
pid : 9, ticks : 764, queue : 2 
pid : 9, ticks : 765, queue : 2 
pid : 9, ticks : 766, queue : 2 
pid : 9, ticks : 767, queue : 2 
pid : 9, ticks : 768, queue : 2 
pid : 8, ticks : 769, queue : 0 
pid : 9, ticks : 770, queue : 0 
pid : 8, ticks : 771, queue : 1 
pid : 8, ticks : 772, queue : 1 
pid : 8, ticks : 773, queue : 1 
pid : 9, ticks : 774, queue : 1 
pid : 9, ticks : 775, queue : 1 
pid : 9, ticks : 776, queue : 1
"""

# Parse the data
processes = {}
for line in data.strip().split('\n'):
    line_parts = line.replace("pid :", "").replace("ticks :", "").replace("queue :", "").split(',')
    pid, ticks, queue = map(int, line_parts)
    if pid not in processes:
        processes[pid] = {'ticks': [], 'queue': []}
    processes[pid]['ticks'].append(ticks)
    processes[pid]['queue'].append(queue)

# Plotting
plt.figure(figsize=(15, 8))

colors = ['r', 'g', 'b', 'c', 'm']  # Colors for different processes

# Plotting with lines and scatter points
for i, (pid, data) in enumerate(processes.items()):
    # Scatter points
    plt.scatter(data['ticks'], data['queue'], label=f'Process {pid}', 
                color=colors[i % len(colors)], s=50, alpha=0.7)
    # Line plot
    plt.plot(data['ticks'], data['queue'], color=colors[i % len(colors)], alpha=0.7)

plt.xlabel('Ticks')
plt.ylabel('Queue Number')
plt.title('MLFQ Scheduler Visualization with Lines')
plt.yticks(range(4))  # 4 queues (0-3)
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.legend()

# Improve x-axis readability
plt.xticks(np.arange(min(ticks for proc in processes.values() for ticks in proc['ticks']), 
                     max(ticks for proc in processes.values() for ticks in proc['ticks']) + 1, 5))

plt.show()
