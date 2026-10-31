{ lib, pkgs, ... }:
{
  networking.hostName = "mini-drivebrain";
  networking.useDHCP = true;
  networking.firewall.enable = false;

  services.openssh.enable = true;

  users.users.nixos.isNormalUser = true;
  users.users.nixos.password = "nixos";
  users.users.nixos.extraGroups = [ "wheel" ];

  environment.systemPackages = [
    pkgs.htop
    pkgs.libgpiod
  ];

  system.activationScripts.createRecordingsDir = lib.stringAfter [ "users" ] ''
    mkdir -p /home/nixos/recordings
    chown nixos:users /home/nixos/recordings
  '';

  # lnxrouter runs hostapd for the hotspot and dnsmasq to give laptops an IP
  systemd.services.linux_router = {
    wantedBy = [ "multi-user.target" ];
    after = [ "network.target" ];

    serviceConfig = {
      # Give each pi its own (mini-drivebrain-1, mini-drivebrain-2..) so we dont have conflicting network names
      ExecStart = "${pkgs.linux-router}/bin/lnxrouter -d -g 192.168.203.67 -n --ap wlan0 mini-drivebrain";
      Restart = "on-failure";
    };
  };

  systemd.services.mini-drivebrain = {
    description = "mini-drivebrain";
    wantedBy = [ "multi-user.target" ];
    after = [ "network.target" ];

    serviceConfig = {
      ExecStart = "/home/nixos/mini-drivebrain";
      Restart = "on-failure";
    };
  };

  system.stateVersion = "23.11";
}
