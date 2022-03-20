## Features

- [x] Shell Commands
- [x] Authentication
- [x] Popping MessageBox
- [x] File I/O
- [ ] Encrypted Traffic
- [ ] Self-Update
- [ ] Screenshot
- [ ] Network Proxy

## Building

```sh
export ALAKAZAM_KEY=AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE
export GENGAR_KEY=AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE

# Debug
make

# Release
make release

# Release With CNC Host
export ALAKAZAM_HOST="address.of.cnc"
make release_host
```
