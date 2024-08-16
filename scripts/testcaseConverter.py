
# script for convert to MBFF design
# python ./testcaseConverter.py [input desgin] [banking result] [output design] 
# 
import sys
import random

if len(sys.argv) != 4:
    print("python ./testcaseConverter.py [input desgin] [banking result] [output design] ")

inputDesign = sys.argv[1]
outputResult = sys.argv[2]
outputDesign = sys.argv[3]

# parse output banking result
instList = []
instMap = {}
newInstNum = 0
with open(outputResult, 'r') as f:
    for line in f.readlines():
        lineSplit =  line.split()
        if(lineSplit[0] == "CellInst"):
            newInstNum = int(lineSplit[1])
        elif(lineSplit[0] == "Inst"):
            instList.append(line)
        else:
            instMap[lineSplit[0]] = lineSplit[2]

# parse input design to convert
FFcell = set()
outputFile = open(outputDesign, 'w')
stdCellCount = 0
with open(inputDesign, 'r') as f:
    for line in f.readlines():
        lineSplit =  line.split()
        if(lineSplit[0] == "FlipFlop"):
            FFcell.add(lineSplit[2])
        elif(lineSplit[0] == "Inst" and not lineSplit[2] in FFcell):
            stdCellCount += 1

instListIdx = 0
with open(inputDesign, 'r') as f:
    for line in f.readlines():
        lineSplit =  line.split()
        if(lineSplit[0] == "NumInstances"):
            outputFile.write(lineSplit[0] + " " + str(stdCellCount + newInstNum) + "\n")
        elif(lineSplit[0] == "Inst" and lineSplit[2] in FFcell):
            if(instListIdx < len(instList)):
                outputFile.write(instList[instListIdx])
                instListIdx += 1
        elif(lineSplit[0] == "Pin" and len(lineSplit) == 2):
            if(lineSplit[1] in instMap):
                outputFile.write(f'Pin {instMap[lineSplit[1]]}\n')
            else:
                outputFile.write(line)
        elif(lineSplit[0] == "TimingSlack"):
            original = lineSplit[1] + '/' + lineSplit[2]
            new = instMap[original]
            FFandPin = new.split("/")
            outputFile.write(f'{lineSplit[0]} {FFandPin[0]} {FFandPin[1]} {random.uniform(-5, 10)}\n')
        else:
            outputFile.write(line)