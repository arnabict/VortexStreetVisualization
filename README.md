# Interactive Visualization Projcect 

Programming framework for the course "Interactive Visualization Project".

## Installation

### Git and CMAKE
Before we begin, you must have Git running, a distributed revision control system which you need to hand in your assignments as well as keeping track of your code changes. We refer you to the online [Pro Git book](https://git-scm.com/book/en/v2) for more information. There you will also find [instructions](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git]) on how to to install it. On Windows, we suggest using [git for windows](https://git-for-windows.github.io/).

CMake is the system this framework uses for cross-platform builds. If you are using Linux or macOS, we recommend installing it with a package manager instead of the CMake download page. E.g. on Debian/Ubuntu:
```
sudo apt-get install cmake
```
or with MacPorts on macOS:
```
sudo port install cmake.
```
On Windows, you can download it from:
[https://cmake.org/download/](https://cmake.org/download/)


### Note for linux users

Many linux distributions do not include `gcc` and the basic development tools in their default installation. On Ubuntu, you need to install the following packages:

```
sudo apt-get install build-essential libx11-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxi-dev libxmu-dev libblas-dev libxinerama-dev libxcursor-dev
```

If you are using linux with a virtual machine on Windows, it is *recommended* to use **Visual Studio** instead.

### Note for Windows users

You can download *Visual Studio 2022 Community* for free from [here](https://visualstudio.microsoft.com/vs/).

### Installing Qt

We will use **Qt** for building our user interface. Go [here](https://www.qt.io/download-open-source) and select *Download the Qt Online Installer*. You will need to create a Qt account to proceed with the installation. During the installation process, select and install **Qt 5.15.2**. The latest Qt version (6.0 and higher) is not tested yet and might not be compatible with VTK.

### Installing VTK

VTK is among the most popular libraries for scientific visualization. We will use VTK for rendering.

Visit [here](http://www.vtk.org/download/) and download the latest version of VTK. VTK has to be built from source. Thus, scroll down to the sources and download them. It is not necessary to download the data sets.

1. Start *cmake-gui*. 
1. Enter into the text box that is labeled with *Where is the source code:* the path to VTK. For instance: `D:/dev/VTK-9.2.6`
1. Enter into the text box that is labeled with *Where to build the binaries:* another arbitrary folder. For instance: `D:/dev/VTK-9.2.6/build`
1. Press *Configure*. If the build folder does not exist yet, CMake will ask you, whether it should create the folder for you. Say, yes.
1. Select a compiler. A window will open with a list of all supported compilers. Select your favorite compiler, e.g., \emph{Visual Studio 17 2022}. Also, if you have a 64-bit system, select the \emph{x64} as optional platform for generator. After selecting the compiler, CMake will run a few diagnostics and test your compiler. If you immediately get an error, you probably selected a compiler that cannot be found on your computer. Check again, whether you selected the right version of your compiler. Expect that the first configuration might take a while. After the first run, CMake will have found and listed numerous configuration options. Every newly found configuration option is highlighted in red. 
1. The new option 'VTK_GROUP_ENABLE_Qt' appeared, on which you select *YES*.
1. Press *Configure* again. In a second run, the red highlights should disappear, since the configuration options have already been found. They only remain red if there are problems, for instance if a dependent library could not be found. If Qt is not found automatically, set as *Qt5_Dir* the path to your Qt installation, for example `D:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5`. If all red highlights disappeared, you can continue to the next step. 
1. Press *Generate*. This button will generate the project files for your selected compiler. If you selected Visual Studio, a solution file (sln) will be created for you in your build folder.
1. Press *Open Project*. This button will open the project using the default IDE.

The following steps are for Visual Studio. For other IDEs the procedure is probably somewhat similar.

1. Select the configuration *Release*.
1. Build the project *ALL_BUILD*. This will compile every library of VTK. This will take a while. (Get coffee and hope that no compiler errors have shown up while you were gone.)
1. (optional) Select the configuration *Debug*.
1. Build the project *ALL_BUILD*. Time for another coffee.


### Cloning the Repository
Before you are able to clone your exercise repository, you need to have an active [gitlab@FAU](https://gitlab.cs.fau.de/) account. Then you can [fork](https://docs.gitlab.com/ee/gitlab-basics/fork-project.html) this project to create your own private online repository.

In the next step you need to clone it to your local hard drive:
```
git clone git@gitlab.cs.fau.de:'Your_Git_Username'/vispro-ss23.git
```
'Your_Git_Username' needs to be replaced accordingly. Cloning recursively will download the Eigen library, as well.

Next, cd into the newly created folder, and run the following commands inside the relevant subfolder to setup the build folder:
```
cd vispro-ss23; mkdir build
cd build
cmake ..
```
On Windows, use the CMAKE gui with the buttons Configure and Generate.
The configuration will probably not find VTK on its own. Set as *VTK_DIR* your build folder of VTK, for example `D:/dev/VTK-9.2.6/build`.

Compile and run the executable, e.g. Ubuntu:
```
make && ./cmd/main
```
Or use your favorite IDE. In case of Visual Studio, you need to open the `build/vispro.sln` file.

### Configuring the Project

This repository contains a demo project that we will use to build a first simple visualization tool. Later, you can customize this demo to visualize your chosen data set. The *cmd* executable is used for preprocessing and the *viewer* is used for data visualization. The library *common* contains a few shared classes that are utilized by both executables.

Download the demo data [here](https://cgl.ethz.ch/Downloads/Data/ScientificData/halfcylinder3d-Re320_vti.zip) (17.9 GB) and unzip the folder.

In both executables, the path to the data is passed in as a command line argument. When running the executables from Visual Studio on Windows you can set the path in the project settings of `cmd` and `viewer` by setting *Properties->Debugging->Command Arguments*, for example to `D:/halfcylinder3d-Re320_vti/` (adjust the paths to your system).

When running the executables from Visual Studio on Windows, certain DLLs from VTK and Qt might not be found. You can add the search paths in the project settings of `cmd` and `viewer` by setting *Properties->Debugging->Environment* to `PATH=%PATH;D:/Dev/VTK-9.2.6/build/bin/$(Configuration);D:/Qt/5.15.2/msvc2019_64/bin` (adjust the paths to your system).


### Update Your Forked Repository

Throughout the semester, new features will be added to this repository. To update your forked repository, check this page: [how-do-i-update-a-github-forked-repository](https://stackoverflow.com/questions/7244321/how-do-i-update-a-github-forked-repository)

Basically, you are required to add our repository as a remote to your own one:
```
git remote add upstream git@gitlab.cs.fau.de:i9vc/teaching/ss23/vispro-ss23.git
```
Then, fetch updates from it:
```
git fetch upstream
```
Lastly, move to your `master` branch and merge updates into yours:
```
git checkout master
git merge upstream/master
```
Note that you need to run the first line *only once* for adding, and the following steps (cmake as well!) should be done again for new updates.