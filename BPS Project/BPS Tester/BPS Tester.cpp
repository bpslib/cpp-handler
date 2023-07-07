#include <iostream>

#include "../BPS/BPS.hpp"

int main() {
	//auto bpsStructData = BPS::parse("key1:\"value\";");
	//std::string strData = std::any_cast<std::string>(bpsStructData["key1"]);
	//std::cout << strData;

	std::string data = "key01:\"value\";";
	data += "key02:10;\n";
	data += "key03:10f;\n";
	data += "key04:10.5;\n";
	data += "key05:5.5f;\n";
	data += "key06:5d;\n";
	data += "key07:'a';\n";
	data += "key08:'\'';\n";
	data += "key09:true;\n";
	data += "key10:false;\n";
	data += "key11:[false,true,false];\n";
	//data += "key12:[[10,5],[5,8]];\n";
	//data += "key13:-55;\n";
	//data += "key14:[\"fal\\\"se\"];\n";
	auto bpsStructData = BPS::parse(data);

	//std::string strData = std::any_cast<std::string>(bpsStructData["key1"]);
	//std::cout << strData;

	for (auto& pair : bpsStructData) {
		const std::string& key = pair.first;
		const std::any& value = pair.second;

		std::cout << key << ":";

		if (value.type() == typeid(std::string)) {
			std::cout << std::any_cast<std::string>(value);
		}
		else if (value.type() == typeid(bool)) {
			std::cout << std::any_cast<bool>(value);
		}
		else if (value.type() == typeid(char)) {
			std::cout << std::any_cast<char>(value);
		}
		else if (value.type() == typeid(int)) {
			std::cout << std::any_cast<int>(value);
		}
		else if (value.type() == typeid(float)) {
			std::cout << std::any_cast<float>(value);
		}
		else if (value.type() == typeid(double)) {
			std::cout << std::any_cast<double>(value);
		}
		else if (value.type() == typeid(bool*)) {
			std::cout << std::any_cast<bool*>(value);
		}

		std::cout << ";" << std::endl;
	}
}
