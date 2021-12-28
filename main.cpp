#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

struct instruction {

    string name, type, opcode, funct3, funct7;
    int address, rd, src1, src2;
    long long imm;
    bool ecall;
    string ecall_bin;
    vector<int> binary;

    instruction() {
        name = "";
        type = "";
        rd = -1;
        src1 = -1;
        src2 = -1;
        funct3 = "";
        funct7 = "";
        opcode = "";
        address = -1;
        imm = -1;
        ecall = false;
        ecall_bin = "";
    }
};

vector <long long> registers(32, 0);
map <int, vector<int>> mem_data;
map <int, vector<int>> stack;
int addr_instr; //starting address of the instructions
int PC = 0;
bool zero_modify = false;
unordered_map <string, string> instr_type;
unordered_map <string, int> reg_map;
unordered_map <string, int> my_procedures; //key=name, value=address
vector<instruction> my_instructions;
unordered_map <string, string> opcodes;
unordered_map <string, string> funct3;
unordered_map <string, string> funct7;

void init_mapping() {
    reg_map["zero"] = 0; reg_map["ra"] = 1; reg_map["sp"] = 2;  reg_map["gp"] = 3;
    reg_map["tp"] = 4;  reg_map["t0"] = 5;  reg_map["t1"] = 6;  reg_map["t2"] = 7;
    reg_map["s0"] = 8;  reg_map["s1"] = 9;  reg_map["a0"] = 10; reg_map["a1"] = 11;
    reg_map["a2"] = 12; reg_map["a3"] = 13; reg_map["a4"] = 14; reg_map["a5"] = 15;
    reg_map["a6"] = 16; reg_map["a7"] = 17; reg_map["s2"] = 18; reg_map["s3"] = 19;
    reg_map["s4"] = 20; reg_map["s5"] = 21; reg_map["s6"] = 22; reg_map["s7"] = 23;
    reg_map["s8"] = 24; reg_map["s9"] = 25; reg_map["s10"] = 26; reg_map["s11"] = 27;
    reg_map["t3"] = 28; reg_map["t4"] = 29; reg_map["t5"] = 30; reg_map["t6"] = 31;

    ifstream typefile;
    typefile.open("type.txt");

    if (!typefile.is_open()) {
        cout << "Error opening the typefile\n";
    }
    else {
        string line, key, value;;
        while (getline(typefile, line)) {
            stringstream s(line);
            s >> key >> value;
            instr_type[key] = value;
        }
    }
    typefile.close();
    // unordered_map<string, string>::iterator itr;
    // for (itr = instr_type.begin(); itr != instr_type.end(); itr++) {
    //     cout << itr->first << ':' << itr->second;
    //     cout << endl;
    // }
    ifstream opcodefile;
    opcodefile.open("opcode.txt");
    if (!opcodefile.is_open()) {
        cout << "Error opening the opcodefile\n";
    }
    else {
        string line, key, value;;
        while (getline(opcodefile, line)) {
            stringstream s(line);
            s >> key >> value;
            opcodes[key] = value;
        }
    }
    opcodefile.close();

    // cout << "OPCODESSSS " << endl;
    // unordered_map<string, string>::iterator itr;
    // for (itr = opcodes.begin(); itr != opcodes.end(); itr++) {
    //     cout << itr->first << ':' << itr->second;
    //     cout << endl;
    // }
    ifstream funct7file;
    funct7file.open("funct7.txt");
    if (!funct7file.is_open()) {
        cout << "Error opening the funct7file\n";
    }
    else {
        string line, key, value;;
        while (getline(funct7file, line)) {
            stringstream s(line);
            s >> key >> value;
            funct7[key] = value;
        }
    }
    funct7file.close();
    // cout << "funct77777777: " << endl;
    // unordered_map<string, string>::iterator itr;
    // for (itr = funct7.begin(); itr != funct7.end(); itr++) {
    //     cout << itr->first << ':' << itr->second;
    //     cout << endl;
    // }

    ifstream funct3file;
    funct3file.open("funct3.txt");
    if (!funct3file.is_open()) {
        cout << "Error opening the funct3file\n";
    }
    else {
        string line, key, value;;
        while (getline(funct3file, line)) {
            stringstream s(line);
            s >> key >> value;
            funct3[key] = value;
        }
    }
    funct3file.close();

    // cout << "funct33333 " << endl;
    // // unordered_map<string, string>::iterator itr;
    // for (itr = funct3.begin(); itr != funct3.end(); itr++) {
    //     cout << itr->first << ':' << itr->second;
    //     cout << endl;
    // }
}

