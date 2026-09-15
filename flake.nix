{
  description = "Development flake for a platform layer in C using htils.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    htils.url = "github:h4rldev/htils";
    conjure.url = "git+https://codeberg.org/h4rl/conjure.git";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    htils,
    conjure,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};
      pversion = "0.1.0";

      commonInputs = [
        conjure.packages.${system}.default
        pkgs.gcc
        pkgs.mold
        htils.packages.${system}.htils-threadsafe
      ];

      waylandInputs = with pkgs; [
        wayland-scanner
        wayland-protocols
        libxkbcommon
        wayland
      ];

      x11Inputs = with pkgs; [
        libxcb
        libxcb-cursor
        libxcb-wm
        libxkbcommon
      ];

      waylandCodegen = ''
        mkdir -p ./src/wayland
        mkdir -p ./include/wayland

        wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
        wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
        wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
        wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c
      '';

      mkBread = {
        name,
        profile,
        artifact,
        pc,
        wayland ? false,
      }: let
        artifactStem =
          pkgs.lib.removePrefix "lib"
          (pkgs.lib.removeSuffix ".so"
            (pkgs.lib.removeSuffix ".a" (baseNameOf artifact)));
      in
        pkgs.stdenv.mkDerivation {
          pname = name;
          version = pversion;

          src = ./.;

          nativeBuildInputs =
            commonInputs
            ++ (
              if wayland
              then waylandInputs
              else x11Inputs
            );

          buildPhase = ''
            runHook preBuild
            ${pkgs.lib.optionalString wayland waylandCodegen}
            conjure as ${profile} build
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out/lib/pkgconfig
            mkdir -p $out/include/bread

            cp ${artifact} $out/lib
            sed -e "s|^prefix=.*|prefix=$out|" \
              -e "s|^libdir=.*|libdir=$out/lib|" \
              lib/${profile}/pkgconfig/${artifactStem}.pc \
              > $out/lib/pkgconfig/${pc}

            cp -r include/bread/* $out/include/bread

            runHook postInstall
          '';
        };
    in {
      packages = {
        bread-wayland-release = mkBread {
          name = "bread-wayland";
          profile = "wayland-release";
          artifact = "lib/wayland-release/libbread-wayland.so";
          pc = "bread-wayland.pc";
          wayland = true;
        };

        bread-wayland-release-static = mkBread {
          name = "bread-wayland-static";
          profile = "wayland-release-static";
          artifact = "lib/wayland-release-static/libbread-wayland.a";
          pc = "bread-wayland-static.pc";
          wayland = true;
        };

        bread-wayland-debug = mkBread {
          name = "bread-wayland-debug";
          profile = "wayland-debug";
          artifact = "lib/wayland-debug/libbread-wayland-debug.a";
          pc = "bread-wayland-debug.pc";
          wayland = true;
        };

        bread-x11-release = mkBread {
          name = "bread-x11";
          profile = "x11-release";
          artifact = "lib/x11-release/libbread-x11.so";
          pc = "bread-x11.pc";
        };

        bread-x11-release-static = mkBread {
          name = "bread-x11-static";
          profile = "x11-release-static";
          artifact = "lib/x11-release-static/libbread-x11.a";
          pc = "bread-x11-static.pc";
        };

        bread-x11-debug = mkBread {
          name = "bread-x11-debug";
          profile = "x11-debug";
          artifact = "lib/x11-debug/libbread-x11-debug.a";
          pc = "bread-x11-debug.pc";
        };
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
          htils.packages.${system}.htils-threadsafe
        ];

        nativeBuildInputs = with pkgs; [
          mold
          glibc
        ];

        packages = with pkgs; [
          nixd
          bear
          clang-tools
          nix-index
          valgrind

          conjure.packages.${system}.default
        ];

        shellHook = ''
          mkdir -p include/wayland
          mkdir -p src/wayland

          export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.gcc.cc.lib}/lib:${pkgs.libxcb}/lib:${pkgs.libxcb-wm}/lib:${pkgs.libxcb-cursor}/lib"
          export NIX_LDFLAGS="-rpath ${htils.packages.${system}.htils-threadsafe}/lib -rpath ${pkgs.libxcb}/lib -rpath ${pkgs.libxcb-wm}/lib  $NIX_LDFLAGS"

          [[ -f ./include/wayland/xdg-shell-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          [[ -f ./src/wayland/xdg-shell-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          [[ -f ./include/wayland/xdg-decoration-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          [[ -f ./src/wayland/xdg-decoration-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c
        '';
      };
    });
}
