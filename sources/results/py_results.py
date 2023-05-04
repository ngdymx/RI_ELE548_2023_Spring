#!/usr/bin/python3
import sys
import os

AllBench = [
    "400.perlbench",
    "401.bzip2",
    "403.gcc",
    "410.bwaves",
    "416.gamess",
    "429.mcf",
    "433.milc",
    "434.zeusmp",
    "435.gromacs",
    "436.cactusADM",
    "437.leslie3d",
    "444.namd",
    "445.gobmk",
    "447.dealII",
    "450.soplex",
    "453.povray",
    "456.hmmer",
    "458.sjeng",
    "459.GemsFDTD",
    "462.libquantum",
    "464.h264ref",
    "465.tonto",
    "470.lbm",
    "471.omnetpp",
    "473.astar",
    "481.wrf",
    "482.sphinx3",
    "483.xalancbmk"
]

CFG = sys.argv[1]
repls = sys.argv[2].split()

lru_ipc = []

for bench in AllBench:
    name = bench + "." + "lru" + "-" + CFG
    val = float(os.popen("grep 'CPU 0 cummulative IPC: ' " + name + \
                   " | cut -f5 -d' '").read())
    lru_ipc.append(val)



geo_mean_lru = 1
for i in range(len(AllBench)):
    geo_mean_lru = geo_mean_lru * lru_ipc[i]

geo_mean_lru = geo_mean_lru ** (1/len(AllBench))
lru_ipc.append(geo_mean_lru)

algorithm = []
for repl in repls:
    c_algorithm = []
    geo_mean = 1
    for bench in AllBench:
        name = bench + "." + repl + "-" + CFG
        val = float(os.popen("grep 'CPU 0 cummulative IPC: ' " + name + \
                       " | cut -f5 -d' '").read())
        geo_mean = geo_mean * val
        c_algorithm.append(val)
    geo_mean = geo_mean ** (1 / len(AllBench))
    c_algorithm.append(geo_mean)
    algorithm.append(c_algorithm)
    del c_algorithm

print("%-20s" % ("Benchmark"), end = " ")

print("|%8s" % "IPC", end = " ")

for repl in repls:
    print("|%8s" % repl, end = " ")

print("")

AllBench.append("Geo Mean")

print("%-20s" % ("-" * 20), end = "-")
for repl in repls:
    print("+%8s" % ("-" * 8), end = "-")
print("+%8s" % ("-" * 8))


for i in range(len(AllBench)):
    name = AllBench[i]
    print("%-20s |%8.4f" % (name, lru_ipc[i]), end = " ")
    for j in range(len(repls)):
        print("|%8.4f" % (algorithm[j][i] / lru_ipc[i]), end = " ")

    print("")
