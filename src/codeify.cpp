/**
Copyright (c) 2007 - 2009 Jordan "Earlz/hckr83" Earls  <http://www.Earlz.biz.tm>
Copyright (c) 2019 Qtum Foundation <https://qtum.org>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.
   
THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include <elfloader.h>

using namespace std;


struct ContractMapInfo {
    //This structure is CONSENSUS-CRITICAL
    //Do not add or remove fields nor reorder them!
    uint32_t optionsSize;
    uint32_t codeSize;
    uint32_t dataSize;
    uint32_t reserved;
} __attribute__((__packed__));

bool readElf = false;
bool silent = false;
bool printHex = false;
bool toFile = false;
string outputName = "";
string inputName = "";

int main(int argc, char* argv[]){
    if(argc == 1){
        cout << "qx86-codeify -elf contract-file [-silent] [-o outputfile] [-hex]" << endl;
        return 1;
    }
    bool readOutputFile = false;
    for(int i = 1; i < argc ; i++){
        bool correct = false;
        if(readOutputFile){
            if(argv[i][0] == '-'){
                cerr << "Expected output filename, not the command line option: " << argv[i] << endl;
                return 1;
            }
            outputName = argv[i];
            correct = true;
            readOutputFile = false;
        }
        else{
            if(argv[i][0] != '-'){
                inputName = argv[i];
                correct = true;
            }
        }
        if(strcmp(argv[i], "-elf") == 0){
            readElf = true;
            correct = true;
        }
        if(strcmp(argv[i], "-silent") == 0){
            silent = true;
            correct = true;
        }
        if(strcmp(argv[i], "-hex") == 0){
            printHex = true;
            correct = true;
        }
        if(strcmp(argv[i], "-o") == 0){
            toFile = true;
            correct = true;
            readOutputFile = true;
        }
        if(correct == false){
            cerr << "Invalid command line option: " << argv[i] << endl;
            return 1;
        }

    }
    if(toFile && outputName == ""){
        cerr << "Must specify output file name if using -o" << endl;
        return 1;
    }
    if(!readElf){
        cerr << "Must include -elf" << endl;
        return 1;
    }

    if(inputName == ""){
        cerr << "No input file specified" << endl;
        return 1;
    }

    //read input file
    uint8_t* inputData = nullptr;
    size_t inputLength = 0;
    const size_t maxSize = 0x20000;
    {
        ifstream file(inputName, ios::binary);
        if(!file){
            cout << "file \"" << inputName << "\" does not exist" << endl;
            return 1;
        }
        inputLength = file.tellg();
        file.seekg(0, std::ios::end);
        inputLength = (uint32_t) (((long)file.tellg()) - (long) inputLength);
        inputData = new uint8_t[inputLength];
        file.seekg(0, std::ios::beg);
        file.read((char*)inputData, maxSize);
    }
    ostream *output = &cout;
    if(toFile){
        output = new ofstream(outputName, ios::binary | ios::out);
        if(!output){
            cerr << "file \"" << outputName << "\" could not be opened for writing" << endl;
            return 1;
        }
    }

    const size_t maxCodeSize = 0x10000;
    const size_t maxDataSize = 0x10000;
    uint8_t* code = new uint8_t[maxCodeSize];
    uint8_t* data = new uint8_t[maxDataSize];
    size_t codeSize=0;
    size_t dataSize=0;

    if(readElf){
        //load ELF32 file
        if(!silent){
            cout << "Attempting to load ELF file, should be named .bin if not ELF format" << endl;
        }
        if(!loadElf((char*)code, &codeSize, (char*)data, &dataSize, (char*)inputData, inputLength)){
            cerr << "error loading ELF" << endl;
            return -1;
        }
    }
    /*
    // properly support this later on.. 
    else{
        //load BIN file (no option to load data with bin files)
        if(!rawOutput){
            cout << "Attempting to load BIN file. Warning: It is not possible to load data with this" << endl;
        }
        memcpy(coderom.GetMemory(), fileData, fileLength);
        datasize = 0;
        codesize = fileLength;
    }
    */
    int totalSize = 16 + codeSize + dataSize;
    if(!silent){
        cout << "code: " << codeSize << " data: " << dataSize << endl;
    }
    //todo, refactor to use vectors to make this more consistent
    char *out = new char[totalSize];
    ContractMapInfo map;
    map.optionsSize = 0;
    map.codeSize = codeSize;
    map.dataSize = dataSize;
    map.reserved = 0;
    memset(out, 0, totalSize);
    memcpy(&out[0], &map.optionsSize, sizeof(uint32_t));
    memcpy(&out[4], &map.codeSize, sizeof(uint32_t));
    memcpy(&out[8], &map.dataSize, sizeof(uint32_t));
    memcpy(&out[12], &map.reserved, sizeof(uint32_t));
    memcpy(&out[16], code, codeSize);
    memcpy(&out[16 + codeSize], data, dataSize);
    
    if(!silent){
        cout << "Uncompressed total size: " << dec << totalSize << endl;;
    }
    if(printHex){
        for(int i = 0;i<totalSize;i++){
            *output << hex << setfill('0') << setw(2) << (int)(uint8_t)out[i];
        }
        *output << endl;
    }else{
        for(int i = 0; i < totalSize; i++){
            *output << out[i];
        }
    }
    output->flush();

    delete[] out;

    delete[] code;
    delete[] data;
    delete[] inputData;
    if(output != &cout){
        delete output;
    }

    return 0;
    
}













