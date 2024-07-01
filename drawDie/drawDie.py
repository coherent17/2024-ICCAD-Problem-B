import matplotlib.pyplot as plt
from matplotlib.pyplot import MultipleLocator
import matplotlib.patches as patches
import sys
import numpy as np
from decimal import Decimal
from shapely import Polygon
import random
import math
import time


# read file
def readFile(filename):
    try:
        f = open(filename, 'r')
        FFCells = {}
        FFPinList = {}
        GateCells = {}
        GatePinList = {}
        InstList = {}
        SiteRows = []
        PinNum = 0
        NumInput = 0
        IOList = {}
        NumOutput = 0
        isFFPin = True
        isNet = False
        NetList = {}
        for line in f:  # new line
            line = line.strip('\n')    # delete '\n'
            lineList = line.split(' ') # split line by space 
            
            if NumInput != 0:
                if lineList[0] != 'Input':      # check correct input number
                    print("Input Number Wrong!")
                else:
                    IOList[lineList[1]] = ["I",lineList[2:4]]
                    NumInput = NumInput-1
            elif NumOutput != 0:
                if lineList[0] != 'Output':      # check correct output number
                    print("Output Number Wrong!")
                else:
                    IOList[lineList[1]] = ["O",lineList[2:4]]
                    NumOutput = NumOutput-1
                    
            elif PinNum != 0:
                if lineList[0] != 'Pin':        # check correct pin number
                    print("Pin Number Wrong!")
                elif isNet == True:             # save net pin 
                    NetList[NetName].append(lineList[1])
                    PinNum = PinNum-1
                    if PinNum == 0:
                        isNet = False
                    continue
                else:                           # save pin position in 3D dict
                    if isFFPin == True:         # decide to which pin list
                        FFPin = FFPinList.get(CellName, 'none')
                        if FFPin == 'none':     # build new cell pin dictionary
                            FFPinList[CellName] = {}
                            FFPinList[CellName][lineList[1]] = (lineList[2:])   # add pin in dictionary
                        else:                   
                            FFPinList[CellName][lineList[1]] = (lineList[2:])   # add pin in dictionary
                    else:
                        GatePin = GatePinList.get(CellName, 'none')
                        if GatePin == 'none':   # build new cell pin dictionary
                            GatePinList[CellName] = {}
                            GatePinList[CellName][lineList[1]] = (lineList[2:]) # add pin in dictionary
                        else:
                            GatePinList[CellName][lineList[1]] = (lineList[2:]) # add pin in dictionary
                    PinNum = PinNum-1
                    continue    # jump to new line
            # save useful Info
            match lineList[0]:
                case 'DieSize':         # save die size
                    DieSize = lineList[1:]
                case 'NumInput':
                    NumInput = int(lineList[1])
                case 'NumOutput':
                    NumOutput = int(lineList[1])
                case 'BinWidth':        # save bin width
                    BinWidth = Decimal(lineList[1])
                case 'BinHeight':       # save bin height
                    BinHeight = Decimal(lineList[1])
                case 'FlipFlop':        # save FF cells name & W,H
                    CellName = lineList[2]
                    FFCells[CellName] = lineList[3:5]
                    isFFPin = True
                    PinNum = int(lineList[5])
                case'Gate':             # save gate cells name & W,H
                    CellName = lineList[1]
                    GateCells[CellName] = lineList[2:4]
                    isFFPin = False
                    PinNum = int(lineList[4])
                case 'Inst':            # save all instance name, position and library name 
                    InstList[lineList[1]] = (lineList[2:])
                case 'PlacementRows':   # save placement row
                    SiteRows.append(lineList[1:])
                case 'Net':             # save net name and pin number
                    isNet = True
                    NetName = lineList[1]
                    PinNum = int(lineList[2])
                    NetList[NetName] = []
        f.close()
        # print(IOList)
        DieSize, IOList, SiteRows, FFCells, GateCells, FFPinList, GatePinList, InstList = dataPreprocess(DieSize, IOList, SiteRows, FFCells, GateCells, FFPinList, GatePinList, InstList)
        return DieSize, IOList, BinWidth, BinHeight, SiteRows, FFCells, GateCells, FFPinList, GatePinList, InstList, NetList
    except FileNotFoundError:
        print("file not exist")

