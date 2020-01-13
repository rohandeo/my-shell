Instructions to run.
- Unzip contents of tar file
- Open bash terminal and use 'make run' to build and run the shell
- Use 'make clean' to delete the output file

Using the custom shell
- Use 'exit' to exit the sheel and return control back to bash
- You can use any bash commands with a few exceptions like 'cd', 'help', etc (since execvp does not implement these commands)

A few caveats:
- The background processes do not work as intended. Their order is jumbled up