#!/bin/bash

# -- Run:
# > ./install.sh
# -- If it fails due to lack of permissions, do:
# > sudo chmod +x install.sh
# > ./install.sh


# Step 1: Compile the code using "make"
make clean # Ensure fresh compilation
make

if [ $? -ne 0 ]; then
    echo "[-] Compilation failed."
    exit 1
fi

# Step 2: Copy the executable to a directory in the system's PATH
install_dir="/usr/local/bin"
if [ ! -d "$install_dir" ]; then
    sudo mkdir -p "$install_dir"
fi

sudo cp build/minipad "$install_dir"

if [ $? -ne 0 ]; then
    echo "[-] Copying executable to /usr/local/bin failed."
    exit 1
fi

echo "\n[+] Installation completed with success!"
echo " |___ You can now call 'minipad' directly from the terminal."
