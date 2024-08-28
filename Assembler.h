#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>      // IO functionality like std::cin and std::cout
#include <fstream>       // file read/write functionality like std::if(of)stream
#include <string>        // string functionality
#include <unordered_map> // dictionary
#include <vector>        // dynamic, automatically self-resizing array
#include <bitset>        // manipulate fixed number of bits

using namespace std;

class HackAssembler
{
public:
    HackAssembler(const string &inputFile, const string &outputFile)
        : inputFile(inputFile), outputFile(outputFile)
    {
        initializeSymbolTable();
    }

    void assemble() 
    {
        cout << "Assembling started." << endl;
        firstPass(); // populate symbol table with labels
        secondPass(); // convert assembly to machine code
        cout << "Assembling ended." << endl;
    }

private:
    string inputFile;
    string outputFile;
    unordered_map<string, int> symbolTable;
    int nextAvailableAddress = 16; // Variable memory starts at address 16

    // Initialize symbol table with predefined symbols
    void initializeSymbolTable()
    {
        symbolTable["SP"] = 0;
        symbolTable["LCL"] = 1;
        symbolTable["ARG"] = 2;
        symbolTable["THIS"] = 3;
        symbolTable["THAT"] = 4;
        for (int i = 0; i < 16; i++)
        {
            symbolTable["R" + to_string(i)] = i;
        }
        symbolTable["SCREEN"] = 16384;
        symbolTable["KBD"] =24576;
    }

    // Skipping comments and empty lines, populate symbol table with labels
    void firstPass() 
    {
        ifstream infile(inputFile);
        if (!infile.is_open())
        {
            cerr << "Error: Could not open input file " << inputFile << endl;
            return;
        }
        cout << "First pass started. Input file: " << inputFile << endl;

        string line;
        int lineNumber = 0;

        while (getline(infile, line))
        {
            line = strip(line); // Strip whitespace, ignore comments
            if (line.empty() || line[0] == '/')
                continue; // Skip empty lines and comments
            if (line[0] == '(' && line.back() == ')')
            {
                string label = line.substr(1, line.length() - 2);
                symbolTable[label] = lineNumber;
            }
            else
            {
                ++lineNumber; // Count instructions, not label declarations
            }
        }
        infile.close();
    }

    // Translate assembly into binary code
    void secondPass()
    {
        ifstream infile(inputFile);
        ofstream outfile(outputFile);
        if (!infile.is_open())
        {
            cerr << "Error: Could not open input file " << inputFile << endl;
            return;
        }
        cout << "Second pass started. Input file: " << inputFile << endl;

        string line;
        int lineNumber = 0;

        while (getline(infile, line))
        {
            line = strip(line);
            if (line.empty() || line[0] == '/' || (line[0] == '(' && line.back() == ')'))
                continue;
            if (line[0] == '@')
            {
                cout << "Processing A-instruction: " << line << endl;
                string binary = handleAInstruction(line);
                outfile << binary << endl;
                cout << "A-instruction binary: " << binary << endl;
            }
            else
            {
                cout << "Processing C-instruction: " << line << endl;
                string binary = handleCInstruction(line);
                outfile << binary << endl;
                cout << "C-instruction binary: " << binary << endl;
            }
        }
    }

    // Loop through char - if / then comment, adding all char but space to result
    string strip(const string &line)
    {
        string result;
        bool inComment = false;
        for (char c : line)
        {
            if (c == '/')
                inComment == true;
            if (!inComment && !isspace(c))
                result += c;
        }
        cout << "Stripped line: " << result << endl;
        return result;
    }

    // Return 15bit binary code of decimal number
    string handleAInstruction(const string &line)
    {
        string symbol = line.substr(1);
        int address;
        if (isdigit(symbol[0]))
        {
            address = stoi(symbol);
        }
        else
        {
            if (symbolTable.find(symbol) == symbolTable.end())
            {
                symbolTable[symbol] = nextAvailableAddress++;
            }
            address = symbolTable[symbol];
            cerr << symbolTable[symbol] << endl;
        }
        return "0" + bitset<15>(address).to_string();
    }

    // Split line into constituent parts and convert to binary
    string handleCInstruction (const string &line)
    {
        string dest, comp, jump;
        size_t eqPos = line.find("=");
        size_t scPos = line.find(";");

        if (eqPos != string::npos)
        {
            dest = line.substr(0, eqPos);
            comp = line.substr(eqPos + 1, scPos - eqPos -1);
        }
        else
        {
            comp = line.substr(0, scPos);
        }

        if (scPos != string::npos)
        {
            jump = line.substr(scPos + 1);
        }

        return "111" + compBin(comp) + destBin(dest) + jumpBin(jump);
    }

    string destBin(const string &bits)
    {
        unordered_map<string, string> destTable = {
            {"", "000"}, {"M", "001"}, {"D", "010"}, {"MD", "011"}, {"A", "100"}, {"AM", "101"}, {"AD", "110"}, {"AMD", "111"}};
        return destTable[bits];
    }

    string compBin(const string &bits)
    {
        unordered_map<string, string> compTable = {
            {"0", "0101010"}, {"1", "0111111"}, {"-1", "0111010"}, {"D", "0001100"}, {"A", "0110000"}, {"!D", "0001101"}, {"!A", "0110001"}, {"-D", "0001111"}, {"-A", "0110011"}, {"D+1", "0011111"}, {"A+1", "0110111"}, {"D-1", "0001110"}, {"A-1", "0110010"}, {"D+A", "0000010"}, {"D-A", "0010011"}, {"A-D", "0000111"}, {"D&A", "0000000"}, {"D|A", "0010101"}, {"M", "1110000"}, {"!M", "1110001"}, {"-M", "1110011"}, {"M+1", "1110111"}, {"M-1", "1110010"}, {"D+M", "1000010"}, {"D-M", "1010011"}, {"M-D", "1000111"}, {"D&M", "1000000"}, {"D|M", "1010101"}};
        return compTable[bits];
    }

    string jumpBin(const string &bits)
    {
        unordered_map<string, string> jumpTable = {
            {"", "000"}, {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"}, {"JLT", "100"}, {"JNE", "101"}, {"JLE", "110"}, {"JMP", "111"}};
        return jumpTable[bits];
    }
};

#endif // ASSEMBLER_H