vector <int> DecimalToBinary(long long n, int size) {
    vector <int> binary(size, 0);
    long long num = n;
    if (n < 0) n = abs(n);
    // Building the binary vector 
    int i = 0;
    while (n > 0 && i < size) {
        binary[i] = n % 2;
        n = n / 2;
        i++;
    }
    // Two's Complement
    if (num < 0)
    {
        int index = 0;
        int k = 0;
        while (index == 0)
        {
            if (binary[k] == 0)
            {
                k++;
            }
            else
                index = ++k;
        }
        for (int l = k; l < size; l++)
        {
            if (binary[l] == 0)
                binary[l] = 1;
            else
                binary[l] = 0;
        }
    }
    return binary;
}

long long BinaryToDecimal(int size, vector <int> binary, bool sign)
{
    bool its_negative = false;
    long long decimal = 0;
    int old_size = binary.size();
    binary.resize(size);
    if (sign && size > old_size)
    {
        for (int i = old_size; i < size; i++)
            binary[i] = binary[old_size - 1];
    }

    if (sign && binary[size - 1] == 1) // Negative number
    {
        its_negative = true;
        //Two's Complement
        int index = 0;
        int k = 0;
        while (index == 0)
        {
            if (binary[k] == 0)
            {
                k++;
            }
            else
                index = ++k;
        }
        for (int l = k; l < size; l++)
        {
            if (binary[l] == 0)
                binary[l] = 1;
            else
                binary[l] = 0;
        }

    }

    // Converting from binary to decimal
    for (int i = 0; i < binary.size(); i++)
        decimal += binary[i] * pow(2, i);
    if (its_negative)
        decimal *= -1;


    return decimal;
}

instruction init_instructions(string name, string variables, int pc) {

    instruction obj;
    obj.name = name;
    stringstream ss(variables);
    // types: U, UJ , SB, I, R, S, ECALL
    if (instr_type[name] == "U") {
        string rd, imm;
        getline(ss, rd, ',');
        if (rd.substr(0, 1) == "x") obj.rd = stoi(rd.substr(1, rd.length() - 1));
        else obj.rd = reg_map[rd];
        getline(ss, imm);
        obj.imm = my_procedures[imm] - pc;
        obj.opcode = opcodes[name];
        obj.type = "U";

    }
    else if (instr_type[name] == "UJ") {
        string rd, imm;
        getline(ss, rd, ',');
        if (rd.substr(0, 1) == "x") obj.rd = stoi(rd.substr(1, rd.length() - 1));
        else obj.rd = reg_map[rd];
        getline(ss, imm);
        obj.imm = my_procedures[imm] - pc;
        obj.opcode = opcodes[name];
        obj.type = "UJ";


    }
    else if (instr_type[name] == "SB") {
        string rs1, rs2, imm;
        getline(ss, rs1, ',');
        if (rs1.substr(0, 1) == "x") obj.src1 = stoi(rs1.substr(1, rs1.length() - 1));
        else obj.src1 = reg_map[rs1];
        getline(ss, rs2, ',');
        if (rs2.substr(0, 1) == "x") obj.src2 = stoi(rs2.substr(1, rs2.length() - 1));
        else obj.src2 = reg_map[rs2];
        getline(ss, imm);
        // handling value of immediate for branching
        obj.imm = (my_procedures[imm] - pc);
        obj.opcode = opcodes[name];
        obj.type = "SB";
        obj.funct3 = funct3[name];

    }
    else if (instr_type[name] == "I") {
        string rs1, rd, imm;
        getline(ss, rd, ',');
        if (rd.substr(0, 1) == "x") obj.rd = stoi(rd.substr(1, rd.length() - 1));
        else obj.rd = reg_map[rd];
        if (variables.find('(') != string::npos) {
            getline(ss, imm, '(');
            obj.imm = stoi(imm);
            getline(ss, rs1, ')');
            if (rs1.substr(0, 1) == "x") obj.src1 = stoi(rs1.substr(1, rs1.length() - 1));
            else obj.src1 = reg_map[rs1];

        }
        else {
            getline(ss, rs1, ',');
            if (rs1.substr(0, 1) == "x") obj.src1 = stoi(rs1.substr(1, rs1.length() - 1));
            else obj.src1 = reg_map[rs1];
            getline(ss, imm);
            obj.imm = stoi(imm);
        }
        obj.type = "I";
        obj.opcode = opcodes[name];
        obj.funct3 = funct3[name];

    }
    else if (instr_type[name] == "R") {
        string rs1, rd, rs2;
        getline(ss, rd, ',');
        if (rd.substr(0, 1) == "x") obj.rd = stoi(rd.substr(1, rd.length() - 1));
        else obj.rd = reg_map[rd];
        getline(ss, rs1, ',');
        if (rs1.substr(0, 1) == "x") obj.src1 = stoi(rs1.substr(1, rs1.length() - 1));
        else obj.src1 = reg_map[rs1];
        getline(ss, rs2);
        if (rs2.substr(0, 1) == "x") obj.src2 = stoi(rs2.substr(1, rs2.length() - 1));
        else obj.src2 = reg_map[rs2];
        obj.type = "R";
        obj.opcode = opcodes[name];
        obj.funct3 = funct3[name];
        obj.funct7 = funct7[name];


    }
    else if (instr_type[name] == "S") {
        string rs1, rs2, imm;
        getline(ss, rs2, ',');
        if (rs2.substr(0, 1) == "x") obj.src2 = stoi(rs2.substr(1, rs2.length() - 1));
        else obj.src2 = reg_map[rs2];
        getline(ss, imm, '(');
        obj.imm = stoi(imm);
        getline(ss, rs1, ')');
        if (rs1.substr(0, 1) == "x") obj.src1 = stoi(rs1.substr(1, rs1.length() - 1));
        else obj.src1 = reg_map[rs1];
        obj.type = "S";
        obj.opcode = opcodes[name];
        obj.funct3 = funct3[name];

    }
    else if (instr_type[name] == "ECALL") {
        obj.ecall = true;
        obj.ecall_bin = "00000000000000000000000001110011";
    }

    return obj;
}

