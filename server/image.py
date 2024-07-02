from PIL import Image
from io import BytesIO
import requests


def download_image(url: str) -> Image:
    """Download an image from a URL and return a Pillow Image object."""
    response = requests.get(url)
    response.raise_for_status()
    img = Image.open(BytesIO(response.content))
    return img


def downscale_image(image: Image, max_width: int, max_height: int) -> Image:
    """Downscale the image to fit within the specified dimensions."""
    image.thumbnail((max_width, max_height))
    return image


def save_image(image: Image, path: str) -> None:
    """Save the image to the specified path."""
    image.save(path)


def get_thumbnail_from_uri(uri: str) -> Image:
    """Fetch the thumbnail and downscale it"""
    if not uri:
        raise ValueError("No uri parameter")
    try:
        img = download_image(uri)
        downscaled = downscale_image(img, 135, 240)
        return downscaled
    except ValueError as e:
        raise ValueError(str(e))
