import os
import pickle
import numpy as np


pre = "src/data_collection/data/original/triangle_343_4010040"
post = "/1500/clockwise/right_hand/data.pickle"

data = []
with open(pre + post, "rb") as f:
    while(1):
        try:
            data.append((pickle.load(f)))
        except:
            break

data0 = []

with open(pre + "A" + post, "rb") as f:
    while(1):
        try:
            data0.append((pickle.load(f)))
        except:
            break


#os.remove(pre + post)
os.remove(pre + "A" + post)

with open(pre + post, "ab+") as f:
    #pickle.dump(np.array(data), f)
    for i in data0:
        pickle.dump(i, f)