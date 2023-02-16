# iFoot3d 
## 3D reconstruction of a foot

This folder contains C++ implementation of the algorithm to generate a 3D model of a human's leg from LIDAR and TrueDepth data.

### Launching the code

#### Prerequisites
* You need **Open3D** to be installed on your PC. Change the variable Open3D_ROOT in *CMakeLists.txt* files if it is installed not in the folder C:/Open3D.

#### To-do

1. Download the folder **data** with this [link](https://drive.google.com/file/d/17CKJ_UXTU1MyAB5VjzoO_km52OjgL4Jn/view?usp=share_link). Unzip the archive in the root of the project.
2. `mkdir build`
3. `cd build`
4. `cmake ..`
5. ` cmake --build . --config Release --parallel 12`
6. Open **build/Release** folder and launch *iFoot3D.exe*