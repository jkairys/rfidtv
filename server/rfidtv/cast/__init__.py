from typing import List
from pychromecast import Chromecast, CastBrowser
import pychromecast
from loguru import logger
from .controller import get_cast, play_on_chromecast, cast_status


casts: List[Chromecast] = []
browser: CastBrowser = None


def list_casts() -> List[Chromecast]:
    return casts


def load_casts():
    global casts
    global browser
    logger.debug("Loading chromecasts")
    _casts, _browser = pychromecast.get_chromecasts()
    logger.info(f"Found {len(_casts)} chromecasts")
    casts.clear()
    for idx, cast in enumerate(_casts):
        assert isinstance(cast, Chromecast)
        logger.debug(
            f"cast<{cast.cast_info.host}>: {cast.cast_info.friendly_name}, model={cast.cast_info.model_name}"
        )
        casts.append(cast)
    browser = _browser
