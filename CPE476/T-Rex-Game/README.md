First update apt-get, so run the following:
sudo apt-get update
sudo apt-get upgrade

Install via apt-get:
sudo apt-get install libxcursor-dev g++ xorg-dev libglu1-mesa-dev libglfw-dev doxygen doxygen-gui libftgl-dev freetyped2

To install glfw:
git clone https://github.com/glfw/glfw.git to some folder not in t-rex
cd into glfw
sudo apt-get install cmake
cmake .
sudo make install

Do this for the freetype library
sudo ln -s /usr/include/freetype2/freetype/ /usr/include/freetype

For the audio stuff:
sudo apt-get install libjack-dev libasound2-dev libasound-dev

