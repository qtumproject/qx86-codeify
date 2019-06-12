# qx86-codeify

This application is used to convert an ELF executable file, such as that output from the GCC toolchain for Qtum-x86, to the simplified contract bytecode format used by Qtum-x86 on the blockchain. This reduces the complexity of an ELF file with various headers, directives, and unneeded data, to just a set of non-mutable code and data as well as a set of mutable data preloaded with a set of initial values. 

# Usage

    qx86-codeify -elf contract-file [-silent] [-o outputfile] [-hex]

Note that not using `-o` and not including `-hex` will print raw binary to the terminal, which can corrupt the current terminal session. 