void init_files(string file_name) {
    init_mapping();
    ifstream my_file;
    my_file.open(file_name);

    if (!my_file.is_open()) {
        cout << "Error opening the file\n";
    }
    else {
        string line;
        getline(my_file, line);
        // cout << "This is " << line << " section\n";

        //Reading the .data section
        while (getline(my_file, line)) {
            if (line == ".address")  break;
            if (line.substr(0, 1) == "#") continue;
            if (line.empty()) continue;

            // handling side comments
            if (line.find('#') != string::npos) {
                int pos = line.find('#');
                string ss = line.substr(pos, string::npos);
                line.erase(pos, ss.length());
                int last = line.length() - 1;
                while (last >= 0 && line[last] == ' ')
                    --last;
                line = line.substr(0, last + 1);
            }

            // Erase spaces
            line.erase(remove(line.begin(), line.end(), ' '), line.end());

            stringstream s(line);
            string key, value, bytes;
            getline(s, key, ',');
            getline(s, bytes, ',');
            getline(s, value);

            // cout << "key: " << key << "value: " << value << "bytes: " << bytes << endl;
            vector<int> temp = DecimalToBinary(stoll (value), stoi(bytes) * 8);

            for (int i = 0; i < stoi(bytes); i++) {
                vector<int> range(&temp[i * 8], &temp[i * 8] + 8);
                mem_data[stoi(key) + i] = range;
            }
        }
        //Reading the .address section
        while (getline(my_file, line)) {
            if (line == ".text")  break;
            if (line.substr(0, 1) == "#") continue;
            if (line.empty()) continue;

            // handling side comments
            if (line.find('#') != string::npos) {
                int pos = line.find('#');
                string s = line.substr(pos, string::npos);
                line.erase(pos, s.length());
                int last = line.length() - 1;
                while (last >= 0 && line[last] == ' ')
                    --last;
                line = line.substr(0, last + 1);
            }

            addr_instr = stoi(line);
            // cout << addr_instr;
        }
        //Reading the .text section
        int c = addr_instr;
        vector <string> instr;
        // Handling Procedures
        while (getline(my_file, line)) {
            if (line.empty()) continue;
            if (line.substr(0, 1) == "#") continue;
            int pos = 0;
            if (line.find(':') != string::npos) {
                pos = line.find(':');
                while (line[pos + 1] == ' ') {
                    line.erase(pos + 1, 1);
                }
                if (line[line.length() - 1] == ':') {
                    string name = line.substr(0, line.length() - 1);
                    my_procedures[name] = c;
                    continue;
                }
                else {
                    pos = line.find(':');
                    string proce = line.substr(0, pos);
                    my_procedures[proce] = c;
                    line.erase(0, proce.length() + 1);
                    instr.push_back(line);
                    c += 4;
                    continue;
                }
            }
            c += 4;
            instr.push_back(line);
        }

        // handling comments inside the code and the spaces after each line of code
        for (int i = 0; i < instr.size(); i++) {
            if (instr[i].find('#') != string::npos) {
                // cout << "First: " << instr[i] << endl;
                int pos = instr[i].find('#');
                string s = instr[i].substr(pos, string::npos);
                instr[i].erase(pos, s.length());
                // cout << "length before trim: " << instr[i].length() << endl;
                int last = instr[i].size() - 1;
                while (last >= 0 && instr[i][last] == ' ')
                    --last;
                instr[i] = instr[i].substr(0, last + 1);
                // cout << "Second: " << instr[i] << endl;
                // cout << "length after trim: " << instr[i].length() << endl;

            }
        }
        c = addr_instr;
        for (int i = 0; i < instr.size(); i++) {
            stringstream s(instr[i]);
            string name;
            string variables;
            getline(s, name, ' ');
            getline(s, variables);
            variables.erase(remove(variables.begin(), variables.end(), ' '), variables.end());
            // cout << "c: " << c << " name: " << name  << " rest: "<< variables <<endl;
            instruction obj = init_instructions(name, variables, c);
            obj.address = c;
            my_instructions.push_back(obj);
            c += 4;
        }

        //  cout << "Done readingggg the user input file\n";
    }
    my_file.close();

    // Printing procedures and their addresses
    // unordered_map<string, int>::iterator itr;
    // cout << "hiii procedures: " << endl;
    // for (itr = my_procedures.begin(); itr != my_procedures.end(); itr++) {
    //     cout << itr->first << ':' << itr->second;
    //     cout << endl;
    // }

    // printing the data in the memory
    // map<int, vector<int>>::iterator it;
    // for (it = mem_data.begin(); it!=mem_data.end(); it++){
    //     cout << it->first << ':';
    //     for (int i =0; i < it->second.size(); i++)
    //         cout<<  it->second[i];
    // cout << endl;
    // }

// printing the instructions vector 
    // for (int i = 0; i < my_instructions.size(); i++) {
    //     cout << "instruction number " << i << " :" << endl;
    //     cout << "name: " << my_instructions[i].name << endl;
    //     cout << "address: " << my_instructions[i].address << endl;
    //     cout << "type: " << my_instructions[i].type << endl;
    //     cout << "rs1: " << my_instructions[i].src1 << endl;
    //     cout << "rs2: " << my_instructions[i].src2 << endl;
    //     cout << "rd: " << my_instructions[i].rd << endl;
    //     cout << "imm : " << my_instructions[i].imm << endl;
    //     cout << "opcode: " << my_instructions[i].opcode << endl;
    //     cout << "funct3: " << my_instructions[i].funct3 << endl;
    //     cout << "funct7: " << my_instructions[i].funct7 << endl;
    //     cout << "whether ecall? " << my_instructions[i].ecall << endl;
    //     if (my_instructions[i].ecall) cout << "ecall bin: " << my_instructions[i].ecall_bin << endl;
    // }

}

