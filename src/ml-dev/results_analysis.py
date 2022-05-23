from unittest import result
from requests import head
from rich.table import Table
from typing import List
import constants
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf

def plot_losses(history: tf.keras.callbacks.History):
    loss = history.history['loss']
    val_loss = history.history['val_loss']

    epochs = range(1, len(loss) + 1)

    plt.plot(epochs, loss, 'g.', label='Training loss')
    plt.plot(epochs, val_loss, 'b', label='Validation loss')
    plt.title('Training and validation loss')
    plt.xlabel('Epochs')
    plt.ylabel('Loss')
    plt.legend()
    plt.show()


def plot_accuracies(history: tf.keras.callbacks.History):
    acc = history.history['sparse_categorical_accuracy']
    val_acc = history.history['val_sparse_categorical_accuracy']

    epochs = range(1, len(acc) + 1)

    plt.plot(epochs, acc, 'g.', label='Training accuracy')
    plt.plot(epochs, val_acc, 'b', label='Validation accuracy')
    plt.title('Training and validation accuracies')
    plt.xlabel('Epochs')
    plt.ylabel('Accuracy')
    plt.legend()
    plt.show()


def print_kfold_results(acc_per_fold: List[float], loss_per_fold: List[float]):
    """
    Print the results of k-fold cross validation to the console

    Args:
        acc_per_fold: A list with the testing accuracy computed for each fold
        loss_per_fold: A list with the testing loss computed for each fold
    """
    num_folds = len(acc_per_fold)
    
    constants.CONSOLE.rule(f"[bold cyan]{num_folds}-fold Validation Summary")
    constants.CONSOLE.print('Score per fold:', style="bold deep_pink4")
    for i in range(0, num_folds):
        print(f'> Fold {i+1} - Loss: {loss_per_fold[i]:.3f} - Accuracy: {acc_per_fold[i]:.3f}%')
    print()
    constants.CONSOLE.print("Average scores for all folds:", style="bold deep_pink4")
    print(f'> Accuracy: {np.mean(acc_per_fold):.3f}% (+- {np.std(acc_per_fold):.3f}%)')
    print(f'> Loss: {np.mean(loss_per_fold):.3f}')


def print_multiple_kfold_results(model_names: List[str], 
                                 accuracies: List[List[float]],
                                 losses: List[List[float]],
                                 quantized_sizes: List[int]):
    num_folds = len(accuracies[0])
    
    results = Table(title=f"{num_folds}-fold Comparison Results",
                    title_style="bold cyan",
                    show_header=True,
                    header_style="bold deep_pink4")
    results.add_column("Model")
    results.add_column("Accuracy")
    results.add_column("Loss")
    results.add_column("Quantized Size (Bytes)")

    for model_num in range(len(model_names)):
        results.add_row(
            model_names[model_num],
            f"{np.mean(accuracies[model_num]):.3f}% (+- {np.std(accuracies[model_num]):.3f}%)",
            f"{np.mean(losses[model_num]):.3f} (+- {np.std(losses[model_num]):.3f})",
            f"{quantized_sizes[model_num]}"
        )

    constants.CONSOLE.print(results)
