# mini-drivebrain pi image

A stripped-down copy of [hytech_nixos](https://github.com/hytech-racing/hytech_nixos). It sets up:

* the `mini-drivebrain` systemd service, which runs `/home/nixos/mini-drivebrain` on boot
* `/home/nixos/recordings` for the MCAP files
* a wifi hotspot named `mini-drivebrain`. On it, the Pi is always `192.168.203.67`
* ssh as `nixos` (password `nixos`), and no firewall, so Foxglove can reach port 5555
* `htop` and `libgpiod` (`gpiodetect`, `gpiomon`)

## Build the SD image
You need nix with flakes turned on. Since this builds a Pi image on your laptop, you also need aarch64 emulation: install `qemu-user-static`, add these lines to `/etc/nix/nix.conf`, then restart `nix-daemon.service`:
```
extra-platforms = aarch64-linux
trusted-users = root <your username>
```

```bash
nix build .#nixosConfigurations.mini-drivebrain.config.system.build.sdImage --accept-flake-config
```

Flash it
```bash
zstdcat result/sd-image/*.img.zst | sudo dd of=/dev/sdX bs=4M status=progress oflag=direct
```

It's set up for a Pi 5. For a Pi 4, change `board` in `flake.nix` to `bcm2711`.

Connect your laptop to the `mini-drivebrain` wifi, then `ssh nixos@192.168.203.67`.