void I_exec(instruction obj) {

    // cout << "Instruction type is I, Name --> " << obj.name << endl;

    if (obj.rd == 0) zero_modify = true;

    int num1 = BinaryToDecimal(32, DecimalToBinary(registers[obj.src1], 32), false);

    if (obj.name == "addi") {
        registers[obj.rd] = registers[obj.src1] + obj.imm;
    }

    else if (obj.name == "slti") {
        if (registers[obj.src1] < obj.imm)
            registers[obj.rd] = 1;
        else registers[obj.rd] = 0;
    }

    else if (obj.name == "sltiu") {
        int num = BinaryToDecimal(12, DecimalToBinary(registers[obj.src1], 12), false);
        if (num < obj.imm)
            registers[obj.rd] = 1;
        else registers[obj.rd] = 0;
    }

    else if (obj.name == "xori") {
        registers[obj.rd] = registers[obj.src1] ^ obj.imm;
    }

    else if (obj.name == "ori") {
        registers[obj.rd] = registers[obj.src1] | obj.imm;
    }

    else if (obj.name == "andi") {
        registers[obj.rd] = registers[obj.src1] & obj.imm;
    }

    else if (obj.name == "slli") {
        registers[obj.rd] = registers[obj.src1] * pow(2, obj.imm);
    }

    else if (obj.name == "srli") {
        registers[obj.rd] = num1 / pow(2, obj.imm);
    }

    else if (obj.name == "srai") {
        registers[obj.rd] = registers[obj.src1] / pow(2, obj.imm);
    }

    else if (obj.name == "jalr") {
        // updating returning address jalr x0, 0(x3) and putting it in x0 cuz we don't care
        registers[obj.rd] = obj.address;
        PC = registers[obj.src1] + (obj.imm) + 4;       //jump to 0(x3)
    }

    else if (obj.name == "lb") {
        registers[obj.rd] = BinaryToDecimal(32, mem_data[registers[obj.src1] + obj.imm], true);
        // lb x2, 1(x3)
    }

    else if (obj.name == "lh") {
        vector <int> temp(8);
        temp = mem_data[registers[obj.src1] + obj.imm];
        vector <int> temp2 = mem_data[registers[obj.src1] + obj.imm + 1];
        temp.insert(temp.end(), temp2.begin(), temp2.end());
        registers[obj.rd] = BinaryToDecimal(32, temp, true);

    }

    //lw x9, 0(x8)
    //x8 = 1000
    else if (obj.name == "lw") {
        vector <int> tempp(0);
        vector <int> temp(8);
        for (int i = 0; i < 4; i++) {
            if (obj.src1 != 2)
                temp = mem_data[registers[obj.src1] + obj.imm + i];
            else
                temp = stack[registers[obj.src1] + obj.imm + i];
            tempp.insert(tempp.end(), temp.begin(), temp.end());
        }
        registers[obj.rd] = BinaryToDecimal(32, tempp, true);

    }
    else if (obj.name == "lbu") {
        if (obj.src1 != 2)
            registers[obj.rd] = BinaryToDecimal(32, mem_data[registers[obj.src1] + obj.imm], false);
        else
            registers[obj.rd] = BinaryToDecimal(32, stack[registers[obj.src1] + obj.imm], false);
    }

    else if (obj.name == "lhu") {
        vector <int> temp(8); 
        vector <int> temp2 (8);
        if (obj.src1 != 2)
        {
            temp = mem_data[registers[obj.src1] + obj.imm];
            temp2 = mem_data[registers[obj.src1] + obj.imm + 1];
        }
        else
        {
            temp = stack[registers[obj.src1] + obj.imm];
            temp2 = stack[registers[obj.src1] + obj.imm + 1];
        }
        temp.insert(temp.end(), temp2.begin(), temp2.end());
        registers[obj.rd] = BinaryToDecimal(32, temp, false);
    }

    if (obj.name != "jalr")
        PC += 4;
}

