# AGENTS.md - Muon Programming Language

## Project Overview

Muon is an extensible dataflow language implemented in C. It features a type system based on Algebraic Subtyping (the SimpleSub algorithm), with explicit polymorphism via `∀` annotations rather than let-polymorphism.

## Build Commands

- **Build**: `cmake --build build`
- **Clean build**: `rm -rf build && mkdir build && cmake -B build`
- **Run**: `./build/muon sample.muon`
- **Run with DOT graph output**: `DOT=1 ./build/muon sample.muon` (writes `out.dot` and `out.dot.png`)
- **Test**: `cmake --build build --target test` or `ctest --test-dir build`
- **Format**: `cmake --build build --target format`

## Pipeline

The compiler pipeline is: **scan** → **detect** → **induce** → **reduce** → **author**

| Stage | Directory | Purpose |
|-------|-----------|---------|
| scan | `source/scan/` | Lexer (re2c) + parser (Bison) producing AST |
| detect | `source/detector/` | Name resolution and binding |
| induce | `source/inductor/` | Type inference — constraint generation |
| reduce | `source/inductor/reduce.c` | Coalescing — solving constraints to concrete types |
| author | `source/author/` | LLVM code generation (currently disabled) |

The reductor (`source/reductor/`) contains coercion witness extraction (post-inference).

## Source Organization

- `header/muon/` — Public API headers
- `source/` — Implementation, organized by pipeline stage
- `source/engine/` — Core data structures (types, cores, nodes)
- `source/common/` — Shared utilities (vectors, areas, debugging)

Standards: C23 throughout. Requires LLVM for code generation.

## Code Style

### Naming

| Element | Convention | Example |
|---------|------------|---------|
| Types | `PascalCase` with `Muon` prefix | `MuonNode`, `MuonEngine` |
| Functions | `snake_case` with `muon_` prefix (public) | `muon_implicit_type()` |
| Variables | `snake_case` | `engine`, `rule_length` |
| Macros/Enums | `UPPER_SNAKE_CASE` with `MUON_` prefix | `MUON_EACH_NODE_STEM` |

### Conventions

- Use `_Bool` (not `bool`)
- Use `size_t` for sizes and counts
- Use `goto except` pattern for error cleanup
- Use `assert()` for internal invariants
- 2-space indentation
- Use X-macro pattern (`MUON_EACH_*`) for code generation over tagged unions

### Formatting

- Opening brace on same line
- Space after keywords (`if (`, `for (`), no space before function call parens
- Imports: corresponding header, local headers, parent headers, system headers

## Language Syntax

```
-- Definitions (no `define` keyword)
x = 42
flag = true

-- Lambdas
identity = lambda x = x

-- Records
point = (x: 42, y: true)

-- Vectors
numbers = [1, 2, 3]

-- Scheme expressions (∀ or @ are interchangeable)
id = ∀(t: t -> t) lambda x = x
f = @(u: u -> (foo: u)) lambda x = (foo: id x)

-- Type annotations with * holes
g = ∀(u: u -> *) lambda x = (1, x)

-- Datatypes and coercions
datatype MyBoolean = MyTrue | MyFalse
instance MyBoolean <: Integer = switch (case MyTrue = 1, case MyFalse = 0)

-- Type cast
x = expr ∷ sign
```

## Type System (Inductor)

The type system is based on **Algebraic Subtyping** using the **SimpleSub** algorithm. There is no let-polymorphism; polymorphism requires explicit `∀` annotations.

### Key Concepts

**Types** (`header/muon/engine/type.h`): Every type has a `tag`, `id`, and `scheme` pointer indicating which scheme scope it belongs to (`NULL` = outermost static scope).

| Type | Tag | Description |
|------|-----|-------------|
| `MuonCoreType` | `MUON_CORE_TYPE` | Structural types (functions, records, etc.) with variance per member |
| `MuonImplicitType` | `MUON_IMPLICIT_TYPE` | Inference variables (unknowns) |
| `MuonVariableType` | `MUON_VARIABLE_TYPE` | Bound scheme variables (`u` in `∀(u: ...)`) with `join`/`meet` bounds |
| `MuonJoinType` | `MUON_JOIN_TYPE` | Union types (∨); empty = ⊥ |
| `MuonMeetType` | `MUON_MEET_TYPE` | Intersection types (∧); empty = ⊤ |
| `MuonSchemeType` | `MUON_SCHEME_TYPE` | Polymorphic type with bound variables and a `matter` (body type) |

