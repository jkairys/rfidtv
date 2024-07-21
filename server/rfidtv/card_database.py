from rfidtv.models import Card
import json
from typing import List, Dict, Optional
from loguru import logger


class UnregisteredCard(Exception):
    pass


def load_cards() -> Dict[str, Card]:
    """Load cards from file."""
    try:
        with open("cards.json", "r", encoding="utf8") as f:
            all_cards = [Card(**card) for card in json.load(f)]
        return {c.uid: c for c in all_cards}
    except Exception as e:
        logger.error(f"Failed to load cards from file. Error: {e}")


def save_cards(cards: Dict[str, Card]):
    """Save cards to file."""
    logger.info(f"Saving cards {cards}")
    output = [json.loads(card.model_dump_json()) for card in cards.values()]
    with open("cards.json", "w", encoding="utf8") as f:
        json.dump(output, f)


mapping: Dict[str, Card] = load_cards()


def register_card(card: Card):
    """Register a card."""
    if card.uid in mapping:
        raise ValueError(f"Card with UID {card.uid} already registered")
    mapping[card.uid] = card
    save_cards(mapping)


def unregister_card(uid: str):
    """Unregister a card."""
    if uid not in mapping:
        raise ValueError(f"Card with UID {uid} not registered")
    del mapping[uid]
    save_cards(mapping)


def lookup_card(uid: Optional[str] = None) -> Card:
    """Lookup a card by its UID."""
    if uid is None:
        raise ValueError("No UID provided")
    uid = uid.upper().strip()
    if uid not in mapping:
        raise UnregisteredCard(f"Card with UID {uid} has not been registered")
    return mapping.get(uid)


def list_all_cards() -> List[Card]:
    """List all cards."""
    return [mapping.get(uid) for uid in mapping.keys()]
