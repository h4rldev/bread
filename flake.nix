{
  description = "Development flake for a platform layer in C using htils.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    htils.url = "github:h4rldev/htils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    htils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      devShells.default = pkgs.mkShell {
        name = "bread-dev";

        buildInputs = with pkgs; [
          wayland-scanner
          vulkan-headers
          vulkan-loader
          pkg-config
          shader-slang
          gcc
          cglm
          libx11
          libxcb
          libxcb-wm
          libxkbcommon
          wayland
          wayland-protocols
          htils.packages.${system}.htils
        ];

        packages = with pkgs; [
          nixd
          alejandra
          bear
          just
          clang-tools
          vulkan-tools
        ];

        shellHook = ''
          mkdir -p include/wayland
          mkdir -p src/wayland

          [[ -f ./include/wayland/xdg-shell-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          [[ -f ./src/wayland/xdg-shell-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
        '';
      };
    });
}
