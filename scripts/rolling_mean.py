import sys
import pandas as pd

with open("scores") as scores:
    goal_counts = [score.split()[int(sys.argv[1])] for score in scores]
    df = pd.DataFrame({'in_goal': goal_counts}).rolling(window=55, center=True).mean()
    for row in df.itertuples():
        print("{} {}".format(row[0], row[1]))
