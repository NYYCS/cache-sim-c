#!/usr/bin/python3
import os
import itertools
import pandas as pd

cache_block_sizes = ["8", "16", "32", "64", "128", "256"]
cache_sizes = ["16", "32", "64", "128", "256"]
cache_assocs = ["1", "2", "4", "8"]

cache_cfgs = list(itertools.product(cache_block_sizes, cache_sizes, cache_assocs))


def create_cfgs():
    print("Creating all possible configurations for cache...")
    if os.path.exists("cfgs"):
        os.system("rm -rf cfgs")

    if os.path.exists("outs"):
        os.system("rm -rf outs")

    os.mkdir("cfgs")
    os.mkdir("outs")

    for i, cfg in enumerate(cache_cfgs):
        block_size, size, assoc = cfg
        with open(f"cfgs/cfg-{i:02}.txt", "w") as file:
            file.write("\n".join([block_size, assoc, size, "1", "100", "1"]))


def run_sim():
    print("Running simulation...")
    os.system("make clean && make")
    for i, cfg in enumerate(sorted(os.listdir("cfgs"))):
        cmd = f"./cachesim -c cfgs/{cfg} -t ls.trace -o outs/out-{i:02}.txt"
        os.system(cmd)

def extract_data():
    print("Extracting data from results...")
    data = {
        "Block Size": [],
        "Associativity": [],
        "Cache Size": [],
        "Total Hit Rate": [],
        "Load Hit Rate": [],
        "Store Hit Rate": [],
        "Total Run Time": [],
        "AVG MA Latency": [],
    }
    for i, out in enumerate(sorted(os.listdir("outs"))):
        block_size, size, assoc = cache_cfgs[i]
        with open(f"outs/{out}", "r") as file:
            lines = file.readlines()
            data["Block Size"].append(block_size)
            data["Associativity"].append(assoc)
            data["Cache Size"].append(size)
            data["Total Hit Rate"].append(lines[0].split()[-1])
            data["Load Hit Rate"].append(lines[1].split()[-1])
            data["Store Hit Rate"].append(lines[2].split()[-1])
            data["Total Run Time"].append(lines[3].split()[-1])
            data["AVG MA Latency"].append(lines[4].split()[-1])
    
    df = pd.DataFrame(data)
    df.to_csv("results.csv", index=False)


if __name__ == "__main__":
    create_cfgs()
    run_sim()
    extract_data()
    print("Done!")