# change string to decimal
def dataPreprocess(DieSize, IOList, SiteRows, FFCells, GateCells, FFPinList, GatePinList, InstList):
    for x in range(len(DieSize)):
        DieSize[x] = Decimal(DieSize[x])

    for IO in IOList:
        IOList[IO][1][0] = Decimal(IOList[IO][1][0])
        IOList[IO][1][1] = Decimal(IOList[IO][1][1])

    for row in SiteRows:
        last_idx = len(row)-1
        for i in range(last_idx):
            row[i] = Decimal(row[i])
        row[last_idx] = int(row[last_idx])
    
    for cell in FFCells:
        FFCells[cell][0] = Decimal(FFCells[cell][0])
        FFCells[cell][1] = Decimal(FFCells[cell][1])
    # print(FFCells)

    for cell in GateCells:
        GateCells[cell][0] = Decimal(GateCells[cell][0])
        GateCells[cell][1] = Decimal(GateCells[cell][1])
    # print(GateCells)

    for cell in FFPinList:
        for pin in FFPinList[cell]:
            FFPinList[cell][pin][0] = Decimal(FFPinList[cell][pin][0])
            FFPinList[cell][pin][1] = Decimal(FFPinList[cell][pin][1])

    for cell in GatePinList:
        for pin in GatePinList[cell]:
            GatePinList[cell][pin][0] = Decimal(GatePinList[cell][pin][0])
            GatePinList[cell][pin][1] = Decimal(GatePinList[cell][pin][1])
    
    for inst in InstList:
        InstList[inst][1] = Decimal(InstList[inst][1])
        InstList[inst][2] = Decimal(InstList[inst][2])
    # print(InstList)

    return DieSize, IOList, SiteRows, FFCells, GateCells, FFPinList, GatePinList, InstList


# draw die
def drawDie(DieSize, BinWidth, BinHeight, SiteRows, gridOff):
    # die size
    chipXLeft = DieSize[0]
    chipYLow = DieSize[1]
    chipXRight = DieSize[2]
    chipYHigh = DieSize[3]
    chipWidth = chipXRight - chipXLeft
    chipHeight = chipYHigh - chipYLow
    chipArea = chipWidth*chipHeight

    # FF & Gate color label
    labelX_pos = chipXRight-chipWidth/4
    labelY_pos = chipYHigh + chipYHigh/20
    plt.text(labelX_pos,   labelY_pos, "   ", size=12,       color='orange',         ha='center',  bbox=dict(boxstyle="square",fc='orange', ec='none'))
    labelX_pos = labelX_pos + chipWidth/20
    plt.text(labelX_pos,   labelY_pos, " :FF", size=12,      color='black',          ha='center')
    labelX_pos = labelX_pos + chipWidth/16
    plt.text(labelX_pos,   labelY_pos, "   ", size=12,       color='lightseagreen',  ha='center',  bbox=dict(boxstyle="square",fc='lightseagreen',        ec='none'))
    labelX_pos = labelX_pos + chipWidth/16
    plt.text(labelX_pos,   labelY_pos, " :Gate", size=12,    color='black',          ha='center')
    

    
    # draw bin line & xy label
    x_major_locator = MultipleLocator(int(chipWidth/BinWidth))
    y_major_locator = MultipleLocator(int(chipHeight/BinHeight))
    plt.xlim(chipXLeft,chipXRight)
    plt.ylim(chipYLow,chipYHigh)
    ax = plt.gca()
    ax.xaxis.set_major_locator(x_major_locator)
    ax.yaxis.set_major_locator(y_major_locator)
    if gridOff != True :
        plt.grid(color='black', alpha=0.2, linewidth=1.5, linestyle="-", zorder=1)

        # draw placement row
        for row_i in range(len(SiteRows)):
            # Row Info
            curSite = SiteRows[row_i]
            siteW = curSite[2]
            siteH = curSite[3]
            siteNum = curSite[4]
            downRow = curSite[1]
            upRow = downRow + siteH
            leftX = curSite[0]
            rightX = leftX + siteW * (siteNum + 1)
            # draw site lines
            plt.hlines(downRow, leftX, rightX, color='gainsboro', alpha=1, linewidth=2.1, linestyle="--", zorder=3)
            plt.hlines(upRow, leftX, rightX, color='gainsboro', alpha=1, linewidth=2.1, linestyle="--", zorder=3)
            for col_x in range(siteNum+2):
                curX = leftX + col_x*siteW
                plt.vlines(curX, downRow, upRow, color='gainsboro', alpha=1, linewidth=2.1, linestyle="--", zorder=3)

    return chipArea


    

