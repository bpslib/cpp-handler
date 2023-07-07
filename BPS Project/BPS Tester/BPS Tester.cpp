#include <iostream>

#include "../BPS/BPS.hpp"

int main() {
    std::string data = "key1:\"value\";";
    //data += "key2:10;\n";
    //data += "key3:10f;\n";
    //data += "key4:10.5;\n";
    //data += "key5:5.5f;\n";
    //data += "key6:5d;\n";
    //data += "key7:'a';\n";
    //data += "key8:'\'';\n";
    //data += "key9:true;\n";
    //data += "key10:false;\n";
    //data += "key11:[false,true,false];\n";
    //data += "key12:[[10,5],[5,8]];\n";
    //data += "key13:-55;\n";
    //data += "key14:[\"fal\\\"se\"];\n";
    auto bpsStructData = BPS::parse(data);


    std::string strData = std::any_cast<std::string>(bpsStructData["key"]);
    std::cout << strData;

    //for (auto it = bpsStructData.begin(); it != bpsStructData.end(); ++it) {
    //    std::cout << "Chave: " << it->first << ", Valor: " << std::any_cast<void*>(it->second) << std::endl;
    //}
}
