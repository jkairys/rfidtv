import os
import pychromecast
import sys
from typing import List, Tuple
from plexapi.server import PlexServer
from plexapi.video import Movie as PlexMovie
from pychromecast.controllers.plex import PlexController
from pychromecast.controllers.receiver import CastStatus
from loguru import logger
from flask import Flask


def get_cast(
    cast_name: str,
) -> Tuple[pychromecast.Chromecast, pychromecast.CastBrowser, PlexController]:
    """Get chromecast by name"""
    logger.debug("Searching for casts")
    casts, browser = pychromecast.get_chromecasts()
    print("Casts:")
    for cast in casts:
        print(cast.cast_info.friendly_name)
    plex_controller = PlexController()
    casts: List[pychromecast.Chromecast] = casts

    our_casts = [cast for cast in casts if cast.cast_info.friendly_name == cast_name]

    if len(our_casts) == 0:
        logger.info(f"No chromecast device found with name={cast_name}")
        sys.exit(1)
    else:
        logger.debug(f"Found cast {casts[0]}")

    cast = our_casts[0]
    cast.register_handler(plex_controller)
    logger.debug("Waiting for cast")
    cast.wait()
    logger.debug(cast.status)

    # If there's nothing playing, block_until_active will hang.
    status: CastStatus = cast.status
    if status.display_name == "Plex":
        logger.debug("Waiting until active")
        cast.media_controller.block_until_active(timeout=5)
    return cast, browser, plex_controller


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