def drawOverlap(InstSave):
    overlap = []
    InstSave.sort(key=lambda x:(x[0], x[1]))    # sort by xmin & xmax
    for inst_i in range(len(InstSave)-1):
        for inst_j in range(inst_i+1, len(InstSave)):
            instA = InstSave[inst_i]
            instB = InstSave[inst_j]
            if instA[1] <= instB[0]:            # only need checking those insts' x between xwidth
                break
            # instance A polygon point
            PolyA_pos1 = (instA[0], instA[2])
            PolyA_pos2 = (instA[1], instA[2])
            PolyA_pos3 = (instA[1], instA[3])
            PolyA_pos4 = (instA[0], instA[3])
            PolyA = Polygon([PolyA_pos1,PolyA_pos2,PolyA_pos3,PolyA_pos4])
            # instance B polygon point
            PolyB_pos1 = (instB[0], instB[2])
            PolyB_pos2 = (instB[1], instB[2])
            PolyB_pos3 = (instB[1], instB[3])
            PolyB_pos4 = (instB[0], instB[3])
            PolyB = Polygon([PolyB_pos1,PolyB_pos2,PolyB_pos3,PolyB_pos4])
            # check whether overlaping
            overlapRegion = PolyA.intersection(PolyB)
            # draw the overlapping region
            if overlapRegion.area != 0:
                plt.fill(*overlapRegion.exterior.xy, color='palevioletred', alpha=0.4)
    return overlap

# draw blocks
def drawBlocks( chipArea, IOList, FFCells, GateCells, FFPinList, GatePinList, InstList, pinOff, netOff):
    InstSave = []
    ax = plt.gca()
    instPinList = {}
    for instName in InstList:
        cellName = InstList[instName][0]
        # check if instance's cell in library
        FFInfo = 'none'
        GateInfo = 'none'
        FFInfo = FFCells.get(cellName, 'none')
        if FFInfo == 'none':
            GateInfo = GateCells.get(cellName, 'none')
        # set block's color
        if FFInfo != 'none':
            block = FFInfo
            blockColor = 'orange'
            isFF = True
        elif GateInfo != 'none':
            block = GateInfo
            blockColor = 'lightseagreen'
            isFF = False
        else:
            print("Not exist in cell library !!!")
        # print(block)
        # draw block
        blockX = InstList[instName][1]
        blockY = InstList[instName][2]
        blockW = block[0]
        blockH = block[1]
        rect = patches.Rectangle((blockX, blockY),blockW, blockH, linewidth=2, ec = blockColor, fc = 'none', zorder=2)
        ax.add_patch(rect)

        
        # draw block pin and IO pin
        if pinOff == False and netOff == True:
            for IO in IOList:
                plt.text(IOList[IO][1][0], IOList[IO][1][1], "      ", color='cornflowerblue', ha='center', bbox=dict(boxstyle="round",fc='cornflowerblue', ec='none'), size=4)
                # drawIO(chipArea, IOList, 'cornflowerblue')
        instPinList[instName] = {}
        if isFF == True:
            # draw each FF instance pin
            for PinList in FFPinList[cellName]:
                pinX = blockX + FFPinList[cellName][PinList][0]
                pinY = blockY + FFPinList[cellName][PinList][1]
                instPinList[instName][PinList] = [pinX, pinY]
                # if pinOff == False and netOff == True:
                if pinOff == False:
                    ax.plot(pinX, pinY, marker='*', ms=8, zorder=3, color='crimson')
        else:
            # draw each gate instance pin
            for PinList in GatePinList[cellName]:
                pinX = blockX + GatePinList[cellName][PinList][0]
                pinY = blockY + GatePinList[cellName][PinList][1]
                instPinList[instName][PinList] = [pinX, pinY]
                # if pinOff == False and netOff == True:
                if pinOff == False:
                    ax.plot(pinX, pinY, marker='*', ms=8, zorder=3, color='crimson')

        # draw instance name 
        centerX = blockX + blockW/2
        centerY = blockY + blockH/2
        # print(centerX, centerY)
        ax.annotate(instName, xy=(centerX, centerY), fontsize = 10, ha='center', va='center', zorder=4)
        
        # update inst info
        InstList[instName].append(blockW)
        InstList[instName].append(blockH)
        InstList[instName].append(centerX)

        # save instance info for drawing overlapping region
        InstSave.append([blockX, blockX+blockW, blockY, blockY+blockH])     
    # print(InstList)
    drawOverlap(InstSave)
    return InstList, instPinList

# def drawIO(chipArea, IOList, color):
#     for IO in IOList:
#         plt.text(Decimal(IOList[IO][0]), Decimal(IOList[IO][1]), "      ", color=color, ha='center', bbox=dict(boxstyle="round",fc=color, ec='none'), size=int(chipArea/500))

# def generate_random_color(min_distance=0.2, previous_colors=None, max_attempts=100):

#     def color_distance(color1, color2):
#         # 计算两个颜色之间的欧氏距离。
#         return math.sqrt(sum((c1 - c2) ** 2 for c1, c2 in zip(color1, color2)))

#     if previous_colors is None:
#         previous_colors = []

#     if not previous_colors:
#         return (random.random(), random.random(), random.random())

