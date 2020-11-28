## How to build on HamoniKR 4.0 (Ubuntu 20.04, LinuxMint 20)

### 1) Install devscripts, build-essential, debhelper.

```
sudo apt install devscripts build-essential debhelper
```

### 2) Check dependent packages

```
dpkg-checkbuilddeps
```
You may see something like:

```
dpkg-checkbuilddeps: Unmet build dependencies: some-package1 some-package2 ...
```

### 3) Install all dependent packages

```
sudo apt install libgtk-3-dev qtbase5-private-dev libappindicator3-dev libhangul-dev libanthy-dev anthy librime-dev libxkbcommon-dev libwayland-dev wayland-protocols libxklavier-dev libm17n-dev m17n-db
```

### 4) dpkg-buildpackage

```
dpkg-buildpackage -T clean
dpkg-buildpackage -us -uc
```

### 5) Intall from deb files

```
# Install nesscery library firts
sudo dpkg -i libnimf1_*_amd64.deb 

# Install im package
sudo apt install nimf nimf-libhangul

im-config -n nimf
```
