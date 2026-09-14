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
    in {
      packages.bread-wayland-release = pkgs.stdenv.mkDerivation {
        pname = "bread-wayland";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.gcc
          pkgs.mold
          pkgs.wayland-scanner
          pkgs.wayland-protocols
          pkgs.libxkbcommon
          pkgs.wayland
          htils.packages.${system}.htils-threadsafe
          conjure.packages.${system}.default
        ];

        buildPhase = ''
          runHook preBuild

          mkdir -p ./src/wayland
          mkdir -p ./include/wayland

          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c

          conjure as wayland-release build

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib/pkgconfig
          mkdir -p $out/include/bread

          sed -e "s|^prefix=.*|prefix=$out|" -e "s|^libdir=.*|libdir=$out/lib|" lib/wayland-release/pkgconfig/bread-wayland.pc > $out/lib/pkgconfig/bread-wayland.pc
          cp lib/wayland-release/libbread-wayland.so $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-wayland-release-static = pkgs.stdenv.mkDerivation {
        pname = "bread-wayland-static";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.gcc
          pkgs.mold
          pkgs.wayland-scanner
          pkgs.wayland-protocols
          pkgs.libxkbcommon
          pkgs.wayland
          htils.packages.${system}.htils-threadsafe
          conjure.packages.${system}.default
        ];

        buildPhase = ''
          runHook preBuild

          mkdir -p ./src/wayland
          mkdir -p ./include/wayland

          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c

          conjure as wayland-release-static build

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib/pkgconfig
          mkdir -p $out/include/bread

          sed -e "s|^prefix=.*|prefix=$out|" -e "s|^libdir=.*|libdir=$out/lib|" lib/wayland-release-static/pkgconfig/bread-wayland.pc > $out/lib/pkgconfig/bread-wayland-static.pc
          cp lib/wayland-release-static/libbread-wayland.a $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-wayland-debug = pkgs.stdenv.mkDerivation {
        pname = "bread-wayland-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.gcc
          pkgs.wayland-scanner
          pkgs.wayland-protocols
          pkgs.libxkbcommon
          pkgs.wayland
          htils.packages.${system}.htils-threadsafe
          conjure.packages.${system}.default
        ];

        buildPhase = ''
          runHook preBuild

          mkdir -p ./include/wayland
          mkdir -p ./src/wayland

          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c

          conjure as wayland-debug build

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib/pkgconfig
          mkdir -p $out/include/bread

          sed -e "s|^prefix=.*|prefix=$out|" -e "s|^libdir=.*|libdir=$out/lib|" lib/wayland-debug/pkgconfig/bread-wayland-debug.pc > $out/lib/pkgconfig/bread-wayland-debug.pc
          cp lib/wayland-debug/libbread-wayland-debug.a $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-x11-release = pkgs.stdenv.mkDerivation {
        pname = "bread-x11";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.gcc
          pkgs.libxcb
          pkgs.libxcb-cursor
          pkgs.libxcb-wm
          pkgs.libxkbcommon
          htils.packages.${system}.htils-threadsafe
          conjure.packages.${system}.default
        ];

        buildPhase = ''
          runHook preBuild

          conjure as x11-release build

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib/pkgconfig
          mkdir -p $out/include/bread

          sed -e "s|^prefix=.*|prefix=$out|" -e "s|^libdir=.*|libdir=$out/lib|" lib/x11-release/pkgconfig/bread-x11.pc > $out/lib/pkgconfig/bread-x11.pc
          cp lib/x11-release/libbread-x11.so $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-x11-release-static = pkgs.stdenv.mkDerivation {
        pname = "bread-x11-static";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.gcc
          pkgs.libxcb
          pkgs.libxcb-cursor
          pkgs.libxcb-wm
          pkgs.libxkbcommon
          htils.packages.${system}.htils-threadsafe
          conjure.packages.${system}.default
        ];

        buildPhase = ''
          runHook preBuild

          conjure as x11-release-static build

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib/pkgconfig
          mkdir -p $out/include/bread

          sed -e "s|^prefix=.*|prefix=$out|" -e "s|^libdir=.*|libdir=$out/lib|" lib/x11-release-static/pkgconfig/bread-x11.pc > $out/lib/pkgconfig/bread-x11-static.pc
          cp lib/x11-release-static/libbread-x11.a $out/lib
          cp -r include/bread/* $out/include/bread

          runHook postInstall
        '';
      };

      packages.bread-x11-debug = pkgs.stdenv.mkDerivation {
        pname = "bread-x11-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.gcc
          pkgs.libxcb
          pkgs.libxcb-cursor
          pkgs.libxcb-wm
          pkgs.libxkbcommon
          htils.packages.${system}.htils-threadsafe
        ];

        buildPhase = ''
          runHook preBuild

          conjure as x11-debug build

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          mkdir -p $out/lib/pkgconfig
          mkdir -p $out/include/bread

          sed -e "s|^prefix=.*|prefix=$out|" -e "s|^libdir=.*|libdir=$out/lib|" lib/x11-debug/pkgconfig/bread-x11-debug.pc > $out/lib/pkgconfig/bread-x11-debug.pc
          cp lib/x11-debug/libbread-x11-debug.a $out/lib
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
          export NIX_LDFLAGS="-rpath ${htils.packages.${system}.htils}/lib -rpath ${pkgs.libxcb}/lib -rpath ${pkgs.libxcb-wm}/lib  $NIX_LDFLAGS"


          [[ -f ./include/wayland/xdg-shell-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./include/wayland/xdg-shell-client-protocol.h
          [[ -f ./src/wayland/xdg-shell-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ./src/wayland/xdg-shell-client-protocol.c
          [[ -f ./include/wayland/xdg-decoration-client-protocol.h ]] || wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./include/wayland/xdg-decoration-client-protocol.h
          [[ -f ./src/wayland/xdg-decoration-client-protocol.c ]] || wayland-scanner private-code ${pkgs.wayland-protocols}/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ./src/wayland/xdg-decoration-client-protocol.c
        '';
      };
    });
}