#     for _ in range(max_attempts):
#         r = random.random()
#         g = random.random()
#         b = random.random()

#         min_distance_to_previous = min(color_distance(new_color, prev_color) for new_color in ((r, g, b),) for prev_color in previous_colors)

#         if min_distance_to_previous >= min_distance:
#             return (r, g, b)

def randomcolor():
    colorArr = ['1','2','3','4','5','6','7','8','9','A','B','C','D','E','F']
    color ="#"+''.join([random.choice(colorArr) for i in range(6)])
    return color

def drawNetList(InstList, instPinList, IOList, NetList):
    
    # print(instPinList)
    for net in NetList:
        # choose color
        # previous_colors = []
        # color = generate_random_color(min_distance=0.3, previous_colors=previous_colors)
        # previous_colors.append(color)
        color = randomcolor()
        
        pinLeft = []    # record pin at block's left
        pinRight = []   # record pin at block's right
        for pin in NetList[net]:
            
            pinSet = pin.split('/') # distinguish pin is IO pin or instance pin 
            # print(pinSet)
            if len(pinSet) == 1:    # IO pin
                # plt.plot(Decimal(IOList[pinSet[0]][0]), Decimal(IOList[pinSet[0]][1]), marker='d', ms=int(chipArea/200), zorder=3, color='crimson')
                IOpinX = IOList[pinSet[0]][1][0]
                IOpinY = IOList[pinSet[0]][1][1]
                plt.text(IOList[pinSet[0]][1][0], IOList[pinSet[0]][1][1], "      ", color=color, va='center', ha='center', bbox=dict(boxstyle="round",fc=color, ec='none'), size=int(4))
                if IOList[pinSet[0]][0] == "I":
                    pinRight.append([IOpinX, IOpinY])
                else:
                    pinLeft.append([IOpinX, IOpinY])
            else:   
                # draw instance pin 
                instName = pinSet[0]
                pinName = pinSet[1]
                pinX = instPinList[instName][pinName][0]
                pinY = instPinList[instName][pinName][1]
                if pinOff == False:
                    plt.plot(pinX, pinY, marker='*', ms=int(8), zorder=3, color=color)

                # split net pin in left/right
                if pinX > InstList[instName][5]:
                    pinRight.append([pinX, pinY])
                else:
                    pinLeft.append([pinX, pinY])
        
        # find left min and right max pin in netlist
        leftMinX= list(map(min,zip(*pinLeft)))[0]
        rightMaxX= list(map(max,zip(*pinRight)))[0]

        maxY= list(map(max,zip(*(pinRight+pinLeft))))[1]
        minY= list(map(min,zip(*(pinRight+pinLeft))))[1]

        # draw verticle net 
        vlineX = (leftMinX + rightMaxX)/2
        plt.vlines(vlineX, minY, maxY, color=color, alpha=1, linewidth=1, zorder=3)
        
        # draw horizontal net 
        for pin in pinRight:
            pinX = pin[0]
            pinY = pin[1]
            plt.hlines(pinY, pinX, vlineX, color=color, alpha=1, linewidth=1, zorder=3)
        for pin in pinLeft:
            pinX = pin[0]
            pinY = pin[1]
            plt.hlines(pinY, pinX, vlineX, color=color, alpha=1, linewidth=1, zorder=3)



if __name__ == "__main__":

    start_time = time.time()

    # decide whether to draw grid, pin, netlist
    gridOff = False
    pinOff = False
    netOff = False
    for command in sys.argv[1:]:
        if command == "gridOff":
            gridOff = True
        elif command == "netOff":
            netOff = True
        elif command == "pinOff":
            pinOff = True
    
    filename = sys.argv[1]
    DieSize, IOList, BinWidth, BinHeight, SiteRows, FFCells, GateCells, FFPinList, GatePinList, InstList, NetList = readFile(filename)
    
    # figure size
    chipXLeft = DieSize[0]
    chipYLow = DieSize[1]
    chipXRight = DieSize[2]
    chipYHigh = DieSize[3]
    chipWidth = chipXRight - chipXLeft
    chipHeight = chipYHigh - chipYLow
    chipRatio = chipHeight/chipWidth
    plt.figure(figsize=(10,int(10*chipRatio)))

    chipArea = drawDie(DieSize, BinWidth, BinHeight, SiteRows, gridOff)
    InstList, instPinList = drawBlocks(chipArea, IOList, FFCells, GateCells, FFPinList, GatePinList, InstList, pinOff, netOff)
    if netOff == False:
        drawNetList(InstList, instPinList, IOList, NetList)
    
    # print runtime
    end_time = time.time()
    execution_time = end_time - start_time
    print("Runtime：", execution_time, "s")

    plt.savefig("die_pic.png")
    plt.show()

