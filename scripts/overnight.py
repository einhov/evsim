import multiprocessing as mp
import subprocess as sp
import os
import time
import parse
import signal

from tqdm import tqdm

UPDATE = 1
DONE = 2
NEW = 3

queue = mp.Queue()

def execute(args):
    with open('overnight/{0}-{2}-{4} {3}-{5}-{1}.timing'.format(*args), "w+") as timing_file:
        current = mp.current_process()
        queue.put((current.name, NEW, "{0}-{2}-{4} {3}-{5}-{1}".format(*args)))

        start = time.time()
        timing_file.write(str(start) + '\n')

        proc = sp.Popen(
            [   './evsim',
                '../overnight.lua',
                str(args[0]),
                str(args[1]),
                str(args[2]),
                str(args[3]),
                str(args[4]),
                str(args[5])
            ],
            stdout=sp.PIPE,
            stderr=sp.PIPE
        )

        for line in proc.stderr:
            result = parse.parse('Generation: {}', line.decode("utf-8"))
            if(result):
                queue.put((current.name, UPDATE, int(result[0])))

        end = time.time()
        timing_file.write(str(end) + '\n')
        timing_file.write(str(end - start) + '\n')

        queue.put((current.name, DONE))

runs = []

# individual fitness door and food runs
runs.extend([
    (env, 0, "normal", pop_size, "none", 0)
    for env in ["door", "food"]
    for pop_size in [25, 50, 75]
])

# shared fitness door and food runs
runs.extend([
    (env, sim_count, "shared", pop_size, "none", 0)
    for env in ["door", "food"]
    for pop_size in [10, 50, 100]
    for sim_count in [2,10,25]
])

# individual fitness move and multi_food runs
runs.extend([
    (env, 0, "normal", pop_size_0, "normal", pop_size_1)
    for env in ["multi_food", "multi_move"]
    for pop_size_0 in [25, 50, 75]
    for pop_size_1 in [25, 50, 75]
])

# shared fitness move and multi_food runs

# prey
runs.extend([
    (env, sim_count, "shared", pop_size_0, "normal", 50)
    for env in ["multi_food", "multi_move"]
    for pop_size_0 in [10, 50, 100]
    for sim_count in [25, 50, 75]
])

# predators
runs.extend([
    (env, sim_count, "normal", 50, "shared", pop_size_1)
    for env in ["multi_food", "multi_move"]
    for pop_size_1 in [10, 50, 100]
    for sim_count in [10, 25, 50]
])

if __name__ == '__main__':
    #runs = runs[:1]
    bars = {'main': tqdm(total=len(runs), ncols=120, position=0, desc="Runs: ")}

    def handler_sigwinch(num, frame):
        print("\033c")
        for _,bar in bars.items():
            bar.refresh()
    signal.signal(signal.SIGWINCH, handler_sigwinch)

    with mp.Pool(8) as pool:
        results = []

        for run in runs:
            results.append(pool.apply_async(execute, (run,)))

        remaining = len(runs)
        while remaining > 0:
            msg = queue.get()
            if msg[1] == DONE:
                bars['main'].update()
                remaining -= 1
            elif msg[1] == UPDATE:
                bars[msg[0]].update()
            elif msg[1] == NEW:
                if msg[0] in bars:
                    bars[msg[0]].close()
                position = parse.parse('ForkPoolWorker-{}', msg[0])
                bars[msg[0]] = tqdm(total=500, ncols=120, position=int(position[0]), desc=msg[2])
                signal.signal(signal.SIGWINCH, handler_sigwinch)

        for res in results:
            res.wait()

    for _,bar in bars.items():
        bar.close()