void U_exec(instruction obj)
{
    //cout << "Instruction type is U, Name --> " << obj.name << endl;
    if (obj.rd == 0) zero_modify = true;

    if (obj.name == "lui") {
        registers[obj.rd] = 0;
        registers[obj.rd] = obj.imm * pow(2, 12);
    }

    else if (obj.name == "auipc") {
        registers[obj.rd] = PC + obj.imm * pow(2, 12);
    }
    PC += 4;
}

void UJ_exec(instruction obj)
{
    //cout << "Instruction type is UJ, Name --> " << obj.name << endl;
    if (obj.rd == 0) zero_modify = true;

    if (obj.name == "jal") {
        PC = PC + obj.imm;
        registers[obj.rd] = obj.address;
    }
}

void R_exec(instruction obj)
{
    //cout << "Instruction type is R, Name --> " << obj.name << endl;
    // add sub sll slt sltu xor srl sra or and 
    int sourse1 = obj.src1;
    int sourse2 = obj.src2;
    long long num1 = BinaryToDecimal(32, DecimalToBinary(registers[sourse1], 32), false);
    long long num2 = BinaryToDecimal(32, DecimalToBinary(registers[sourse2], 32), false);
    int dist = obj.rd;
    if (dist == 0)
        zero_modify = true;

    if (obj.name == "add") {
        registers[dist] = registers[sourse1] + registers[sourse2];
    }

    else if (obj.name == "sub") {
        registers[dist] = registers[sourse1] - registers[sourse2];
    }

    else if (obj.name == "mul") {
        registers[dist] = registers[sourse1] * registers[sourse2];

    }

    else if (obj.name == "sll") {
        registers[dist] = registers[sourse1] * pow(2, num2);

    }

    else if (obj.name == "slt") {
        if (registers[sourse1] < registers[sourse2])
            registers[dist] = 1;
        else
            registers[dist] = 0;
    }

    else if (obj.name == "sltu") {
        if (num1 < num2)
            registers[dist] = 1;
        else
            registers[dist] = 0;
    }

    else if (obj.name == "xor") {
        registers[dist] = registers[sourse1] ^ registers[sourse2];
    }

    else if (obj.name == "srl") {
        registers[dist] = num1 / pow(2, num2);
    }

    else if (obj.name == "sra") {
        registers[dist] = registers[sourse1] / pow(2, num2);

    }

    else if (obj.name == "or") {
        registers[dist] = registers[sourse1] | registers[sourse2];
    }

    else if (obj.name == "and") {
        registers[dist] = registers[sourse1] & registers[sourse2];
    }

    PC += 4;
}

