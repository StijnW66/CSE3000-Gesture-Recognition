from PIL import ImageFont
import constants
import data_processing
import models
import visualkeras

if __name__ == "__main__":
    features, labels = data_processing.load_and_combine_uwave()
    features, labels = data_processing.preprocess_input(features, labels)
    model = models.slam_cnn_padding(features[0].shape, constants.NUM_CLASSES_INITIAL_RAW_DATA)

    font = ImageFont.truetype("Roboto-Regular.ttf", 48)
    visualkeras.layered_view(
        model,
        to_file="model-visualisation.png",
        background_fill=(0, 0, 0, 1),
        legend=True, 
        font=font,
        font_color="white", 
        spacing=25,
        max_xy=1000,
        max_z=125,
        scale_xy=32).show()
