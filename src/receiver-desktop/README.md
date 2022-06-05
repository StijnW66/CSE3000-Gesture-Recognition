## Setup for Windows
- Run on PowerShell with Administrator Permissions
```
choco install make  // installs the make GNU utility
choco install mingw // installs the g++ compiler
```
- Restart

## Run
- Go to the `receiver-desktop` directory.
- Run `make`
- Run `./main <input-file> <output-file>`
    - <output-file> will be created if it does not exist
    - <output-file> is overwritten so provide a different name every time

- <input-file> is expected to have the following structure:
    - line 1: gesture-signal-length
    - line 2: threshold-1 threshold-2 threshold-3
    - line 3-length: v1 v2 v3

    - Example: 
        ```
        3
        2 3 3 
        1 1 1 
        2 2 2
        3 3 3
        ```
- <output-file> will have the following structure:
    - line1-100: o1 o2 o3