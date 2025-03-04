import os
import seaborn as sns 
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import re

class BoundChecker:
    def __init__(self, lower_bound, upper_bound):
        self.lower_bound = lower_bound
        self.upper_bound = upper_bound

    def check_bound(self, value):
        if value < self.lower_bound:
            return 0
        elif self.lower_bound <= value <= self.upper_bound:
            return 1
        else:
            return 2

def timing(*, plot=True) -> tuple[float, float, float]:
    df = pd.read_csv("results/timing.csv")
    ram_upper = df["RAM"].mean() + df["RAM"].std()
    df = df[~(df > ram_upper).any(axis=1)] # drop any measurement greater than ram max
   
    print("#### Timing Analysis ####")
    for col in df.columns:
        print(f"{col:>4} |> Min: {df[col].min():4}, Max: {df[col].max():4}, Mean: {df[col].mean():4.4}, Std: {df[col].std():4.4}")
    print()

    if not plot:
        return ram_upper 

    # Create a bar plot
    fig = plt.figure(figsize=(12, 6))
    
    # Plot each column as a separate set of bars
    for column in df.columns:
        plt.hist(df[column], bins=np.arange(0,int(ram_upper)), label=column, alpha = 0.5)
    
    # Add bounds to L2 and L3 
    l2_upper = df["L2"].mean() + df["L2"].std()
    l3_lower = df["L3"].mean() - df["L3"].std()
    l3_upper = df["L3"].mean() + 2 * df["L3"].std()
    y_min, y_max = plt.gca().get_ylim()
    plt.vlines(l2_upper, y_min, y_max, label="L2 Upper Bound", color="tab:orange", linestyle="--")
    plt.vlines(l3_lower, y_min, y_max, label="L3 Lower Bound", color="tab:green", linestyle="--")

    # Customize the plot
    plt.xlim(0, 350);
    plt.xlabel('Cycles (+ error from pipeline and rdtscp)')
    plt.ylabel('Count')
    plt.title('Cache Latencies')
    plt.legend()
    
    # Show the plot
    plt.tight_layout()
    plt.show()
    fig.savefig("figs/latencies.pdf")
    plt.close(fig)

    return (l3_lower, l3_upper, ram_upper)

def nlp(ram_bound: float, plot=False):
    df = pd.read_csv("results/next_line.csv")
    df = df[df["Cycles"] <= ram_bound]

    print("#### Next Line Analysis ####")
    for ts in df["TrainingSize"].unique():
        if ts > 10:
            continue
        average_latency = df[df["TrainingSize"] == ts]["Cycles"].mean()
        print(f"{ts:4} |> {average_latency:4.4} cycles")
    print()

    if not plot:
        return

    fig = plt.figure(figsize=(12, 6))

    value_counts = df["Cycles"].value_counts()
    df["Size"] = [value_counts[cycle] for cycle in df["Cycles"]]
    df['Size'] = df['Size'].apply(lambda x: 50 + (x - df['Size'].min()) * (75 - 50) / (df['Size'].max() - df['Size'].min()))

    y_max = df[df["TrainingSize"] == 0].mean() + df[df["TrainingSize"] == 0].std()
    y_max = y_max["Cycles"]

    plt.scatter(df["TrainingSize"], df["Cycles"], s=df["Size"])

    plt.ylim(0, y_max)
    plt.xlim(-5, df["TrainingSize"].max())
    plt.xlabel("Training Size")
    plt.ylabel("Cycles")
    plt.title("Evidence of Next-Line Prefetching")

    plt.show()
    fig.savefig("figs/next_line.pdf")
    plt.close(fig)

def stride(ram_bound: float, *, plot=False, max_lines=20):
    data = pd.DataFrame()

    strides = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 32]

    for stride in strides:
        df = pd.read_csv(f"results/next_line/{stride}.csv")
        df = df[df["Cycles"] <= ram_bound]
       
        # Group by TrainingSize and calculate mean of Cycles
        result = df.groupby('TrainingSize')['Cycles'].mean().reset_index()
        
        # Rename the column to make it clear it's an average
        result = result.rename(columns={'Cycles': f"{stride}"})
        
        # Sort by TrainingSize for clarity
        result = result.sort_values('TrainingSize')

        data = pd.concat([data, result[f'{stride}']], axis=1)

    data.index.name = "TrainingSize"
    data.reindex(index=data.index[::-1])
    print(data)

    fig = plt.figure(figsize=(12, 6))
    sns.heatmap(data, cmap="inferno_r")

    # Add labels
    plt.title('Training Cycles Heatmap')
    plt.ylabel('Training Size')
    plt.xlabel('Stride')

    plt.show()
    fig.savefig("figs/stride_heatmap.pdf")
    plt.close(fig)

    return

def occupancy(bounds: BoundChecker):
    file_match = re.compile(r"(\d+)?.*_O1_CPU4_S([0-9]+)")
    
    os.makedirs("figs/occupancy", exist_ok=True)

    for file in os.listdir("results/occupancy"):
        matches = file_match.findall(file)
        if len(matches) != 1: 
            continue

        (warmup, s) = matches[0]
        if warmup == '':
            warmup = 0
        else:
            warmup = int(warmup)
        s = int(s)

        # get data 
        df = pd.read_csv(f"results/occupancy/{file}")
        data = df.groupby(['Iteration', 'SetIndex', 'LineIndex'])['Cycles'].mean().reset_index()
        data = data.drop(['Iteration'], axis = 1).pivot_table(index='SetIndex', columns='LineIndex', values='Cycles')
        
        expected_min = s - 10
        expected_max = s + 10

        if expected_min < 0:
            expected_max += abs(expected_min)
            expected_min = 0

        if expected_max > 511:
            expected_min -= expected_max - 511 
            expected_max = 511

        data = data.loc[df.index[expected_min:expected_max+1]]

        fig = plt.figure(figsize=(6, 4.5))
        sns.heatmap(data, vmin=50, vmax=300, cbar_kws={'label': 'Cycles'})
        plt.gca().set_xlabel("Line index")
        plt.gca().set_ylabel("Set index")
        plt.title(f"Set {s} with {warmup} warmup lines") 
        fig.savefig(f"figs/occupancy/s{s}_w{warmup}.png", dpi=600)
        plt.close(fig)


        

if __name__ == "__main__":
    (l3_lower, l3_upper, ram_bound) = timing(plot=True)
    l3_bounds = BoundChecker(l3_lower, l3_upper)
    # nlp(ram_bound, plot=True)
    # stride(ram_bound, plot=True)
    occupancy(l3_bounds)

