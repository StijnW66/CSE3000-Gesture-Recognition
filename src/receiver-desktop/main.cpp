#include "GRPreprocessingPipeline.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char * argv[]) {

    if (argc != 3) {
        cout << "INCORRECT: Provide an input and an output file name\n";
        return 1;
    }

    uint16_t rawData[NUM_PDs][GESTURE_BUFFER_LENGTH];
    uint16_t thresholds[NUM_PDs];
    int gestureSignalLength = 0;
    float (* output)[ML_DATA_LENGTH];

    ifstream inFile;
    inFile.open(argv[1]);

    inFile >> gestureSignalLength;
    for (size_t i = 0; i < NUM_PDs; i++)
    {
        inFile >> thresholds[i];
    }

    for (size_t i = 0; i < gestureSignalLength; i++)
    {
        for (size_t di = 0; di < NUM_PDs; di++)
        {
            inFile >> rawData[di][i];
        }   
    }

    inFile.close();

    GRPreprocessingPipeline pipe;
    pipe.RunPipeline(rawData, gestureSignalLength, thresholds, 100);
    output = pipe.getPipelineOutput();

    std::cout << "Pipeline Done\n";

    fstream outFile;
    outFile.open(argv[2], ios::out);

    for (size_t i = 0; i < ML_DATA_LENGTH; i++)
    {
        for (size_t di = 0; di < NUM_PDs; di++)
        {
            outFile << output[di][i] << " ";
        }   
        outFile << "\n";
    }

    outFile.close();
}