**Attitude** (`source/engine/type.h`): A `(type, charge)` pair. Charge 0 = positive/covariant, charge 1 = negative/contravariant. Variance flips charge: `charge ^ member.variance`.

### Constraint Graph

Constraints are represented as **rules** (edges) in a directed graph stored in `Inductor.edge[]`.

**Rule structure** (`source/inductor/common.h`):
- `source ⇒ target` means `source <: target`
- `locked[2]`: polarity-dependent visibility. `locked[0]` hides from charge-0 iteration; `locked[1]` hides from charge-1 iteration
- `tag`: `NORMAL_RULE`, `REJECTED_RULE`, `INDIRECT_RULE` (transitive), `JOIN_RULE`
- `instance`: if non-NULL, this rule connects a scheme variable to its instance

**`rule_iterator(inductor, attitude)`** iterates edges incident on a type at a given charge, respecting `locked` bits. This is the primary polarity-aware traversal mechanism.

### Constraint Generation (`type_restrain` in `induce.c`)

`type_restrain(inductor, source, target, reason)` adds a `source <: target` constraint:
- **Implicit ⇒ explicit**: propagate to all predecessors of source
- **Explicit ⇒ implicit**: propagate to all successors of target
- **Implicit ⇒ implicit**: full transitive closure of both sides
- **Join source**: constrain each member
- **Meet target**: constrain to each member
- **Scheme source**: instantiate, then constrain the instance
- **Core ⇒ Core**: match cores, recurse into members with variance

`INDIRECT_RULE` edges maintain the transitive closure for implicit-to-implicit paths.

### Scheme Instantiation (`scheme_instance` in `scheme.c`)

Creates fresh implicit types for each bound variable in the scheme. Connects each scheme variable to its instance with a **locked** rule:
- At positive charge: rule `scheme_var ⇒ instance`, `locked[0] = 1`
- At negative charge: rule `instance ⇒ scheme_var`, `locked[1] = 1`

The locking ensures scheme variable bounds don't flow through instance variables in the wrong polarity direction.

### Three Kinds of Type Variables in a ∀

| Kind                   | Created by                                | `scheme` pointer  | Polymorphism                                         |
| ------                 | -----------                               | ----------------- | -------------                                        |
| Scheme variables (`u`) | `∀(u: ...)` binding                       | current scheme    | Universally quantified, rigid                        |
| `*` holes              | `*` in type annotation (`implicit_sign`)  | parent scheme     | Monomorphic to the scheme, solved during subsumption |
| Inference variables    | Lambda params, etc. during body inference | current scheme    | Polymorphic to the scheme                            |

The key invariant question (currently unsolved): when constraining `X <: u_inst` during subsumption, determining which `X` are valid depends on how polymorphic `X` is relative to the scheme — and different variable kinds within the same scheme have different effective polymorphism levels despite sharing the same `scheme` pointer.

### Coalescing (`reduce.c`)

Each implicit type gets **two semisolutions** (one per charge), stored in `inductor->semisolution[type_id * 2 + charge]`.

`reduce_implicit_type` collects bounds at a given charge via `rule_iterator`, simplifies (deduplicates, removes redundant types via `type_assess`), and produces a join. `implicit_solution` combines the two charge-specific solutions.

`reduce_type` performs a depth-first traversal, computing semisolutions for implicit types and reconstructing concrete types with resolved children.

### Subsumption for ∀ Expressions

In `scheme_expr_return` (`node.c`): after the body is inferred, `node_restrain(body, template_matter)` constrains the inferred body type against the scheme's declared type. After this, we need validity checks to reject:

1. **Scheme variables acquiring concrete bounds**: e.g., `∀(u: u -> u) lambda x = 1` — `Integer` ends up as a lower bound of `u_inst`. Detected at constraint time: when a concrete type meets a scheme variable instance, check compatibility with the variable's declared bounds.

2. **`*` holes depending on scheme-polymorphic variables**: e.g., `∀(u: u -> *) lambda x = x` — the `*` can't be resolved without mentioning `u`.

3. **Retroactive generalization**: e.g., `∀(u: u -> u) g` where `g` is monomorphic — external inference variables get identified with scheme variables.

Cases 2 and 3 can be detected with a **polarity-aware reachability walk** after subsumption, using `rule_iterator` (which already respects `locked` bits for polarity isolation).
