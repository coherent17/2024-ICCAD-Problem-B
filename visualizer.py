import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.collections import PatchCollection

def plot_rectangles(file_path, output_path):
    patches_list_ff = []
    patches_list_gate = []
    die_boundary = None

    with open(file_path, 'r') as file:
        lines = file.readlines()

    for line in lines:
        parts = line.strip().split()
        if not parts:
            continue

        if parts[0].startswith('FF_'):
            x, y, w, h = int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4])
            rect = patches.Rectangle((x, y), w, h, linewidth=0.3, edgecolor='b', facecolor='none')
            patches_list_ff.append(rect)             

        if parts[0].startswith('C'):
            x, y, w, h = int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4])
            rect = patches.Rectangle((x, y), w, h, linewidth=0.3, edgecolor='r', facecolor='none')
            patches_list_gate.append(rect)     

        if parts[0].startswith('DieSize'):
            x1, y1, x2, y2 = int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4])
            die_boundary = patches.Rectangle((x1, y1), x2 - x1, y2 - y1, linewidth=3, edgecolor='black', facecolor='none', label='DieBoundary')


    ratio = (y2-y1)/(x2-x1)
    fig, ax = plt.subplots(figsize=(40, int(40*ratio)))

    # plot ff
    collection_ff = PatchCollection(patches_list_ff, facecolor='none', edgecolor='b', label='FF')
    ax.add_collection(collection_ff)

    # plot gate
    collection_gate = PatchCollection(patches_list_gate, facecolor='none', edgecolor='r', label='Gate')
    ax.add_collection(collection_gate)

    # plot die boundary
    if die_boundary:
        ax.add_patch(die_boundary)

    ax.set_xlim(x1, x2)
    ax.set_ylim(y1, y2)

    legend_ff = patches.Patch(edgecolor='b', facecolor='none', label='FF')
    legend_gate = patches.Patch(edgecolor='r', facecolor='none', label='Gate')
    legend_die_boundary = patches.Patch(edgecolor='black', facecolor='none', label='DieBoundary')
    ax.legend(handles=[legend_ff, legend_gate, legend_die_boundary])

    ax.axis('off')
    plt.savefig(output_path, bbox_inches='tight', pad_inches=1, dpi=600)

if __name__ == '__main__':
    plot_rectangles("/home/coherent17/TEMP/2024-ICCAD-Problem-B/testcase/testcase1_0812.txt_PDA_Lab3", "test.png")