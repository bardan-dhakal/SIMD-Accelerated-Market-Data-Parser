# Architecture Documentation

This document provides a detailed overview of the SIMD-Accelerated Market Data Parser architecture, design decisions, and implementation details.

## Table of Contents

1. [System Overview](#system-overview)
2. [Component Architecture](#component-architecture)
3. [Data Flow](#data-flow)
4. [SIMD Implementation Details](#simd-implementation-details)
5. [Memory Management](#memory-management)
6. [Design Decisions](#design-decisions)
7. [Extension Points](#extension-points)

---

## System Overview

The parser is designed as a high-performance FIX protocol message parser that leverages AVX-512 SIMD instructions to achieve significant speedups over traditional scalar implementations.

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Client Application                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Parser Interface                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │ parse_auto()│  │parse_scalar()│  │     parse_simd()       │  │
│  └──────┬──────┘  └─────────────┘  └─────────────────────────┘  │
│         │                                                        │
│         ▼                                                        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │              Runtime CPU Detection                           ││
│  │              has_avx512_support()                            ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                     SIMD Utilities Layer                         │
│  ┌─────────────────────────┐  ┌─────────────────────────────┐   │
│  │  find_delimiters_simd() │  │  find_delimiters_scalar()   │   │
│  │  (AVX-512 accelerated)  │  │  (baseline implementation)  │   │
│  └─────────────────────────┘  └─────────────────────────────┘   │
│  ┌─────────────────────────┐  ┌─────────────────────────────┐   │
│  │      parse_int()        │  │      parse_double()         │   │
│  └─────────────────────────┘  └─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                      FIX Message Structure                       │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  FIXMessage { message_type, symbol, sender, target,         ││
│  │               side, price, quantity, valid }                 ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

---

## Component Architecture

### 1. Parser Module (`parser.hpp` / `parser.cpp`)

The parser module provides three entry points:

| Function | Description | Use Case |
|----------|-------------|----------|
| `parse_auto()` | Auto-detects CPU and selects optimal implementation | Production use |
| `parse_scalar()` | Baseline scalar implementation | Benchmarking, fallback |
| `parse_simd()` | AVX-512 accelerated implementation | Maximum performance |

**Internal Functions:**

- `split_field()`: Extracts tag and value from "tag=value" format
- `populate_message()`: Maps FIX tags to FIXMessage struct fields

### 2. SIMD Utilities Module (`simd_utils.hpp` / `simd_utils.cpp`)

Core SIMD operations and CPU detection:

```
┌─────────────────────────────────────────────────────────┐
│                  simd_utils module                       │
├─────────────────────────────────────────────────────────┤
│  CPU Detection                                           │
│  ├── has_avx512_support()                               │
│  │   ├── CPUID leaf 7 (AVX512F, AVX512BW)              │
│  │   └── XGETBV (OS support verification)              │
├─────────────────────────────────────────────────────────┤
│  Delimiter Finding                                       │
│  ├── find_delimiters_simd()   [64 bytes/iteration]     │
│  └── find_delimiters_scalar() [1 byte/iteration]       │
├─────────────────────────────────────────────────────────┤
│  Numeric Parsing                                         │
│  ├── parse_int()    [std::from_chars + fallback]       │
│  └── parse_double() [std::from_chars + manual]         │
└─────────────────────────────────────────────────────────┘
```

### 3. FIX Message Module (`fix_message.hpp` / `fix_message.cpp`)

Data structures for parsed messages:

```cpp
struct FIXMessage {
    std::string_view message_type;  // Tag 35
    std::string_view symbol;        // Tag 55
    std::string_view sender;        // Tag 49
    std::string_view target;        // Tag 56
    int32_t side;                   // Tag 54
    double price;                   // Tag 44
    int32_t quantity;               // Tag 38
    bool valid;                     // Parsing success flag
};
```

**Supported FIX Tags:**

| Tag | Name | Type | Description |
|-----|------|------|-------------|
| 8 | BeginString | string | FIX protocol version |
| 9 | BodyLength | int | Message body length |
| 35 | MsgType | string | Message type identifier |
| 49 | SenderCompID | string | Sender identification |
| 56 | TargetCompID | string | Target identification |
| 54 | Side | int | Order side (1=Buy, 2=Sell) |
| 55 | Symbol | string | Trading symbol |
| 38 | OrderQty | int | Order quantity |
| 44 | Price | double | Order price |

---

## Data Flow

### Parsing Pipeline

```
Input FIX Message (string_view)
        │
        ▼
┌───────────────────────────────────────┐
│     1. Find Delimiters                │
│     ┌─────────────────────────────┐   │
│     │ SIMD: Process 64 bytes/iter │   │
│     │ Output: vector<size_t>      │   │
│     └─────────────────────────────┘   │
└───────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────┐
│     2. Extract Fields                 │
│     ┌─────────────────────────────┐   │
│     │ Split on delimiter positions│   │
│     │ Output: string_view fields  │   │
│     └─────────────────────────────┘   │
└───────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────┐
│     3. Parse Tag=Value Pairs          │
│     ┌─────────────────────────────┐   │
│     │ Find '=' separator          │   │
│     │ Extract tag number          │   │
│     │ Extract value string_view   │   │
│     └─────────────────────────────┘   │
└───────────────────────────────────────┘
        │
        ▼
┌───────────────────────────────────────┐
│     4. Populate FIXMessage            │
│     ┌─────────────────────────────┐   │
│     │ Switch on tag number        │   │
│     │ Convert numerics as needed  │   │
│     │ Assign string_views         │   │
│     └─────────────────────────────┘   │
└───────────────────────────────────────┘
        │
        ▼
Output FIXMessage struct
```

### Zero-Copy Design

The parser achieves zero-copy parsing by using `std::string_view`:

```
Original Message Buffer (owned by caller)
┌─────────────────────────────────────────────────────────┐
│ 8=FIX.4.4|35=D|49=SENDER|55=AAPL|54=1|38=100|44=150.25│
└─────────────────────────────────────────────────────────┘
      ▲           ▲      ▲        ▲
      │           │      │        │
      │           │      │        └── symbol points here
      │           │      └── sender points here
      │           └── message_type points here
      └── (begin_string - not stored)

FIXMessage struct contains string_views pointing into original buffer
- No memory allocation for string data
- Caller must keep original buffer alive while using FIXMessage
```

---

## SIMD Implementation Details

### AVX-512 Delimiter Finding Algorithm

```
Input: "8=FIX.4.4|35=D|49=SENDER|55=AAPL|"  (34 bytes)
Delimiter: '|' (0x7C)

Step 1: Load 64 bytes into ZMM register
┌────────────────────────────────────────────────────────────────┐
│ 8 = F I X . 4 . 4 | 3 5 = D | 4 9 = S E N D E R | 5 5 = A ... │
└────────────────────────────────────────────────────────────────┘
                    ▲             ▲                 ▲
                   [9]          [13]              [23]

Step 2: Broadcast delimiter to all 64 positions
┌────────────────────────────────────────────────────────────────┐
│ | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ...│
└────────────────────────────────────────────────────────────────┘

Step 3: Compare all 64 bytes simultaneously
Result mask: 0b0000...0000100000000010000100000000
                              ▲         ▲    ▲
                             [23]      [13]  [9]

Step 4: Extract positions using __builtin_ctzll
- First iteration:  ctzll(mask) = 9,  positions.push_back(9)
- Second iteration: ctzll(mask) = 13, positions.push_back(13)
- Third iteration:  ctzll(mask) = 23, positions.push_back(23)
```

### Key AVX-512 Instructions Used

| Instruction | Latency | Throughput | Purpose |
|-------------|---------|------------|---------|
| `_mm512_loadu_si512` | 1-2 | 0.5 | Load 64 bytes (unaligned) |
| `_mm512_set1_epi8` | 1 | 0.5 | Broadcast byte to 64 positions |
| `_mm512_cmpeq_epi8_mask` | 1 | 0.5 | Compare 64 bytes, produce mask |
| `__builtin_ctzll` | 3-4 | 1 | Count trailing zeros |

### Handling Remainder Bytes

For messages not divisible by 64 bytes, remaining bytes are processed with scalar code:

```cpp
// SIMD processes chunks of 64 bytes
while (pos + 64 <= size) {
    // AVX-512 processing...
    pos += 64;
}

// Scalar processes remaining bytes
while (pos < size) {
    if (data[pos] == delimiter) {
        positions.push_back(pos);
    }
    pos++;
}
```

---

## Memory Management

### Allocation Strategy

| Component | Allocation | Lifetime |
|-----------|------------|----------|
| Input message | Caller-owned | Must outlive FIXMessage |
| FIXMessage | Stack or caller-managed | Determined by caller |
| Delimiter positions | `std::vector` (heap) | Function scope |
| String views | No allocation | Points to input |

### Cache Optimization

The parser is designed for cache efficiency:

1. **Sequential Access**: Data is read sequentially, enabling hardware prefetching
2. **Minimal Allocations**: Only delimiter positions vector is heap-allocated
3. **Small Working Set**: FIXMessage struct is compact (~80 bytes)

---

## Design Decisions

### 1. Focus SIMD on Delimiter Finding

**Rationale**: Profiling showed delimiter finding is the dominant cost in FIX parsing. SIMD acceleration of this single operation yields the largest performance gain.

**Alternative Considered**: SIMD for numeric parsing was considered but deferred because:
- Integer/double parsing is already fast (~5-15ns)
- Complexity of SIMD numeric parsing doesn't justify marginal gains
- Delimiter finding provides 8-10x speedup vs 1-2x for numerics

### 2. Runtime CPU Detection

**Rationale**: Enables single binary deployment across different hardware.

```cpp
FIXMessage parse_auto(std::string_view message) {
    static bool avx512_available = has_avx512_support();  // Cached
    if (avx512_available) {
        return parse_simd(message);
    }
    return parse_scalar(message);
}
```

### 3. Zero-Copy with string_view

**Rationale**: Eliminates memory allocation overhead in hot path.

**Trade-off**: Caller must ensure input buffer outlives FIXMessage. This is acceptable for streaming scenarios where messages are processed immediately.

### 4. Minimal Validation

**Rationale**: Production FIX engines perform validation at protocol level. Parser focuses on extraction speed.

**Validation Performed**:
- Non-empty message type
- Non-empty symbol
- Valid field format (tag=value)

### 5. Switch-based Tag Dispatch

**Rationale**: Compiler optimizes switch statements into jump tables for O(1) dispatch.

```cpp
void populate_message(FIXMessage& msg, uint32_t tag, std::string_view value) {
    switch (tag) {
        case 35: msg.message_type = value; break;
        case 55: msg.symbol = value; break;
        // ...
    }
}
```

---

## Extension Points

### Adding New FIX Tags

1. Add field to `FIXMessage` struct in `fix_message.hpp`
2. Add tag constant to `FIXTag` enum
3. Add case to `populate_message()` switch in `parser.cpp`

### Adding New SIMD Implementations

1. Add detection function (e.g., `has_avx2_support()`)
2. Implement new delimiter finder (e.g., `find_delimiters_avx2()`)
3. Update `parse_auto()` dispatch logic

### Supporting Different Delimiters

The delimiter is parameterized in the API:

```cpp
std::vector<size_t> find_delimiters_simd(std::string_view data, char delimiter);
```

To support SOH (0x01) delimiter used in real FIX:
```cpp
auto positions = find_delimiters_simd(message, '\x01');
```

---

## File Structure

```
include/
├── parser.hpp          # Public parsing API
├── fix_message.hpp     # FIXMessage struct and FIXTag enum
└── simd_utils.hpp      # SIMD utilities API

src/
├── parser.cpp          # Parser implementation
├── simd_utils.cpp      # SIMD and CPU detection implementation
└── fix_message.cpp     # (Reserved for future utilities)
```

---

## References

- [Intel Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)
- [FIX Protocol Specification](https://www.fixtrading.org/standards/)
- [Agner Fog's Optimization Manuals](https://agner.org/optimize/)
