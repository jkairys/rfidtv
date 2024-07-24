import { Button, Container, Grid, TextField } from "@mui/material";
import React from "react";
import { CardInterface } from "./interfaces";

interface NewCardInterface {
  onCancel: () => void;
  onSubmit: ({ card }: { card: CardInterface }) => void;
}

const NewCard: React.FC<NewCardInterface> = ({
  onCancel,
  onSubmit,
}: NewCardInterface) => {
  const [card, setCard] = React.useState<CardInterface>({
    uid: "",
    title: "",
  });
  return (
    <Container maxWidth="md">
      <Grid container display="flex" flexDirection={"column"} spacing={3}>
        <Grid item>
          <h2>Add a new movie</h2>
        </Grid>
        <Grid item>
          <TextField
            id="uid"
            label="UID"
            variant="outlined"
            fullWidth
            value={card.uid}
            onChange={(e) =>
              setCard((existing) => {
                return { ...existing, uid: e.target.value };
              })
            }
          />
        </Grid>
        <Grid item>
          <TextField
            id="title"
            label="Movie Title"
            variant="outlined"
            fullWidth
            value={card.title}
            onChange={(e) =>
              setCard((existing) => {
                return { ...existing, title: e.target.value };
              })
            }
          />
        </Grid>

        <Grid
          item
          container
          display="flex"
          justifyContent="flex-end"
          spacing={1}
        >
          <Grid item>
            <Button variant="outlined" onClick={() => onCancel()}>
              Cancel
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="contained"
              color="primary"
              onClick={() => onSubmit({ card })}
            >
              Save
            </Button>
          </Grid>
        </Grid>
      </Grid>
    </Container>
  );
};

export default NewCard;
