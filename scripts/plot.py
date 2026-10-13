#!/usr/bin/env python3

import csv
import os
import sys
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

CSV_PATH = os.path.join(os.path.dirname(__file__), "..", "results", "tables", "bench.csv")
PLOTS_DIR = os.path.join(os.path.dirname(__file__), "..", "results", "plots")


def load_data(path):
    data = defaultdict(lambda: {"N": [], "time": []})
    with open(path, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            name = row["type"]
            data[name]["N"].append(int(row["N"]))
            data[name]["time"].append(float(row["time_ms"]))
    return data


def plot_group(data, names, title, output, colors):
    fig, ax = plt.subplots(figsize=(10, 6))

    for name, color in zip(names, colors):
        if name not in data:
            continue
        xs = data[name]["N"]
        ys = data[name]["time"]
        ax.plot(xs, ys, marker="o", label=name, color=color, linewidth=2)

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("N (number of operations)")
    ax.set_ylabel("Time (ms)")
    ax.set_title(title)
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.legend()

    fig.tight_layout()
    fig.savefig(output, dpi=120)
    plt.close(fig)
    print(f"Saved: {output}")


def plot_all(data, output):
    fig, ax = plt.subplots(figsize=(11, 7))

    colors = {
        "UnqPtr":     "#1f77b4",
        "RawUnique":  "#000000",
        "StdUnique":  "#aec7e8",
        "ShrdPtr":    "#d62728",
        "StdShared":  "#ff9896",
        "SmrtPtr":    "#2ca02c",
    }

    for name, series in data.items():
        color = colors.get(name, None)
        ax.plot(series["N"], series["time"],
                marker="o", label=name, color=color, linewidth=2)

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("N (number of operations)")
    ax.set_ylabel("Time (ms)")
    ax.set_title("All smart pointers — time vs N")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.legend(loc="upper left")

    fig.tight_layout()
    fig.savefig(output, dpi=120)
    plt.close(fig)
    print(f"Saved: {output}")


def main():
    if not os.path.isfile(CSV_PATH):
        print(f"CSV file not found: {CSV_PATH}")
        print("Run ./smart_pointers_bench first.")
        sys.exit(1)

    os.makedirs(PLOTS_DIR, exist_ok=True)

    data = load_data(CSV_PATH)

    plot_group(
        data,
        names=["UnqPtr", "RawUnique", "StdUnique"],
        title="unique_ptr family — time vs N",
        output=os.path.join(PLOTS_DIR, "unique_ptr.png"),
        colors=["#1f77b4", "#000000", "#aec7e8"],
    )

    plot_group(
        data,
        names=["ShrdPtr", "StdShared", "SmrtPtr"],
        title="shared_ptr family — time vs N",
        output=os.path.join(PLOTS_DIR, "shared_ptr.png"),
        colors=["#d62728", "#ff9896", "#2ca02c"],
    )

    plot_all(data, os.path.join(PLOTS_DIR, "all.png"))


if __name__ == "__main__":
    main()
