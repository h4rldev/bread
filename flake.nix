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
      packages.bread-wayland-release = pkgs.stdenv.mkDerivation {
        pname = "bread-wayland";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.just
          pkgs.gcc
          pkgs.mold
          pkgs.wayland-scanner
          pkgs.wayland-protocols
          pkgs.libxkbcommon
          pkgs.wayland
          htils.packages.${system}.htils
        ];

        buildPhase = ''
          runHook preBuild

          mkdir -p ./src/wayland
          mkdir -p ./include/wayland

          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c

          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just release

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib
          mkdir -p $out/include/bread

          cp lib/libbread-wayland-release.so $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-wayland-debug = pkgs.stdenv.mkDerivation {
        pname = "bread-wayland-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.just
          pkgs.gcc
          pkgs.wayland-scanner
          pkgs.wayland-protocols
          pkgs.libxkbcommon
          pkgs.wayland
          htils.packages.${system}.htils
        ];

        buildPhase = ''
          runHook preBuild

          mkdir -p ./include/wayland
          mkdir -p ./src/wayland

          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c

          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just debug

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib
          mkdir -p $out/include/bread

          cp lib/libbread-wayland-debug.a $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-x11-release = pkgs.stdenv.mkDerivation {
        pname = "bread-x11";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.just
          pkgs.gcc
          pkgs.libxcb
          pkgs.libxcb-wm
          pkgs.libxkbcommon
          htils.packages.${system}.htils
        ];

        buildPhase = ''
          runHook preBuild

          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just release x11

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib
          mkdir -p $out/include/bread

          cp lib/libbread-x11-release.so $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-x11-debug = pkgs.stdenv.mkDerivation {
        pname = "bread-x11-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.just
          pkgs.gcc
          pkgs.libxcb
          pkgs.libxcb-wm
          pkgs.libxkbcommon
          htils.packages.${system}.htils
        ];

        buildPhase = ''
          runHook preBuild

          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just debug x11

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib
          mkdir -p $out/include/bread

          cp lib/libbread-x11-debug.a $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      devShells.default = pkgs.mkShell {
        name = "bread-dev";

        buildInputs = with pkgs; [
          wayland-scanner
          pkg-config
          gcc
          libxcb-cursor
          libxcb
          libxcb-wm
          libxkbcommon
          wayland
          wayland-protocols
          htils.packages.${system}.htils
        ];

        nativeBuildInputs = with pkgs; [
          mold
          glibc
        ];

        packages = with pkgs; [
          nixd
          bear
          just
          clang-tools
          nix-index
          valgrind
        ];

        shellHook = ''
          mkdir -p include/wayland
          mkdir -p src/wayland

          export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.gcc.cc.lib}/lib:${pkgs.libxcb}/lib:${pkgs.libxcb-wm}/lib:${pkgs.libxcb-cursor}/lib"
          export NIX_LDFLAGS="-rpath ${htils.packages.${system}.htils}/lib -rpath ${pkgs.libxcb}/lib -rpath ${pkgs.libxcb-wm}/lib  $NIX_LDFLAGS"


          [[ -f ./include/wayland/xdg-shell-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          [[ -f ./src/wayland/xdg-shell-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          [[ -f ./include/wayland/xdg-decoration-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          [[ -f ./src/wayland/xdg-decoration-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c
        '';
      };
    });
}
