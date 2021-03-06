with import <nixpkgs> {};
{
  shellEnv =
    let
      mylibs = [ wayland egl-wayland libGL xorg.libX11 xorg.libXcursor xorg.libXrender libxkbcommon ];
    in
      stdenv.mkDerivation {
        name = "shell-environment";
        nativeBuildInputs = with pkgs; [ pkg-config wayland-protocols gdb clang-tools cosmopolitan ];
        buildInputs = mylibs;
        LIBRARY_PATH=mylibs;
        LD_LIBRARY_PATH=mylibs;
        XDGSHELL="${wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml";
      };
}