void S_exec(instruction obj)
{
    //cout << "Instruction type is S, Name --> " << obj.name << endl;
    // sb sh sw
    int to_address = registers[obj.src1];
    int value = registers[obj.src2];
    int offset = obj.imm;
    int final_address = to_address + offset;
    vector <int> value8bits;
    //sw x1, 4(x2)
    if (obj.name == "sb") {
        value8bits = DecimalToBinary(value, 8);
        if (obj.src1 != 2)
           mem_data[final_address] = value8bits;
        else
           stack[final_address] = value8bits;

    }

    else if (obj.name == "sh") {
        for (int i = 0; i < 2; i++)
        {
            value8bits = DecimalToBinary(value, 8);
            if (obj.src1 != 2)
                mem_data[final_address + i] = value8bits;
            else
                stack[final_address + i] = value8bits;
            value = value / pow(2, 8);
        }
    }

    else if (obj.name == "sw") {
        for (int i = 0; i < 4; i++)
        {
            value8bits = DecimalToBinary(value, 8);
            if (obj.src1 != 2)
                mem_data[final_address + i] = value8bits;
            else
                stack[final_address + i] = value8bits;
            value = value / pow(2, 8);
        }
    }
    PC += 4;

}

void SB_exec(instruction obj)
{
    //cout << "Instruction type is SB, Name --> " << obj.name << endl;
    // beq bne blt bge bltu bgeu
    int sourse1 = obj.src1;
    int sourse2 = obj.src2;
    long long num1 = BinaryToDecimal(32, DecimalToBinary(registers[sourse1], 32), false);
    long long num2 = BinaryToDecimal(32, DecimalToBinary(registers[sourse2], 32), false);
    int imm = obj.imm;
    if (obj.name == "beq") {
        if (registers[sourse1] == registers[sourse2])
            PC += imm;
        else
            PC += 4;
    }

    else if (obj.name == "bne") {
        if (registers[sourse1] != registers[sourse2])
            PC += imm;
        else
            PC += 4;
    }

    else if (obj.name == "blt") {
        if (registers[sourse1] < registers[sourse2])
            PC += imm;
        else
            PC += 4;
    }

    else if (obj.name == "bge") {
        if (registers[sourse1] >= registers[sourse2])
            PC += imm;
        else
            PC += 4;
    }

    else if (obj.name == "bltu") {
        if (num1 < num2)
            PC += imm;
        else
            PC += 4;
    }

    if (obj.name == "bgeu") {
        if (num1 >= num2)
            PC += imm;
        else
            PC += 4;
    }
}

void printing_registers()
{
    for (int i = 0; i < 32; i++)
        cout << "Register " << i << ": " << registers[i] << endl;
}

