# CSweep  
*A garbage collector for C, using principles of compiler design*  
Because memory leaks are just clingy pointers.

## What is CSweep?

CSweep is a garbage collector for the C programming language. Yes, **C**—the language that trusts you enough to manage your own memory, and laughs when you segfault. We took the bold approach of applying **compiler design principles** to track and clean up your program's memory like a responsible roommate with a label maker.

Gone are the days of `free()`-dom. CSweep is here to *sweep* up your mess.

## Features

- **Automatic memory management** for C programs — finally, you can pretend C is a modern language.
- **Compiler-inspired analysis** — because who better to understand your code than the compiler you ignored when it warned you?
- **Non-intrusive integration** — plug it in and let it do the dirty work. No need to rewrite your entire codebase.
- **Zero segfaults*** — (*your mileage may vary*)

## Why?

Because:
- You *will* forget to `free()`.
- Valgrind is tired of your nonsense.
- Manual memory management is so 1970s.
- You shouldn’t need a PhD to write a buffer-safe `malloc`.

## How does it work?

Using **compiler design sorcery**, CSweep performs static analysis, tracks allocations, and inserts cleanup routines so your code doesn’t hoard heap like a digital raccoon.

In short:
1. Parses your C code like a boss.
2. Detects allocations and references like a memory detective.
3. Inserts collection logic so your heap doesn’t become a haunted house.

## 🗂️ Project Structure

```bash
DevanshMishra-codes-CSweep/
│
├── README.md             # Project overview and instructions
├── augument.py           # Python script (helper for data handling or preprocessing)
├── parser.cpp            # Main C++ file that parses and injects delete statements
│
└── testData/             # Test files and examples
    ├── README.md         # Notes on test cases
    ├── parse.txt         # Output of variable parsing
    ├── sweepedtest.c     # Output file with delete statements inserted
    ├── test.c            # Original input file for testing
    └── test_2.C          # Another test input file
