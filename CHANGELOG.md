# Changelog

## 0.3.0
- Variable scope, positional arguments
- Implemented `$@` and `$#`
- License with GPLv3.0
- Added library: `vladaviedov/c-utils`
- Replaced old parser with a Flex/Yacc parser
- Grammar rules to parse
    - sequential lists
    - conditional lists
    - pipes
    - redirections
    - variable assignments
    - simple commands
    - async (parsed but not implemented)
- Implemented conditional lists
- Use `nanorl` (c-utils) for input handling
    - Able to process input larger than 1024 characters
- Use `git describe` for version number
- Merged absolute and relative indexing
    - Absolute indices accessible with positive numbers
    - Relative indices accessible with negative numbers
- New meta commands
    - `:hcf`
- Removed meta commands
    - `:abs` (obsolete)
- Current issues
    - Command substitution is disabled for now (nanorl issue)
    - Interrupt signal handling is hacky (nanorl issue)
    - Async lists are parsed but not implemented

## 0.2.0
- Variable and command expansion
- Implemented some `:ctx` subcommands
- `:asroot` can use both sudo and doas or environment `ASROOTCMD`
- Save command execution result (`$?`)

## 0.1.0
- Vector library
- Command entry, parsing and splitting
- Normal command lists (semicolon separated)
- Run external programs via `exec`
- New built-ins
    - `exit`
    - `cd`
    - `set`
    - `export`
    - `exec`
- New meta commands
    - `:asroot`
    - `:abs`
    - `:ctx`