void execution()
{
    PC = addr_instr;
    int instruction_num = my_instructions.size();
    int i = (PC - addr_instr) / 4;
    while (PC < instruction_num * 4 + addr_instr)
    {
        cout << "Instruction number : " << i << " --> " << my_instructions[i].name << endl;
        if (my_instructions[i].ecall)
        {
            cout << "The execution has been stopped due to the ECALL instruction" << endl;
            break;
        }

        // Errors
        if (zero_modify)
        {
            cout << "Warning: Register 0 cannot be modified by this line execution" << endl;
        }

        if (registers[0] != 0)
        {
            registers[0] = 0;
        }

        if (my_instructions[i].type == "")
        {
            cout << "ERROR: '" << my_instructions[i].name << "' is not defined" << endl;
            break;
        }

cout << "Current program counter: " << PC << endl;

        if (my_instructions[i].type == "I")
            I_exec(my_instructions[i]);
        else if (my_instructions[i].type == "R")
            R_exec(my_instructions[i]);
        else if (my_instructions[i].type == "S")
            S_exec(my_instructions[i]);
        else if (my_instructions[i].type == "SB")
            SB_exec(my_instructions[i]);
        else if (my_instructions[i].type == "UJ")
            UJ_exec(my_instructions[i]);
        else if (my_instructions[i].type == "U")
            U_exec(my_instructions[i]);
        //else 
         //Not a legal function (handle as an error or pseudo code)

        i = (PC - addr_instr) / 4;

        printing_registers();

        if(mem_data.size()!=0) cout << endl<< "The Contents of the Memory now: "<<endl;
        map<int, vector<int>>::iterator it;
        for (it = mem_data.begin(); it!=mem_data.end(); it++){
             cout << it->first << ':';
             for (int i =0; i < it->second.size(); i++)
                 cout<<  it->second[i];
         cout << endl;
         }
    
    cout <<endl <<endl;
    }
}

