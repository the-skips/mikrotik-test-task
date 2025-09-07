# Neighbour Discovery Service & CLI

This project implements a background service that discovers neighbouring devices on local networks and a CLI to query active neighbours.

## Build

Make sure you have `g++` installed. Then, run:

```bash
make
```

This will build:

* `neighbourSearchingService` — the service executable
* `neighbourCli` — the CLI executable

## Run Service

Start the service manually from the terminal:

```bash
./neighbourSearchingService
```

## Run CLI

In a separate terminal, run:

```bash
./neighbourCli
```

This will display the list of active neighbours. If no neighbours are found, it will show:

```
No active neighbours.
```
