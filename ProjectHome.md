Simple demo to load up shaders and spin a geometry. Requires OpenGL ES 2.0. Adapted from the gl2jni example from the ndk samples.

Pre-requisites:
1. Android NDK release 4
2. Android API level 7 or above.

Steps to build:
1. Build the libspinningcube.so shared library first. Go into the folder you've checked out your sources into and run ndk-build.
2. Import the project into Eclipse, if your environment is already set to build Android projects. Use the New->Android Project->Create Project from existing source and point to the SpinningCube folder. Finish, build and run!

