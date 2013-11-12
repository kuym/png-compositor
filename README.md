**png-composite**: A terribly crude PNG compositor that was written for exactly one combination of image
formats in abot one hour.  Could easily be expanded for other uses.  **I recommend using for reference only.**

##Why?
I needed to composite a PNG on an identically-sized PNG with (src-alpha, 1 - src-alpha) blending prior to
printing it to support one particular business process.  I tried with Imagemagick but it turned out to be
faster to write this than to get Imagemagick set up and working, let alone getting it to composite this
particular case, which is so simplistic that a couple lines of C++ can do it.

##How to use
Install libpng in the root directory of the repository and `./configure` and `make` it as you normmally
would.  Then follow the instructions at the top of `png-composite.cpp` to build it.  You may need to change
the paths to libpng if it isn't the same version.