void machine_code()
{
    vector <int> imm;
    vector <int> source1(5);
    vector <int> source2(5);
    vector <int> dist(5);
    vector <int> first_byte;
    vector <int> second_byte;
    vector <int> third_byte;
    vector <int> forth_byte;

    for (int i = 0; i < my_instructions.size(); i++)
    {
        vector <int> code(32, 0);

        if (my_instructions[i].type == "R")
        {
            source1 = DecimalToBinary(my_instructions[i].src1, 5);
            source2 = DecimalToBinary(my_instructions[i].src2, 5);
            dist = DecimalToBinary(my_instructions[i].rd, 5);

            for (int j = 0; j < 7; j++)
            {
                // opcode vector
                if (my_instructions[i].opcode[6 - j] == '1')
                    code[j] = 1;
                else code[j] = 0;

                // fun7 vector
                if (my_instructions[i].funct7[6 - j] == '1')
                    code[j + 25] = 1;
                else code[j + 25] = 0;

                // fun3 vector
                if (j < 3)
                {
                    if (my_instructions[i].funct3[2 - j] == '1')
                        code[j + 12] = 1;
                    else code[j + 12] = 0;
                }

                // source1 & source2 & dist
                if (j < 5)
                {
                    code[j + 7] = dist[j];
                    code[j + 15] = source1[j];
                    code[j + 20] = source2[j];
                }
            }
        }

        else if (my_instructions[i].type == "I")
        {
            source1 = DecimalToBinary(my_instructions[i].src1, 5);
            dist = DecimalToBinary(my_instructions[i].rd, 5);
            imm.resize(12);
            imm = DecimalToBinary(my_instructions[i].imm, 12);

            for (int j = 0; j < 12; j++)
            {
                // imm vector
                code[j + 20] = imm[j];

                // opcode vector
                if (j < 7)
                {
                    if (my_instructions[i].opcode[6 - j] == '1')
                        code[j] = 1;
                    else code[j] = 0;
                }


                // fun3 vector
                if (j < 3)
                {
                    if (my_instructions[i].funct3[2 - j] == '1')
                        code[j + 12] = 1;
                    else code[j + 12] = 0;
                }

                // source1 & dist
                if (j < 5)
                {
                    code[j + 7] = dist[j];
                    code[j + 15] = source1[j];
                }
            }
        }

        else if (my_instructions[i].type == "S" || my_instructions[i].type == "SB")
        {
            source1 = DecimalToBinary(my_instructions[i].src1, 5);
            source2 = DecimalToBinary(my_instructions[i].src2, 5);
            imm.resize(13);
            imm = DecimalToBinary(my_instructions[i].imm, 13);

            for (int j = 0; j < 12; j++)
            {
                // imm vector
                if (my_instructions[i].type == "S")
                {
                    if (j < 5)
                        code[j + 7] = imm[j];
                    else
                        code[j + 25 - 5] = imm[j];
                }

                else if (my_instructions[i].type == "SB")
                {
                    code[7] = imm[11];
                    code[31] = imm[12];
                    if (j < 5 && j >= 1)
                        code[j + 8 - 1] = imm[j];
                    if (j >= 5 && j < 11)
                        code[j + 25 - 5] = imm[j];
                }

                // opcode vector
                if (j < 7)
                {
                    if (my_instructions[i].opcode[6 - j] == '1')
                        code[j] = 1;
                    else code[j] = 0;
                }


                // fun3 vector
                if (j < 3)
                {
                    if (my_instructions[i].funct3[2 - j] == '1')
                        code[j + 12] = 1;
                    else code[j + 12] = 0;
                }

                // source1 & source2
                if (j < 5)
                {
                    code[j + 20] = source2[j];
                    code[j + 15] = source1[j];
                }
            }
        }

        else if (my_instructions[i].type == "U")
        {
            imm.resize(32);
            imm = DecimalToBinary(my_instructions[i].imm, 32);
            dist = DecimalToBinary(my_instructions[i].rd, 5);
            for (int j = 0; j < 32; j++)
            {
                // imm vector
                if (j >= 12)
                    code[j] = imm[j];

                // opcode vector
                if (j < 7)
                {
                    if (my_instructions[i].opcode[6 - j] == '1')
                        code[j] = 1;
                    else code[j] = 0;
                }

                // dist
                if (j < 5)
                {
                    code[j + 7] = dist[j];
                }
            }

        }

        else if (my_instructions[i].type == "UJ")
        {
            imm.resize(21);
            imm = DecimalToBinary(my_instructions[i].imm, 21);
            dist = DecimalToBinary(my_instructions[i].rd, 5);

            code[20] = imm[11];
            code[31] = imm[20];

            for (int j = 0; j < 20; j++)
            {
                // imm vector
                if (j < 20 && j >= 12)
                    code[j] = imm[j];
                if (j >= 1 && j < 11)
                    code[j + 21 - 1] = imm[j];

                // opcode vector
                if (j < 7)
                {
                    if (my_instructions[i].opcode[6 - j] == '1')
                        code[j] = 1;
                    else code[j] = 0;
                }

                // dist
                if (j < 5)
                {
                    code[j + 7] = dist[j];
                }
            }
        }

        else if (my_instructions[i].name == "ECALL")
        {
            for (int j = 0; j < 32; j++)
            {
                // ecall vector
                if (my_instructions[i].ecall_bin[31 - j] == '1')
                    code[j] = 1;
                else code[j] = 0;

            }
        }
        // Storing the machine code into the memory
        vector <int>::iterator it = code.begin();
        first_byte.assign(it, it + 8);
        second_byte.assign(it + 8, it + 16);
        third_byte.assign(it + 16, it + 24);
        forth_byte.assign(it + 24, it + 32);
        my_instructions[i].binary = code;
        mem_data[my_instructions[i].address] = first_byte;
        mem_data[my_instructions[i].address + 1] = second_byte;
        mem_data[my_instructions[i].address + 2] = third_byte;
        mem_data[my_instructions[i].address + 3] = forth_byte;
    }
}

void printing_machine() {
    cout << endl;
    cout << "Machine code of the program in the memory:" << endl;
    vector <int> byte(8);
    for (int i = 0; i < my_instructions.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cout << my_instructions[i].address + j << " : ";
            byte = mem_data[my_instructions[i].address + j];
            for (int k = 7; k >= 0; k--)
                cout << byte[k] << " ";
            cout << endl;
        }
        cout << endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2)
        cout << "you need to provide a file name!" << endl;
    
    init_files(argv[1]);
    execution();
    machine_code();
    printing_machine();
    return 0;
}