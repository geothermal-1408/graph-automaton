# Graph Automata

Generate graph just like cellular automata

## Requirements

- C compiler (tested with `clang`)
- `make`
- [Graphviz](https://graphviz.org/) (`neato` command available in `PATH`)

## Build

To build the executable:

```sh
make graph
```

(uses [makefile](makefile))

## Run and Render

To generate `.dot` files and convert them to PNGs:

```sh
make render
```

This will:

- Run `./graph` to produce `png_file*.dot`
- Convert them to PNGs into `png_store/`
- Move the `.dot` files into `dot_files/`

## Example Output

An example rendered graph is shown in `assets/png_file6.png`:

![Current Result](assets/png_file6.png)

## Reference

- https://paulcousin.net/graph-rewriting-automata
