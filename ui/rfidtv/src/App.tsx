import React from "react";
import logo from "./logo.svg";
import "./App.css";
import Cards from "./cards";
import { ThemeProvider, createTheme } from "@mui/material/styles";
import CssBaseline from "@mui/material/CssBaseline";

const theme = createTheme();

function App() {
  return (
    <div className="App">
      <ThemeProvider theme={theme}>
        <Cards />
      </ThemeProvider>
    </div>
  );
}

export default App;
