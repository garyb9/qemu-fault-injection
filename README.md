# QEMU-Fault-Injection
Directories:

Debug_Qemu - Contains a code blocks project which just includes all directories in qemu_stm32 and stm32_p103_demos for debugging and dependency tracking.

Smartlock PC Apps - Contains the PC App which modifies and activates the chosen Demo (Smartlock A, B or C), which gets its arguments from terminal flags. After running the script, will output a statistic of the Perturbation attacks.

qemu_stm32 - Forked from : https://github.com/beckus/qemu_stm32/tree/b37686f7e84b22cfaf7fd01ac5133f2617cc3027
Contains the main QEMU simulation, mainly used for Cortex M3.

stm32_p103_demos - Forked from(and modified): https://github.com/beckus/stm32_p103_demos 
Cotains demos for the simulation to run on, some of which are the Smartlocks, that aim to test the security procedures of the Cortex M3 (over the simulation).

New OS run this command:
@
sudo apt-get install virtualbox-guest-dkms virtualbox-guest-utils git vim curl flex bison libgmp3-dev libmpfr-dev texinfo libelf-dev autoconf build-essential libncurses5-dev libmpc-dev python pkg-config libglib2.0-dev libpixman-1-dev gcc-arm-none-eabi
@

Steps for making (cd in this folder):
@
cd qemu_stm32/ 
./configure --enable-debug --target-list="arm-softmmu" --disable-werror
make
cd ../stm32_p103_demos/
make
cd ..
@

Steps for running (cd in this folder):
Run program from Smartlock PC Apps/smartlock/SmartLock.cbp (need codeblocks installed)
or
from terminal (example arguments):
@
cd Smartlock\ PC\ Apps/smartlock/bin/Debug/
./Smartlock 10 100 1 0 0 2000
@



