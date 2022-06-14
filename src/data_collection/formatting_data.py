import glob
import pickle
import os.path
from os.path import split, dirname
import numpy as np
import subprocess

class FormatData():
    def __init__(self):
        self.base_paths = []
        # self.pass_through_pipelie()
        # self.convert_processed_files()

    def convert_processed_files(self):
        """
        This reads through the data that was passed through the pipeline and converts it back to the original format
        """

        # Get a list of all the paths
        self.base_paths = []
        for directory in glob.iglob("./post_process/interpolated_data/src/data_collection/data" + '**/**/**/candidate*', recursive=True):
            self.base_paths.append(directory)

        for path in self.base_paths:
            head, tail = os.path.split(path)
            pre_path = "./reformatted"
            filenames = os.listdir(path)
            data_to_pickle = []

            # Get the data from the files
            for filename in filenames:
                data = np.loadtxt(path + "/" + filename)
                data_to_pickle.append(data)

            new_path = f"{pre_path}/{head}"
            if (not os.path.exists(new_path)):
                os.makedirs(new_path)

            # Save the data back into pickled files
            with open(f"{new_path}/{tail}.pickle", "wb+") as f:
                for data in data_to_pickle:
                    pickle.dump(data, f)





    def pass_through_pipelie(self):
        """
        This reads through the data that needs to be passed through the pipeline and converts a copy of it to the format
        that the pipeline expects then passes this new data through the pipeline
        """

        # Get all the paths for data we want to pass through to pipeline
        self.base_paths = []
        for directory in glob.iglob("./interpolated_data/src/data_collection/data" + '**/**/*_hand', recursive=True):
            self.base_paths.append(directory)

        # Path to prepend
        pre_dir = "./interpolated_preprocessed"

        # Go through every path
        for base_path in self.base_paths:
            # Get all files in the path
            for filename in glob.iglob(f'{base_path}/*.pickle', recursive=True):
                # Extract all the data from the file
                with open(filename, 'rb') as f:
                    self.unpickled = []
                    while True:
                        try:
                            self.unpickled.append(pickle.load(f))
                        except EOFError:
                            break

                # Get the candidate number from the path
                candidate = split(filename)[1].replace(".pickle", "")
                path_dir = dirname(filename)

                # Get the threshold values from the control path
                medians = self.get_baselines(candidate)

                # Create a new path where the reformatted unprocessed data will be saved
                new_path = pre_dir+path_dir+"/"+candidate

                # Create a new path where the processed data will be saved
                post_process_path = "./post_process"+path_dir+"/"+candidate

                if (not os.path.exists(new_path)):
                    os.makedirs(new_path)

                if (not os.path.exists(post_process_path)):
                    os.makedirs(post_process_path)

                for i, iteration in enumerate(self.unpickled):
                    # Save the pickled data in the format that the pipeline expects
                    with open(f"{new_path}/iteration_{i}.txt", 'w') as f:
                        f.write(str(len(iteration)))
                        f.write("\n")
                        f.write(f"{int(medians[1])} {int(medians[0])} {int(medians[2])}")
                        for row in iteration:
                            f.write("\n")
                            f.write(f"{row[1]} {row[0]} {row[2]}")

                    # Run the pipeline on the reformatted unprocessed data that gets saved in the post_process path
                    subprocess.run(["../receiver-desktop/main.exe", f"../data_collection{new_path}/iteration_{i}.txt", f"{post_process_path}/iteration_{i}.txt"])


    def get_baselines(self, candidate):
        with open(f"./src/data_collection/data/control/{candidate}.pickle", 'rb') as f:
            data = pickle.load(f)
            medians = np.median(data, 0)
        return medians



if __name__ == '__main__':
    FormatData()