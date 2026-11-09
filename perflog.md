# Performance Logging

| Optimization              | Performance (s)   |
| ------------------------- | ----------------- |
| Baseline                  | 660.536030        |
| Hoist + Reduce Work       | 478.970447        |
| 1D Grid (no N fix)        | 737.071546        |
| 1D Grid (less N accesses) | 469.562549        |
| Buffer Border             | 397.517755        |
| Masking + Double Buffer   | 372.951428        |
| Fix column bounds         | 244.089095        |
| march=native              | 190.135318        |
| OpenMP                    | 51.601539         |
| Convert to 16 bits        | 25.885143         |