# ELE548 Final report (Spr23, Miaoxiang Yu and Yiming Du)

# Project description

This project aims at adding bypass topology to ship++ cache replacement policy. The details are in ELE548.pdf. The information of the simulator and traces can be found in https://crc2.ece.tamu.edu.


# Compile

```shell
make all
```

# Run

```shell

cd RunScripts
./run_traces lru-config1
./run_traces ship++-config1
./run_traces lru-simpass-config1
./run_traces ship++-simpass-config1
./run_traces prj-16k-3b-config1
./run_traces prj-16k-2b-config1
./run_traces prj-4k-3b-config1
./run_traces prj-4k-2b-config1

```

# Show results

To show the performance, run:

```shell
cd results
./show.sh
```

To see the SHCT distribution, run:

```shell
cd results
./get_SHCT_dist
```

This command generates text files named as "SHCT.benchname.txt". The final number stored in the SHCT table is listed inside the correspoding text file.

