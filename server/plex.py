import os
import pychromecast
import sys
from typing import List, Tuple
from plexapi.server import PlexServer
from plexapi.video import Movie as PlexMovie
from pychromecast.controllers.plex import PlexController
from pychromecast.controllers.receiver import CastStatus
from models import Movie
from loguru import logger
import base64


def fetch_movie(server: PlexServer, movie_name: str) -> Tuple[Movie, PlexMovie]:
    """Fetch the movie object from Plex by name."""
    try:
        logger.debug(f"Fetching movie {movie_name}")
        plex_movie: PlexMovie = server.library.section("Movies").get(movie_name)
        return (
            Movie(
                title=plex_movie.title,
                # art_url=result.artUrl,
                thumb=f"/thumb?uri={base64.b64encode(plex_movie.thumbUrl.encode()).decode()}",
                duration_mins=int(plex_movie.duration / 1000 / 60),
            ),
            plex_movie,
        )
    except Exception as e:
        raise ValueError(f"Movie '{movie_name}' not found. Error: {str(e)}")
