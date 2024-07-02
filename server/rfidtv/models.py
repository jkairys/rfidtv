from pydantic import BaseModel
from typing import Optional


class Movie(BaseModel):
    title: str
    duration_mins: int
    thumb: str


class Card(BaseModel):
    uid: str
    title: Optional[str]
