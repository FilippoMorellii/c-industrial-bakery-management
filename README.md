# Industrial Bakery Order & Inventory Management Engine in C

This repository implements a **time and space efficient** order, inventory, and delivery management simulation engine in pure C, built from scratch using custom **Hash Tables**, **Linked Lists**, **FEFO (First Expired, First Out) batch tracking**, and **priority-based truck loading** algorithms. Developed for the **Algorithms and Principles of Computer Science (Algoritmi e Strutture Dati)** final project at **Politecnico di Milano** (A.Y. 2023/2024), the system simulates the discrete-time operations of an industrial bakery handling dynamic recipe management, ingredient restocking with expiration dates, real-time order processing, and periodic courier dispatching under strict time and memory constraints.

---

## Project Description

The simulation operates in discrete time steps ($t \ge 0$), where each incoming input command advances the global clock by 1 time unit. The system models an automated industrial bakery managing recipes, warehouse stock, incoming client orders, and periodic courier pickup.

Key mechanics:
1. **Dynamic Recipe Catalog & Inventory Management**: The bakery offers recipes consisting of specific ingredient quantities (in grams). Ingredient stock arrives in batches (`lotto`), each defined by a quantity and an expiration timestamp. Expired batches are automatically pruned during operations.

2. **Instant Order Processing & FEFO Ingredient Selection**: Recipes are prepared instantaneously upon order arrival if sufficient non-expired ingredients exist in the warehouse. Ingredients are consumed prioritizing batches with the earliest expiration date (FEFO). If stock is insufficient, the order is placed into a pending queue (`ordini_attesa`). Restocking triggers chronological evaluation and fulfillment of pending orders.

3. **Periodic Courier Dispatch & Truck Loading**: Every $N$ time steps (defined at startup), a courier truck with capacity $C$ (in grams) arrives. Ready orders are evaluated in chronological order of arrival until the truck capacity limit is reached. The selected orders are then loaded into the truck ordered by descending total weight, breaking ties by favoring earlier arrival times.

---

## Features

- **Zero external dependencies**: Implemented purely in standard C using custom data structures and manual memory management (`malloc`/`free`).

- **Fast hash-based lookups**: Uses custom Hash Tables with chaining for $O(1)$ average time complexity when searching recipes and warehouse ingredient entries.

- **FEFO (First-Expired, First-Out) Inventory Management**: Automatically sorts and consumes ingredient batches based on expiration timestamps, clearing expired stock in real time.

- **Automated Pending Order Fulfillment**: Upon warehouse restocking (`rifornimento`), waiting orders are checked and prepared in strict arrival order as soon as ingredients become available.

- **Multi-criteria Courier Dispatching**: Selects deliverable orders chronologically up to truck capacity $C$, then sorts selected packages by weight (descending) and arrival time (chronological).

---

## Architecture & Data Structures

The system combines four core custom data structures in `main.c`:

- **Recipe Hash Table (`ricettario`, size 1507)**: Hash table using the division method on recipe names with singly linked lists for collision chaining. Each recipe node contains a linked list of required ingredients and quantities.

- **Warehouse Hash Table (`magazzino`, size 173)**: Hash table indexing ingredient names to singly linked lists of stock batches (`lotto`), automatically maintained in ascending order of expiration date for optimal FEFO retrieval.

- **Order Queues (`ordine`)**:
  - **Ready Queue (`ordini_pronti`)**: FIFO queue storing fully prepared orders awaiting courier pickup.
  - **Pending Queue (`ordini_attesa`)**: FIFO queue holding orders waiting for missing warehouse ingredients.

- **Courier Delivery Allocator**: Dynamically filters ready orders fitting within truck capacity $C$, computes total order weight as the sum of ingredient quantities, and sorts loaded orders by descending weight and ascending arrival timestamp.

---

## Supported Commands & I/O Format

The program reads initial courier configuration parameters (`periodicity capacity`) on line 1, followed by commands line-by-line from `stdin`, and prints the corresponding output to `stdout`:

| Command | Format | Expected Output |
| :--- | :--- | :--- |
| **Add Recipe** | `aggiungi_ricetta <recipe_name> <ing_1> <qty_1> ... <ing_n> <qty_n>` | `aggiunta` / `ignorato` |
| **Remove Recipe** | `rimuovi_ricetta <recipe_name>` | `rimossa` / `ordini in sospeso` / `non presente` |
| **Restock Warehouse** | `rifornimento <ing_1> <qty_1> <exp_1> ... <ing_n> <qty_n> <exp_n>` | `rifornito` |
| **Place Order** | `ordine <recipe_name> <quantity>` | `accettato` / `rifiutato` |
| **Courier Pickup** | *(Triggered automatically every $N$ time steps)* | List of loaded orders: `<arrival_time> <recipe_name> <qty>` or `camioncino vuoto` |

---

## Usage

### Compilation

Compile `main.c` using `gcc` with optimization flags enabled:

```bash
gcc -Wall -Werror -std=gnu11 -O2 main.c -o bakery_simulator
```

### Execution

Feed an input file containing a sequence of commands via standard input redirection:

```bash
./bakery_simulator < input.txt > output.txt
```

---

## Repository Structure

```text
.
├── main.c          # Complete C implementation (Hash Tables, Queues, FEFO engine, CLI parser)
├── Project_Specification.pdf        # Official project specification (Italian)
├── README.md       # Project documentation
```
