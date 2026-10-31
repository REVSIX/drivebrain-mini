{
  description = "mini-drivebrain pi image";
  nixConfig = {
    extra-substituters = [ "https://nix-community.cachix.org" "https://rcmast3r.cachix.org" ];
    extra-trusted-public-keys = [ "nix-community.cachix.org-1:mB9FSh9qf2dCimDSUo8Zy7bkq5CX+/rkCWyvRCYg3Fs=" "rcmast3r.cachix.org-1:dH22dF877RZ1j7uvAgqnQWNChxdQDeqgBRWpXzoi84c=" ];
  };

  inputs = {
    raspberry-pi-nix.url = "github:nix-community/raspberry-pi-nix";
    nixpkgs.url = "github:NixOS/nixpkgs/cbd8ec4de4469333c82ff40d057350c30e9f7d36";
  };

  outputs = { nixpkgs, raspberry-pi-nix, ... }: {
    nixosConfigurations.mini-drivebrain = nixpkgs.lib.nixosSystem {
      system = "aarch64-linux";
      modules = [
        raspberry-pi-nix.nixosModules.raspberry-pi
        raspberry-pi-nix.nixosModules.sd-image
        ./configuration.nix
        {
          raspberry-pi-nix.libcamera-overlay.enable = false;
          raspberry-pi-nix.board = "bcm2712"; # pi 5. use "bcm2711" for a pi 4
        }
      ];
    };
  };
}
