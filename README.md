# BYTE — A Custom Shell in C++
BYTE is a lightweight, feature-rich custom shell implemented in C++ designed to mimic and extend traditional UNIX shell functionalities. Built from scratch using low-level system calls like `fork()`, `execvp()`, and `chdir()`, BYTE not only supports essential built-in commands but also introduces several *custom enhancements* aimed at improving your workflow, navigation, and scripting efficiency.

---

## Technologies Used

- **Language:** C++
- **System API:** POSIX (Linux syscalls like `fork()`, `execvp()`, `mkdir()`, etc.)
- **Operating System:** Tested on Ubuntu / WSL
- **Terminal Support:** ANSI escape codes for colored prompt

---

## Features

### Built-in Commands

BYTE replicates standard shell built-ins with C++ logic:

| Command      | Description                                                   |
|--------------|---------------------------------------------------------------|
| `cd <path>`  | Changes the current working directory                         |
| `exit`       | Exits the BYTE shell                                          |
| `history`    | Shows history of last used commands                           |
| `help`       | Displays help message for all available commands              |
| `echo`       | Prints the input string                                       |
| `alias`      | Create command shortcuts (aliases), persists across sessions  |
| `unalias`    | Remove saved aliases                                          |
| `let`        | Evaluates simple arithmetic expressions (e.g. `let 3+5*2`)    |
| `logout`     | Same as `exit`                                                |
| `read`       | Reads user input (alias-like behavior)                        |
| `printf`     | Prints formatted output (basic support)                       |
| `type`       | Tells whether a command is built-in, custom, or external      |
| `ulimit`     | Placeholder for user resource limits (not implemented fully)  |
| `enable`     | Reserved for future expansion                                 |
| `mapfile`    | Reads lines into an array (not yet implemented)              |
| `typeset`    | Placeholder (mirroring Bash-style typed variables)            |

---

### Custom BYTE Commands

These commands are unique to BYTE and can be accessed using the `byte-` prefix:

| Command                | Usage Example                       | Description                                                                            |
|------------------------|-------------------------------------|----------------------------------------------------------------------------------------|
| `byte-mkdir`           | `byte-mkdir /tmp/myfolder`          | Create folders elsewhere without navigating away                                      |
| `byte-calc`            | `byte-calc 5*(2+3)`                 | Evaluate complex arithmetic expressions                                               |
| `byte-lastdir`         | `byte-lastdir`                      | Jump to the last directory where you executed a command (excluding `cd`)             |
| `byte-bookmark`        | `byte-bookmark add docs`            | Save current path as bookmark with name `docs`                                       |
| `byte-bookmark`        | `byte-bookmark go docs`             | Jump to bookmark `docs`                                                               |
| `byte-bookmark`        | `byte-bookmark list`                | List all saved bookmarks                                                              |

All these are persistently stored in hidden files:
- `.byte_history`
- `.byte_aliases`
- `.byte_bookmarks`
- `.byte_lastdir`

This ensures your state survives after restarting the shell.

---

## User Interface

The shell prompt is styled for clarity and visual separation:

```bash
BYTE@/your/current/directory >>
````

* **Green** for "BYTE"
* **Blue** for your current directory
* **Grey** user input after `>>`

---

## How to Install

### Prerequisites:

* Linux or WSL (Windows Subsystem for Linux)
* C++ Compiler (`g++`)

### Installation Steps:

```bash
# Clone the repo
git clone https://github.com/idtsitsharsh/nebulaSubmission.git

# Go into the directory
cd nebulaSubmission

# Compile the shell
g++ main.cpp -o byte
```

### Run the Shell

```bash
./byte
```

---

## Command Examples

```bash
BYTE@/home/user >> cd projects
BYTE@/home/user/projects >> byte-mkdir /tmp/newdir
BYTE@/home/user/projects >> byte-bookmark add proj
BYTE@/home/user/projects >> byte-calc 12 / (2 + 4)
BYTE@/home/user/projects >> alias home='cd ~'
BYTE@/home/user/projects >> type cd
cd is a shell builtin
```

---

## Future Additions

* Auto-suggestions and autocomplete
* Command piping and redirection support
* Enhanced `ulimit`, `enable`, `mapfile`, and scripting abilities

---

## Credits & Contact

This project was created as a hands-on deep dive into how UNIX-style shells work internally.
Maintained by **[idtsitsharsh](https://github.com/idtsitsharsh)**.
Feel free to open issues or suggest improvements via GitHub or reach out!

---
