import glob
import pickle
import os.path
from os.path import split, dirname
import numpy as np
import subprocess

class FormatData():
    def __init__(self):
        self.base_paths = []
        self.convert_files()

    def convert_files(self):
        for directory in glob.iglob("./src/data_collection/data" + '**/**/*_hand', recursive=True):
            self.base_paths.append(directory)

        pre_dir = "./new_data"

        count = 0

        print(self.base_paths[9])

        # for path in self.base_paths:
        for filename in glob.iglob(f'{self.base_paths[9]}/*.pickle', recursive=True):
            if count == 1:
                print(filename)
                with open(filename, 'rb') as f:
                    self.unpickled = []
                    while True:
                        try:
                            self.unpickled.append(pickle.load(f))
                        except EOFError:
                            break
                # print(filename)
                # print(split(filename))
                candidate = split(filename)[1].replace(".pickle", "")
                print(candidate)
                path_dir = dirname(filename)
                print(path_dir)
                # print(len(self.unpickled))

                medians = self.get_baselines(candidate)

                new_path = pre_dir+path_dir+"/"+candidate

                post_process_path = "./post_process"+path_dir+"/"+candidate

                if (not os.path.exists(new_path)):
                    os.makedirs(new_path)

                if (not os.path.exists(post_process_path)):
                    os.makedirs(post_process_path)

                for i, iteration in enumerate(self.unpickled):
                    if i == 0:
                        print(iteration)
                    with open(f"{new_path}/iteration_{i}.txt", 'w') as f:
                        f.write(str(len(iteration)))
                        f.write("\n")
                        f.write(f"{int(medians[1])} {int(medians[0])} {int(medians[2])}")
                        for row in iteration:
                            f.write("\n")
                            f.write(f"{row[1]} {row[0]} {row[2]}")
                    # print(new_path)
                    subprocess.run(["../receiver-desktop/main.exe", f"../data_collection{new_path}/iteration_{i}.txt", f"{post_process_path}/iteration_{i}.txt"])


            count += 1

    def get_baselines(self, candidate):
        print(candidate)
        with open(f"./src/data_collection/data/control/{candidate}.pickle", 'rb') as f:
            data = pickle.load(f)
            # print(data)
            medians = np.median(data, 0)
            print(medians)

        return medians



if __name__ == '__main__':
    FormatData()