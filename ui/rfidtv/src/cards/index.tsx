import React, { useEffect, useState } from "react";
import {
  TextField,
  Button,
  Container,
  Typography,
  Paper,
  Box,
  Grid,
  FormControl,
  OutlinedInput,
  InputAdornment,
  FormHelperText,
  IconButton,
} from "@mui/material";

import CreditCardIcon from "@mui/icons-material/CreditCard";
import { CreditCard } from "@mui/icons-material";
import SearchIcon from "@mui/icons-material/Search";
import LinkOffIcon from "@mui/icons-material/LinkOff";
import { CardInterface } from "./interfaces";
import NewCard from "./new-card";

const url = "http://localhost:8000";

const SettingsForm: React.FC = () => {
  const [filter, setFilter] = useState<string>("");
  const [cards, setCards] = useState<CardInterface[]>([]);
  const [showAddCardDialog, setShowAddCardDialog] = useState(false);
  const [filteredCards, setFilteredCards] = useState<CardInterface[]>([]);

  const loadCards = async () => {
    return fetch(`${url}/cards`)
      .then((response) => response.json())
      .then((data: CardInterface[]) => {
        console.log(data);
        setCards(data);
      });
  };

  useEffect(() => {
    // Load cards from the API
    loadCards();
  }, []);

  useEffect(() => {
    const flt = filter.toLowerCase();
    setFilteredCards(
      cards.filter(
        (card) =>
          card.uid.toLowerCase().includes(flt) ||
          card.title.toLowerCase().includes(flt)
      )
    );
  }, [filter, cards]);

  const createCard = async ({ card }: { card: CardInterface }) => {
    const response = await fetch("http://localhost:8000/register-card", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(card),
    });
    const data = await response.json();
    await loadCards();
    setShowAddCardDialog(false);
  };

  const deleteCard = async (uid: string) => {
    const response = await fetch(`${url}/cards/${uid}`, {
      method: "DELETE",
    });
    const data = await response.json();
    alert("Card deleted: " + JSON.stringify(data));
    setCards(cards.filter((card) => card.uid !== uid));
  };

  return (
    <Box padding={2}>
      <Typography variant="h4" component="h1" gutterBottom>
        rfidtv Card Management
      </Typography>
      {!showAddCardDialog && (
        <Box display="flex" flexDirection="row">
          <Box flexGrow="1">
            <FormControl variant="outlined" fullWidth>
              <OutlinedInput
                id="search"
                startAdornment={<SearchIcon />}
                aria-describedby="search-helper-text"
                onChange={(e) => setFilter(e.target.value)}
              />
              <FormHelperText id="search-helper-text">
                Search existing cards
              </FormHelperText>
            </FormControl>
          </Box>

          <Box flexGrow="0" padding={1}>
            <Button
              variant="contained"
              startIcon={<CreditCard />}
              onClick={() => {
                setShowAddCardDialog(true);
              }}
            >
              Add card
            </Button>
          </Box>
        </Box>
      )}
      {showAddCardDialog && (
        <NewCard
          onSubmit={({ card }) => createCard({ card })}
          onCancel={() => setShowAddCardDialog(false)}
        />
      )}
      <Grid container spacing={2} marginTop={2}>
        {filteredCards.map((card) => (
          <Grid item xs={1} sm={3} key={card.uid}>
            <Paper
              style={{ padding: 10, textAlign: "left", position: "relative" }}
            >
              <IconButton
                style={{ position: "absolute", top: 10, right: 10 }}
                onClick={() => deleteCard(card.uid)}
              >
                <LinkOffIcon />
              </IconButton>
              <h4>{card.title}</h4>
              <Box display="flex" alignItems="center">
                <CreditCardIcon />{" "}
                <Typography paddingLeft={1}>{card.uid}</Typography>
              </Box>
            </Paper>
          </Grid>
        ))}
      </Grid>
    </Box>
  );
};

export default SettingsForm;
