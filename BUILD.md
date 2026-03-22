# Build instructions

## GNU/Linux

### Requirements:

* `gcc >= 4.7`
* X11 dev pagages:
    * `libx11-dev libxpm-dev x11proto-dev` (Ubuntu / Debian / most of deb-based distros)
    * `libX11-devel libXpm-devel xproto-devel` (most of rpm-based distros)
* `make`

### Compilation

```
make
```

If X11 is unavailable, you can force a headless build:

```
make USE_X11=0
```

## macOS

thanks to [@aligfellow](https://github.com/aligfellow)

```
brew install --cask xquartz # runtime display
brew install libx11 libxpm pkg-config
make
```

`make` now auto-falls back to a headless build when X11 is not detected.
You can force behavior explicitly:

```
make USE_X11=1   # require X11
make USE_X11=0   # force headless mode
```
