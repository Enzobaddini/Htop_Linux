# Htop clone for Linux

A terminal process monitor for Linux, written in C. It reads the `/proc` filesystem directly (no external libraries besides ncurses) and shows per-core CPU usage, memory and swap, and a searchable, sortable, scrollable process list.

![demo](assets/demo.gif)

## Features

- **CPU:** total usage plus one value per core, computed from `/proc/stat` (refreshed every second)
- **Memory:** used memory (`MemTotal - MemAvailable`), buffers, cache and swap, from `/proc/meminfo`
- **Process list**, from `/proc/[pid]/stat`:
  - PID, PPID, state and thread count
  - name (the kernel truncates it to 15 characters)
  - user time and kernel time, in clock ticks (usually 100 per second), not seconds
  - RSS in KB
  - the CPU the process last ran on (0-based, see [Limitations](#limitations))
- **Scrolling:** arrow keys, page up/down, home/end, with a scrollbar. The CPU and memory header stays fixed while only the process list scrolls

- **Sorting:** by PID, by name (case-insensitive) or by cumulative CPU time. The chosen order is kept across refreshes
- **Search:** filter by PID prefix or by process-name prefix, typed into a clickable search bar
- **Kill:** send `SIGTERM` to the selected process, with a popup showing the result (success, permission denied, or the system error)

## Limitations

**Not implemented**

- No per-process CPU%: only cumulative CPU time is shown, so "sort by CPU time" ranks processes by total time used since they started, not by current load

- No process tree, no user or command-line columns
- `SIGKILL` is not supported
- Mouse support is limited to clicking the search bar (no mouse-wheel scrolling)
- No automated tests

**Known issues**

- `Delete` sends `SIGTERM` **immediately, with no confirmation**

- Name search only accepts letters and `_`. Digits always start a PID search, so names like `python3` or `systemd-journal` can't be typed. Names are also matched against the kernel-truncated 15-character name
- Once the search bar has focus, click anywhere outside it to leave it. `Esc` only quits when the search bar is not focused
- CPU numbering is inconsistent: the header numbers cores from 1 (`CPU 1` to `CPU 14`), while the "Last CPU" column uses the kernel's 0-based numbering, so `Last CPU: 13` is the header's `CPU 14`
- Every key press, not just the one-second tick, rescans `/proc` and redraws the screen 

**Environment**

- Linux only. Tested on Ubuntu 24.04 under WSL2 only, on a machine with 14 logical CPUs. The header grows with the core count, and machines with many more cores have not been tested

## Build and run

Requires a C compiler, `make` and the ncurses development headers.

```bash
sudo apt update
sudo apt install build-essential libncurses-dev

git clone https://github.com/Enzobaddini/Htop_Linux.git
cd Htop_Linux
make
./monitor
```

- `make` expects the sources in `src/` and the headers in `include/`, and builds a binary called `monitor`.
- `make run` builds and starts it, and `make clean` removes the binary.

- Use a terminal about 130 columns wide to see every field of each process line; longer lines are truncated.
- Killing processes owned by other users requires `sudo ./monitor`.

## Controls

| Key | Action |
|-----|--------|
| `↑` / `↓` | Move the selection one line |
| `Home` / `End` | Jump to first / last process |
| `Space` | Sort by PID |
| `Tab` | Sort by name |
| `Enter` | Sort by CPU time (highest first) |
| `/` or click on the search bar | Focus the search bar |
| digits (search focused) | Filter by PID prefix |
| letters or `_` (search focused) | Filter by name prefix (case-insensitive) |
| `Backspace` (search focused) | Delete the last character |
| click outside the search bar | Unfocus the search bar |
| `Delete` | Send `SIGTERM` to the selected process (no confirmation) |
| `Esc` | Quit (when the search bar is not focused) |

## How it works

### Layout

```
include/   headers
src/
  main.c      event loop: input -> sample -> refresh data -> draw
  cpu.c       /proc/stat parsing, usage from deltas, CPU rows
  mem.c       /proc/meminfo parsing, memory and swap rows
  pid.c       /proc directory scan, PID filtering
  process.c   /proc/[pid]/stat parsing, hash table, process rows, kill
  ui.c        scrolling, sorting, search bar, popup window
```

### Data flow

The main loop waits for a key for up to one second, so the screen refreshes on every key press and at least once per second. On each iteration it:

1. handles input (search, sort, scroll, kill)

2. samples `/proc/stat` if at least one second has passed since the last sample, and computes CPU usage against the previous sample
3. reads `/proc/meminfo`
4. scans `/proc` for numeric directory names and re-parses every PID into a hash table
5. builds the visible list by filtering and sorting PIDs
6. clears and redraws the screen

Process data lives in a hash table (separate chaining, 512 buckets, bucket = `pid % 512`). The list shown on screen is just a dynamic array of PIDs that is filtered and sorted, so sorting moves integers, not structs.

Steps 3 to 6 run on every iteration, including key presses, so holding an arrow key rescans all of `/proc` on each repeat.

### Parsing `/proc/[pid]/stat`

The second field of this file is the process name in parentheses, and it can contain spaces and even `)`. Splitting the line on whitespace breaks:

```
1234 (my prog) S 1 1234 1234 0 -1 ...
```

Splitting on spaces gives `(my` and `prog)` as two separate fields, which shifts every later field by one. Instead, the parser takes the name between the first `(` and the **last** `)` and only then tokenizes the rest of the line, counting fields from number 3:

| Field | Meaning |
|-------|---------|
| 3 | state |
| 4 | parent PID |
| 14 | user time (clock ticks) |
| 15 | kernel time (clock ticks) |
| 20 | threads |
| 24 | RSS (in pages, converted to KB) |
| 39 | CPU the process last ran on (0-based) |

For the line above, the result is name = `my prog`, state = `S`, PPID = `1`.

The name comes from the kernel's `comm` field, which is limited to 15 characters, so long names are cut: `systemd-journald` appears as `systemd-journal`, and `init-systemd(Ubuntu)` as `init-systemd(Ub`.

### Computing CPU usage

A single reading of `/proc/stat` is a counter of ticks since boot, so it says nothing about *current* usage. Usage is computed from the difference between two samples:

```
total = user + nice + system + idle + iowait + irq + softirq
idle  = idle + iowait
usage = (Δtotal − Δidle) / Δtotal × 100
```

Example with illustrative numbers:

| | total ticks | idle ticks |
|---|---|---|
| Sample 1 | 1000 | 800 |
| Sample 2 | 1100 | 850 |

Δtotal = 100 and Δidle = 50, so 50 ticks were busy: usage = 50 / 100 = **50%**. The same calculation runs for the `cpu` line (all cores combined) and for each `cpuN` line.

## Design notes and problems solved

**Processes disappear mid-refresh.** A process can exit between listing `/proc` and reading its `stat` file, so the read fails. Every refresh therefore marks all entries as stale, re-parses every PID found, and sweeps entries that were not refreshed. The PID list is then pruned against the hash table, so the UI never asks for an entry that no longer exists.

**Sort order must survive the rebuild.** The list is rebuilt from `/proc` on every refresh, which would discard a sort made earlier. The selected mode is stored (`flag`) and reapplied after every rebuild.

**The standard `qsort` comparator gets no extra context.** Sorting by name or CPU time needs the hash table to look up each PID, but the comparator only receives two element pointers. The hash table pointer is stored in a file-level variable before sorting. The trade-off is that the sort is not reentrant. glibc's `qsort_r` would avoid this by passing a context pointer, at the cost of portability (it is a GNU extension).

**Counters don't always behave.** If a counter goes backwards, or the delta between samples is zero (for example, two samples taken very close together), the formula would give garbage or divide by zero. In those cases the usage is reported as 0%.

**The header must stay visible while scrolling.** CPU and memory rows are drawn at fixed positions, and the process list only gets the remaining `LINES - header_rows` lines, so the header never scrolls off the screen.

## License

MIT, see [LICENSE](LICENSE).
