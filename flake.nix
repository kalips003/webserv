{
  description = "A Nix-flake-based C/C++ development environment";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = inputs: let
    supportedSystems = ["x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin"];
    forEachSupportedSystem = f:
      inputs.nixpkgs.lib.genAttrs supportedSystems (system:
        f {
          pkgs = import inputs.nixpkgs { inherit system; };
        });
  in {
    devShells = forEachSupportedSystem ({ pkgs }: let
      stdenv = pkgs.clangStdenv;
    in {
      default =
        pkgs.mkShell.override
        {
          inherit stdenv;
        }
        {
          packages = with pkgs;
            [
              clang-tools
	      bear
	      cppman
	      inetutils
            ]
            ++ (
              if system == "aarch64-darwin"
              then []
              else [gdb valgrind]
            );

          env = {
            CLANGD_FLAGS = "--query-driver=${pkgs.lib.getExe stdenv.cc}";
          };

          shellHook = ''
          '';
        };
    });
  };
}

