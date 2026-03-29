# face

**A build system written in a single C source file. 1,041 lines. Replaces `make` for every project in this repository — and probably for yours too.**

## Why face Exists

`make` is a fifty-year-old program with fifty years of accumulated edge cases, portability nightmares, and recursive-make antipatterns. It works, in the same way that a bridge held together with duct tape works — you cross it every day, you try not to think about it, and you never look down.

face is the build tool you get when you throw all of that away and start from the only question that matters: what does a build system actually need to do? It needs to read a dependency graph, compare timestamps, and run commands. That's it. Everything else is ceremony.

## Features

Everything you actually use in a Makefile, nothing you don't:

- **Variables**: `=`, `?=`, `:=`, `+=`, and substitution references `$(VAR:.c=.o)`
- **Automatic variables**: `$@`, `$<`, `$^`, `$(FACE)`
- **Pattern rules**: `%.o: %.c` with full stem matching
- **Parallel builds**: `-j N` runs independent targets concurrently
- **`.PHONY` targets**
- **Command prefixes**: `@` for silent, `-` for error-tolerant
- **Line continuation** with `\`
- **Timestamp-based rebuilding**

## Building

```bash
make
```

Or, if you already have face:

```bash
face -f Faceplica
```

## Usage

```bash
face                    # build the default target
face scopus             # build a named target
face -C dir             # change directory before building
face -f file target     # use a specific build file
face -j N target        # parallel build with N workers
face -s target          # silent mode
face VAR=value          # override a variable
```

Searches for `Facefile`, `facefile`, `Makefile`, `makefile`, in that order.

## The Rust Port

A complete, faithful Rust translation lives in `cancer/`. Zero external dependencies. Full feature parity with the C implementation.

## License

Free. Public domain. Use however you like.
