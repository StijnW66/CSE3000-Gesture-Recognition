from os import path
import matplotlib.pyplot as plt
import numpy as np
import data_processing

def plot_data_as_image(feature: np.ndarray):
    fig, ax = plt.subplots(1)

    # Turn off y-axis ticks and tick labels
    ax.set_yticks([])
    ax.set_yticklabels([])

    plt.imshow(feature.squeeze().transpose(), cmap='Greys_r', aspect=10)
    plt.colorbar(orientation='horizontal', fraction=0.10, shrink=0.4)
    plt.show()


def plot_data_as_graph(feature: np.ndarray):
    time_vals = np.arange(0, feature.shape[0])

    plt.plot(time_vals, feature[:, 0], 'r')
    plt.plot(time_vals, feature[:, 1], 'g')
    plt.plot(time_vals, feature[:, 2], 'b')
    plt.title("Photodiode Signals")
    plt.xlabel("Sample")
    plt.ylabel("Reading")
    plt.legend()
    plt.show()


if __name__ == "__main__":
    SELECTED_FEATURE = 2040

    features_processed, labels_processed = data_processing.load_and_combine_data(num_to_process=29)
    data_processing.print_feature_label_pair(features_processed, labels_processed, SELECTED_FEATURE)
    plot_data_as_image(features_processed[SELECTED_FEATURE])
    plot_data_as_graph(features_processed[SELECTED_FEATURE])

    features_raw, labels_raw = data_processing.load_and_combine_data(path.join("data", "raw_data"), num_to_process=29)
    data_processing.print_feature_label_pair(features_raw, labels_raw, SELECTED_FEATURE)
    plot_data_as_image(features_raw[SELECTED_FEATURE])
    plot_data_as_graph(features_raw[SELECTED_FEATURE])
