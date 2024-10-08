import pychromecast
import sys
from typing import List, Tuple
from plexapi.video import Movie as PlexMovie
from pychromecast.controllers.plex import PlexController
from pychromecast.controllers.receiver import CastStatus
from loguru import logger


def get_cast(
    cast_name: str,
) -> Tuple[pychromecast.Chromecast, PlexController]:
    """Get chromecast by name"""
    logger.debug(f"Searching for cast with name {cast_name}")

    casts, _ = pychromecast.get_listed_chromecasts(friendly_names=[cast_name])

    plex_controller = PlexController()

    if len(casts) == 0:
        logger.info(f"No chromecast device found with name={cast_name}")
        sys.exit(1)
    else:
        logger.debug(f"Found cast {casts[0]}")

    cast: pychromecast.Chromecast = casts[0]
    cast.register_handler(plex_controller)
    logger.debug("Waiting for cast")
    cast.wait()
    logger.debug(cast.status)

    # If there's nothing playing, block_until_active will hang.
    status: CastStatus = cast.status
    if status.display_name == "Plex":
        logger.debug("Waiting until active")
        cast.media_controller.block_until_active(timeout=5)
    return cast, plex_controller


def play_on_chromecast(
    plex_controller: PlexController,
    movie: PlexMovie,
) -> Tuple[pychromecast.Chromecast, pychromecast.CastBrowser]:
    """Play the movie on Chromecast."""
    # cast, browser, plex_controller = get_cast(CAST_NAME)
    plex_controller.block_until_playing(movie)


def cast_status(cast: pychromecast.Chromecast) -> dict:
    """Get the status of the Chromecast as a dict"""
    return {
        "is_playing": (
            cast.media_controller.is_playing
            if hasattr(cast.media_controller, "is_playing")
            else False
        ),
        "player_state": cast.media_controller.status.player_state,
        "duration": cast.media_controller.status.duration,
        "current_time": cast.media_controller.status.current_time,
        "title": cast.media_controller.status.title,
        "images": cast.media_controller.status.images,
        "thumbnail": (
            cast.media_controller.thumbnail
            if hasattr(cast.media_controller, "thumbnail")
            else ""
        ),
    }
