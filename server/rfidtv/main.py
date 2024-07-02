import os
import pychromecast
import sys
from typing import List, Tuple

from plexapi.server import PlexServer
from plexapi.video import Movie
from pychromecast.controllers.plex import PlexController
from pychromecast.controllers.receiver import CastStatus
from PIL import Image
from io import BytesIO
import requests
import rfidtv.plex
import rfidtv.card_database

import rfidtv.cast as cast_controller
import rfidtv.image

from loguru import logger

import base64
from flask import Flask, request, jsonify, send_file
import json


PLEX_TOKEN = os.getenv("PLEX_TOKEN")
PLEX_URI = os.getenv("PLEX_URI")
CAST_NAME = "Office display"

debounce = {"last_uid": None}


server = PlexServer(baseurl=PLEX_URI, token=PLEX_TOKEN)
app = Flask(__name__)


cast, browser, plex_controller = cast_controller.get_cast(CAST_NAME)


@app.route("/thumb", methods=["GET"])
def get_thumbnail():
    """Fetch the thumbnail and downscale it"""
    uri = base64.b64decode(request.args["uri"]).decode("utf-8")

    if not uri:
        return jsonify({"error": "No uri parameter"}), 404
    try:
        thumbnail: Image = image.get_thumbnail_from_uri(uri)
        img_io = BytesIO()
        thumbnail.save(img_io, format="JPEG")
        img_io.seek(0)
        return send_file(img_io, mimetype="image/jpg")
    except ValueError as e:
        return jsonify({"error": str(e)}), 404


@app.route("/movies/<movie_name>", methods=["GET"])
def get_movie(movie_name: str):
    """Endpoint to play a movie on Chromecast."""
    if not movie_name:
        return jsonify({"error": "Invalid request. 'movie_name' is required."}), 400

    try:
        movie, plex_movie = plex.fetch_movie(server, movie_name)

        return json.loads(movie.model_dump_json())
    except ValueError as e:
        return jsonify({"error": str(e)}), 404


@app.route("/play", methods=["POST"])
def play_movie():
    """Endpoint to play a movie on Chromecast."""
    data = request.json
    if not data or "movie_name" not in data:
        return jsonify({"error": "Invalid request. 'movie_name' is required."}), 400

    movie_name = data["movie_name"]
    try:

        movie, plex_movie = plex.fetch_movie(server, movie_name)
        logger.debug(f"Playing movie {movie} on chromecast")
        cast_controller.play_on_chromecast(
            plex_controller=plex_controller, movie=plex_movie
        )
        return jsonify({"status": "playing", "movie": movie_name, "cast": cast.name})
    except ValueError as e:
        return jsonify({"error": str(e)}), 404


@app.route("/status", methods=["GET"])
def get_status():
    """Get the status of the chromecast"""
    return jsonify(cast_controller.cast_status(cast))


@app.route("/pause", methods=["POST"])
def pause():
    """Endpoint to pause playback on Chromecast."""
    # cast, browser, plex_controller = get_cast(CAST_NAME)
    if plex_controller.status.player_is_playing is True:
        plex_controller.pause()
        return jsonify({"status": "paused"})
    else:
        return jsonify({"error": "No active playback found."}), 400


@app.route("/resume", methods=["POST"])
def resume():
    """Endpoint to resume playback on Chromecast."""
    if plex_controller.status.player_is_paused is True:
        plex_controller.play()
        return jsonify({"status": "resumed"})
    else:
        return jsonify({"error": "No paused playback found."}), 400


@app.route("/stop", methods=["POST"])
def stop():
    """Endpoint to resume playback on Chromecast."""
    if plex_controller.status.player_is_playing is True:
        plex_controller.stop()
        return jsonify({"status": "stopped"})
    else:
        return jsonify({"error": "No paused playback found."}), 400


@app.route("/register-card", methods=["POST"])
def register_card():
    """Endpoint to register a new card"""
    data = request.json
    if not data or "uid" not in data or "title" not in data:
        logger.error(f"Invalid card registration request, missing uid or title: {data}")
        return (
            jsonify({"error": "Invalid request. 'uid' and 'title' are required."}),
            400,
        )

    card = card_database.Card(uid=data["uid"], title=data["title"])
    try:
        card_database.register_card(card)
        return jsonify({"status": "registered", "card": card.model_dump()})
    except ValueError as e:
        return jsonify({"error": str(e)}), 400


@app.route("/cards/<uid>", methods=["POST"])
def use_card(uid: str):
    """Endpoint to use a card."""
    try:
        card = card_database.lookup_card(uid)
        movie, plex_movie = plex.fetch_movie(server, card.title)
        response = 200
        if uid != debounce["last_uid"]:
            logger.info(f"Sending play command for movie {movie}")
            cast_controller.play_on_chromecast(
                plex_controller=plex_controller, movie=plex_movie
            )
            debounce["last_uid"] = uid
        else:
            logger.debug(
                f"Received the same UID as last time ({uid}), not sending play command to chromecast"
            )
            response = 201
        return jsonify(movie.model_dump()), response
    except card_database.UnregisteredCard:
        return jsonify({"error": "Unregistered"}), 404
    except Exception as exc:
        logger.exception(f"Failed to use card {uid} - {exc}")
        return jsonify({"error": str(exc)}), 500
    finally:
        debounce["last_uid"] = uid


def cli():
    app.run(host="0.0.0.0", port=8000, debug=True)


if __name__ == "__main__":
    # Run Flask app
    cli()
