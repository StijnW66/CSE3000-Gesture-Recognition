import sys
import os

if sys.argv[1] == "plot":
    inputfile = sys.argv[2]

    # recompile the pipeline
    os.system("cd receiver-desktop && make && cd ..")

    # run the pipeline
    os.system("./receiver-desktop/main " + inputfile + " output.txt")

    # display
    os.system("python data_collection/display_files.py output.txt")

elif sys.argv[1] == "build-plot":
    inputfile = "input.txt"

    os.system("python data_collection/display_pickled_files.py")

    # recompile the pipeline
    os.system("cd receiver-desktop && make && cd ..")

    # run the pipeline
    os.system("./receiver-desktop/main " + inputfile + " output.txt")

    # display results
    os.system("python data_collection/display_files.py output.txt")