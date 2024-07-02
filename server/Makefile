# Create a virtualenv at .venv if one doesn't exist,
# activate the virtualenv, and install the package in editable mode

VENV_DIR := .venv
PYTHON := python3
SHELL := /bin/bash

.PHONY: dev

dev:
	@if [ ! -d $(VENV_DIR) ]; then \
			echo "Creating virtual environment..."; \
			$(PYTHON) -m venv $(VENV_DIR); \
			source $(VENV_DIR)/bin/activate && pip install --upgrade pip && pip install -e .; \
		else \
			echo "Use existing virtual environment..."; \
			source $(VENV_DIR)/bin/activate && pip install --upgrade pip && pip install -e .; \
		fi;
	@echo "\n\nNow run the following command:"
	@echo "source .venv/bin/activate\n"

run:
	source $(VENV_DIR)/bin/activate && source .env && python rfidtv/main.py

deploy:
	docker build -t rfidtv .
	source .env
	docker rm -f rfidtv || true
	docker run -p 8000:8000 --net=host --name rfidtv --restart=always -d -e PLEX_URI="$PLEX_URI" -e PLEX_TOKEN="$PLEX_TOKEN" -v $(shell pwd)/cards.json:/app/cards.json rfidtv