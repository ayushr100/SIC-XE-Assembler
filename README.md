SIC/XE Assembler — User Guide

This assembler supports literals, expressions, and program blocks along with standard assembly of SIC/XE instructions. This guide explains how to set up, run, and understand the outputs and internals of the assembler.

Note: Write your assembly program in ALL UPPERCASE with no leading spaces at the start of each line.

## Project Layout

Ensure the following folders and files exist in the current directory:

- data/
  - input.txt
  - intermediate.txt
  - listing_file.txt
  - error.txt
  - object_program.txt
  - SYMTAB.txt
  - BLOCKTAB.txt
- src/
  - assembler.cpp
  - pass1.h
  - pass2.h
  - functions.h

Edit your assembly program in `data/input.txt`.

## Running the Assembler

From a terminal, change into the `src` directory that contains `assembler.cpp`, compile, and run.

macOS/Linux (zsh):

```
g++ assembler.cpp -o assembler
./assembler
```

Windows (Command Prompt or PowerShell):

```
g++ assembler.cpp -o assembler.exe
./assembler.exe
```

When you run the executable, the program prints the assembly status to the terminal and writes outputs to the files in the `data/` folder as described below.

## Output Files

The program generates the following files (plus some intermediate artifacts):

- object_program.txt: Stores the final object program.
- listing_file.txt: Displays the original assembly code with additional information such as line number, address (location counter), program block number, instruction, operand, and object code for each line.
- SYMTAB.txt: The symbol table generated during Pass 1.
- BLOCKTAB.txt: Program block table generated during Pass 1.

Intermediate files:

- error.txt: Lists all errors encountered during Pass 1 and Pass 2 with line numbers and reasons.
- intermediate.txt: Data produced by Pass 1 and consumed by Pass 2.

## Internal Data Structures

The assembler uses the following structures:

- LITTAB (Literal Table): Stores information about literals (constant values) in the program.
- BLOCKTABLE (Block Table): Tracks program blocks and their info: block name, block number, block length, starting address, and block location counter (current LOCCTR value per block during Pass 1).
- SYMTAB (Symbol Table): Records all defined symbols: label, block number, value, and whether the label is relative or absolute.
- OPTAB (Opcode Table): Definitions for supported opcodes (instructions) with opcode values and formats.
- assembler_directives: Set of recognized assembler directives.
- TEXT_RECORDS: Array of text records used during object program generation in Pass 2; each record has fields: initial, start address, length, object_code, and object_code_length.

## Global Variables

- program_name: Stores the program name.
- program_length: Stores the total program length.

### Pass 1

- pass1_input: Input file.
- pass1_output: Intermediate file.
- pass1_error: Error file.
- line: Each line read from the input file for processing.
- label, opcode, operand: The label, directive/instruction, and operand on each line.
- LOCCTR: Current location counter for the current block.
- ERROR_FLAG_PASS1: Set to true if any error is encountered in Pass 1.
- current_block_no: Current block number being assembled.
- current_block_name: Current block name being assembled.
- total_blocks: Total number of different blocks encountered.
- line_no: Current input program line number being assembled.
- star_count: Number of places where `*` is used in the operand.

### Pass 2

- pass2_input: Intermediate file.
- pass2_list: Listing file.
- pass2_object: Object program file.
- pass2_error: Error file.
- ERROR_FLAG_PASS2: Set to true if any error is encountered in Pass 2.
- line: Each line as read from the intermediate file.
- line_no, label, opcode, operand: Current line number, label, directive/instruction, and operand as each line is processed.
- locctr, program_block_no: Location counter and program block number as each line is processed.
- BASE_REGISTER_VALUE: Base register value for Base-relative addressing.
- BASE_RELATIVE_ADDRESSING: Boolean flag indicating whether Base-relative addressing is enabled.
- current_text_record: The current text record being built.
- current_object_code: Object code to add to the text record.
- prev_block_no: Program block number of the previous instruction.
- MODIFICATION_RECORDS: All modification records.

## Assembler Functionality

### Main Functions

- main(): Calls `pass1()` (from `pass1.h`) and `pass2()` (from `pass2.h`). Also calls `write_tables()` in `functions.h` to populate the symbol and block tables.
- pass1():
  - Processes the program file line by line.
  - Generates the symbol table and block table containing information about all symbols and blocks.
  - Creates an intermediate file to store data for Pass 2.
  - Lists all errors encountered during assembly in `error.txt`.
- pass2():
  - Reads the intermediate file generated in Pass 1.
  - Generates a listing file with additional information like addresses, block numbers, and object code.
  - Creates an object program file containing machine code instructions in the record formats H, T, M, and E.
  - Lists all errors encountered during assembly in `error.txt`.

### Source Code Files

- functions.h: Implements the data structures above and utility functions used by `pass1()`, `pass2()`, and `main()`.
- assembler.cpp: Entry point that orchestrates the main program logic including `pass1()` and `pass2()`.