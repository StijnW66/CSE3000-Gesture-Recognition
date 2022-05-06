from typing import List, Tuple
from data_processing import NUM_CLASSES
import tensorflow as tf

def _vgg_block(num_convs: int, num_channels: int) -> tf.keras.Model:
    """
    Construct a VGG CNN block as outlined in (https://d2l.ai/chapter_convolutional-modern/vgg.html#vgg-blocks)

    Args:
        num_convs: Number of convolutional layers in the block
        num_channels: Number of output channels of each convolutional layer

    Returns:
        A VGG-style CNN block as a Keras model
    """
    blk = tf.keras.models.Sequential()
    for _ in range(num_convs):
        blk.add(tf.keras.layers.Conv2D(num_channels,kernel_size=3,
                                    padding='same',activation='relu'))
    blk.add(tf.keras.layers.MaxPool2D(pool_size=2, strides=2))
    return blk

def vgg(input_shape: Tuple, conv_arch: List[Tuple[int, int]]) -> tf.keras.Model:
    """
    Construct a VGG-style CNN as outlined in (https://d2l.ai/chapter_convolutional-modern/vgg.html#vgg-network)

    Args:
        input_shape: Shape of the CNN input
        conv_arch: List of pairs corresponding to the number of convolutions and output channels
        of each VGG block in the CNN
    
    Returns:
        A VGG-style CNN as a Keras model
    """
    net = tf.keras.models.Sequential([tf.keras.Input(shape=input_shape, name="sensor image")])

    # The convolutional part
    for (num_convs, num_channels) in conv_arch:
        net.add(_vgg_block(num_convs, num_channels))

    # The fully-connected part
    net.add(tf.keras.models.Sequential([
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(4096, activation='relu'),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(4096, activation='relu'),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(NUM_CLASSES)]))
    return net


def keras_mnist_example(input_shape: Tuple) -> tf.keras.Model:
    """
    Based on CNN example from Keras documentation, originally for use with MNIST
    (https://keras.io/examples/vision/mnist_convnet/).

    Args:
        input_shape: Shape of the CNN input

    Returns:
        A trimmed version of the MNIST example CNN
    """
    return tf.keras.models.Sequential([
        tf.keras.Input(shape=input_shape, name="sensor image"),
        tf.keras.layers.Conv2D(32, kernel_size=(3, 3), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(NUM_CLASSES, activation="softmax", name="predictions"),
    ])

def slam_cnn(input_shape: Tuple) -> tf.keras.Model:
    return tf.keras.models.Sequential([
        tf.keras.Input(shape=input_shape, name="sensor image"),
        tf.keras.layers.Conv2D(32, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(32, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(16, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(NUM_CLASSES, activation="softmax", name="predictions"),
    ])

def slam_cnn_padding(input_shape: Tuple) -> tf.keras.Model:
    return tf.keras.models.Sequential([
        tf.keras.Input(shape=input_shape, name="sensor image"),
        tf.keras.layers.ZeroPadding2D(padding=(0, 2)),
        tf.keras.layers.Conv2D(32, kernel_size=(3, 3), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(NUM_CLASSES, activation="softmax", name="predictions"),
    ])
