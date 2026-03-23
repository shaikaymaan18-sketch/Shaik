# Custom Play Time Syntax

This document describes the formatting syntax used to display custom playtime values in the emulator.

## Overview

Playtime is internally stored as a total number of seconds. This formatting system allows users to control how that value is displayed by using tokens inside a format string.

Example:

```
{H:02}:{M:02}:{S:02}
```

Output:

```
02:03:04
```

## Tokens

The following tokens can be used in format strings.

| Token | Description              |
| ----- | ------------------------ |
| `{d}` | Total days               |
| `{h}` | Total hours              |
| `{H}` | Hours component (0–23)   |
| `{m}` | Total minutes            |
| `{M}` | Minutes component (0–59) |
| `{s}` | Total seconds            |
| `{S}` | Seconds component (0–59) |

## Padding

Tokens may optionally include zero-padding using the syntax `:NN`, where `NN` is the minimum width.

Example:

```
{H:02}:{M:02}:{S:02}
```

This ensures each component is displayed with at least two digits.

Example output:

```
02:03:04
```

## Conditional Sections

Conditional sections allow parts of the format string to appear only when a specific time unit is non-zero. This is useful for hiding units such as `0h`.

Conditional sections use the following syntax:

```
[unit] ... [/unit]
```

Where `unit` is one of the following:

| Unit | Condition                      |
| ---- | ------------------------------ |
| `d`  | Display section if days > 0    |
| `h`  | Display section if hours > 0   |
| `m`  | Display section if minutes > 0 |

Example:

```
[h]{H}h [/h][m]{M}m [/m]{S}s
```

Possible outputs:

```
1h 3m 4s
3m 4s
4s
```

Conditional sections may contain both tokens and literal text.

## Escaping Braces

To include literal braces in the output, they must be escaped using double braces.

| Input | Output |
| ----- | ------ |
| `{{`  | `{`    |
| `}}`  | `}`    |

Example:

```
Playtime: {{ {H}:{M}:{S} }}
```

Output:

```
Playtime: { 2:3:4 }
```

## Literal Text

Any text outside of tokens or conditional sections is copied directly into the output.

Example:

```
{h}h {M}m {S}s
```

Output:

```
26h 3m 4s
```

## Examples

### Clock Format

```
{H:02}:{M:02}:{S:02}
```

Example output:

```
02:03:04
```

### Human Readable Format

```
{h} hours, {M} minutes, {S} seconds
```

Example output:

```
26 hours, 3 minutes, 4 seconds
```

### Compact Format

```
[h]{H}h [/h][m]{M}m [/m][s]{S}s
```

Example output:

```
3m 4s
```

## Notes

* Playtime values are derived from the total number of elapsed seconds.
* Component tokens (`H`, `M`, `S`) wrap within their normal ranges.
* Total tokens (`h`, `m`, `s`) represent the full accumulated value for that unit.
* This is based on the [fmt syntax](https://fmt.dev/12.0/syntax/). Almost everything there is also supported here.