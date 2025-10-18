- Add flow handling and errors to the syntax parser

- Make errors better in general: add line numbers and file names to the token struct so that we can point to where errors happen
- Make compilation just not abruptly stop if a single error is detected