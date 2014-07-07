## PUMP Workshop Version

A short paper describing PUMP can be found _[here](http://ccs2014.github.io/submission243/PUMP_workshop_anon.pdf)_ .

## Symbolic Rules

A full list of symbolic rules for all policies described in the CCS paper can be found _[here](http://ccs2014.github.io/submission243/symbolic_rules.pdf)_ .

## [_Update_] Simulation Code

PUMP Simulation code can be found in the `Simulation Code` directory. More information in the README file there.

## Mitigation Demonstration

Accompanying C source and assembly (disassembled version of the binary in 64-bit ALPHA ISA) code of the mitigation demonstration examples is also included. The structure is as follows:

```
.
├── Control-Flow Integrity
│   ├── rop.asm
│   └── rop.c
├── Primitive Types
│   ├── type.asm
│   └── type.c
├── README.md
├── Spatial and Temporal Memory Safety
│   ├── Spatial Safety
│   │   ├── memory.asm
│   │   └── memory.c
│   └── Temporal Safety
│       ├── memory.asm
│       └── memory.c
└── Taint Tracking
    ├── input.txt
    ├── taint.asm
    └── taint.c

6 directories, 12 files
```
