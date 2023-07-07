#include "pch.h"
#include "bps_core.hpp"

namespace bps_core {

	token::token(const token_category& category, const std::string& image, int line, int collumn)
		: category(category), image(image), line(line), collumn(collumn) {
	}

	bool token::operator==(const token& other) const {
		return category == other.category && image == other.image && line == other.line && collumn == other.collumn;
	}

	bool bps_core::is_skip(char c) {
		//int size = sizeof(skip) / sizeof(skip[0]);
		//for (int i = 0; i < size; ++i) {
		//	if (c == skip[i]) {
		//		return true;
		//	}
		//}
		//return false;
		auto fc = std::find(std::begin(skip), std::end(skip), c);
		return fc != std::end(skip);
	}

	std::string build_lexer_error_message(std::string problem, int line, int collum) {
		std::stringstream msg;
		msg << problem;
		msg << " at line ";
		msg << line;
		msg << " and collumn ";
		msg << collum;
		msg << ".";
		return msg.str();
	}

	std::vector<token> lexer::tokens = std::vector<token>();
	std::string lexer::_input = std::string();
	char lexer::_curChar;
	int lexer::_curIndex = 0;
	int lexer::_curLine = 1;
	int lexer::_curCollumn = 1;

	std::vector<token> lexer::tokenize(std::string input) {
		_input = input;

		next_char();
		while (!end_of_input()) {
			// to skip the skip chars
			if (is_skip(_curChar)) {
				if (_curChar == symbols::NEWLINE) {
					next_line();
				}
				next_char();
				continue;
			}

			// to skip comments
			if (_curChar == HASH) {
				while (!end_of_input() and _curChar != symbols::NEWLINE) {
					next_char();
				}
				next_line();
				next_char();
			} else if (std::isalpha(_curChar) or _curChar == symbols::UNDERSCORE) { // key, boolean or null
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				next_char();

				// loops the key
				while (!end_of_input() and _curChar != symbols::COLON and (_curChar == symbols::UNDERSCORE or std::isalnum(_curChar))) {
					lexeme += _curChar;
					next_char();
				}

				// true or false
				if (lexeme == "true" or lexeme == "false") {
					tokens.push_back(token(token_category::BOOL, lexeme, _curLine, initCol));
				} else if (lexeme == "null") { // null 
					tokens.push_back(token(token_category::T_NULL, lexeme, _curLine, initCol));
				} else { // key
					tokens.push_back(token(token_category::KEY, lexeme, _curLine, initCol));
				}
			}
			// open array
			else if (_curChar == symbols::LEFT_BRACKETS) {
				tokens.push_back(token(token_category::OPEN_ARRAY, std::string(&_curChar), _curLine, _curCollumn));
				next_char();
			} else if (_curChar == symbols::RIGHT_BRACKETS) { // close array
				tokens.push_back(token(token_category::CLOSE_ARRAY, std::string(&_curChar), _curLine, _curCollumn));
				next_char();
			} else if (_curChar == symbols::SEMICOLON) { // end of data
				tokens.push_back(token(token_category::END_OF_DATA, std::string(&_curChar), _curLine, _curCollumn));
				next_char();
			} else if (_curChar == symbols::COMMA) { // array sep
				tokens.push_back(token(token_category::ARRAY_SEP, std::string(&_curChar), _curLine, _curCollumn));
				next_char();
			} else if (_curChar == symbols::COLON) { // data sep
				tokens.push_back(token(token_category::DATA_SEP, std::string(&_curChar), _curLine, _curCollumn));
				next_char();
			} else if (_curChar == symbols::DQUOTE) { // string
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				auto beforeChar = _curChar;
				next_char();
				while (!end_of_input() and (_curChar != symbols::DQUOTE or beforeChar == '\\')) {
					beforeChar = _curChar;
					lexeme += _curChar;
					next_char();
				}
				if (_curChar != symbols::DQUOTE) {
					throw std::invalid_argument(build_lexer_error_message("String was not closed", _curLine, _curCollumn));
				}
				lexeme += _curChar;
				tokens.push_back(token(token_category::STRING, lexeme, _curLine, initCol));
				next_char();
			} else if (_curChar == symbols::QUOTE) { // char
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				next_char();
				if (_curChar == '\\') {
					lexeme += _curChar;
					next_char();
					if (_curChar != symbols::QUOTE) {
						std::stringstream msg;
						msg << "Unexpected character '";
						msg << _curChar;
						msg << "' encountered";
						throw std::invalid_argument(build_lexer_error_message(msg.str(), _curLine, _curCollumn));
					}
				}
				lexeme += _curChar;
				next_char();
				if (_curChar != symbols::QUOTE) {
					throw std::invalid_argument(build_lexer_error_message("Char was not closed", _curLine, _curCollumn));
				}
				lexeme += _curChar;
				tokens.push_back(token(token_category::CHAR, lexeme, _curLine, initCol));
				next_char();
			} else if (std::isdigit(_curChar) or _curChar == symbols::DOT or _curChar == symbols::MINUS) { // numeric
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				auto dotted = _curChar == symbols::DOT;
				next_char();
				while (!end_of_input() and (std::isdigit(_curChar) or _curChar == symbols::DOT)) {
					if (_curChar == symbols::DOT) {
						if (dotted) {
							throw std::invalid_argument(build_lexer_error_message("Double dot encountered", _curLine, _curCollumn));
						} else {
							dotted = true;
						}
					}
					lexeme += _curChar;
					next_char();
				}
				if (std::tolower(_curChar) == 'f' or std::tolower(_curChar) == 'd') {
					lexeme += _curChar;
					next_char();
				}
				std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::tolower);
				// float, double or int
				if (lexeme.find('f')) {
					tokens.push_back(token(token_category::FLOAT, lexeme, _curLine, initCol));
				} else if (lexeme.find(symbols::DOT) or lexeme.find('d')) {
					tokens.push_back(token(token_category::DOUBLE, lexeme, _curLine, initCol));
				} else {
					tokens.push_back(token(token_category::INTEGER, lexeme, _curLine, initCol));
				}
			} else {
				std::stringstream msg;
				msg << "Invalid character '";
				msg << _curChar;
				msg << "' encountered";
				throw std::invalid_argument(build_lexer_error_message(msg.str(), _curLine, _curCollumn));
			}
		}

		tokens.push_back(token(token_category::T_EOF, std::string(), -1, -1));

		return tokens;
	}

	bool lexer::end_of_input() {
		return _curIndex > _input.length();
	}

	void lexer::next_line() {
		++_curLine;
		_curCollumn = 0;
	}

	void lexer::next_char() {
		if (_curIndex < _input.length()) {
			_curChar = _input[_curIndex];
			++_curCollumn;
		}
		++_curIndex;
	}


	std::string build_parser_error_message(std::string image, int line, int collum, std::string expected) {
		std::stringstream msg;
		msg << "Invalid token '";
		msg << image;
		msg << "' encountered at line ";
		msg << line;
		msg << " and collumn ";
		msg << collum;
		msg << ". Expected ";
		msg << expected;
		msg << ".";
		return msg.str();
	}

	std::map<std::string, std::any> parser::parsedData = std::map<std::string, std::any>();
	std::vector<token> parser::_tokens;
	token parser::_curToken;
	int parser::_curIndex = -1;
	std::string parser::_key;
	std::any parser::_value;
	std::stack<std::vector<std::any>> parser::_arrStack = std::stack<std::vector<std::any>>();
	const int parser::CONTEXT_KEY = 0;
	const int parser::CONTEXT_ARRAY = 1;
	int parser::_context = CONTEXT_KEY;

	std::map<std::string, std::any> parser::parse(std::string data) {
		_tokens = lexer::tokenize(data);
		start();
		return parsedData;
	}

	void parser::start() {
		next_token();
		statement();
		consume_token(token_category::T_EOF);
	}

	void parser::statement() {
		switch (_curToken.category) {
			case token_category::KEY:
				Key();
				break;
			default:
				break;
		}
	}

	void parser::Key() {
		_key = _curToken.image;
		next_token();
		consume_token(token_category::DATA_SEP);
		value();
		consume_token(token_category::END_OF_DATA);
		statement();
	}

	void parser::value() {
		switch (_curToken.category) {
			case token_category::OPEN_ARRAY:
				open_array();
				next_token();
				tarray();
				break;
			case token_category::STRING:
				tstring();
				break;
			case token_category::CHAR:
				tchar();
				break;
			case token_category::INTEGER:
				tinteger();
				break;
			case token_category::FLOAT:
				tfloat();
				break;
			case token_category::DOUBLE:
				tdouble();
				array_sel();
				break;
			case token_category::BOOL:
				tbool();
				break;
			case token_category::T_NULL:
				tnull();
				break;
			default:
				build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, "a value or array");
		}
	}

	void parser::tarray() {
		switch (_curToken.category) {
			case token_category::OPEN_ARRAY:
				open_array();
				next_token();
				tarray();
				break;
			case token_category::STRING:
				tstring();
				array_sel();
				break;
			case token_category::CHAR:
				tchar();
				array_sel();
				break;
			case token_category::INTEGER:
				tinteger();
				array_sel();
				break;
			case token_category::FLOAT:
				tfloat();
				array_sel();
				break;
			case token_category::DOUBLE:
				tdouble();
				array_sel();
				break;
			case token_category::BOOL:
				tbool();
				array_sel();
				break;
			case token_category::T_NULL:
				tnull();
				array_sel();
				break;
			default:
				build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, "a value or array");
		}
	}

	void parser::array_sel() {
		switch (_curToken.category) {
			case token_category::ARRAY_SEP:
				next_token();
				tarray();
				break;
			case token_category::CLOSE_ARRAY:
				close_array();
				next_token();
				array_sel();
				break;
			case token_category::END_OF_DATA:
			case token_category::T_EOF:
				break;
			default:
				build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, "',', ']' or ';'");
		}
	}

	void parser::tstring() {
		std::string strValue = _curToken.image.substr(1, _curToken.image.length() - 2);
		_value = strValue;
		set_value();
	}

	void parser::tchar() {
		std::string val = _curToken.image.substr(1, _curToken.image.length() - 2);
		val = std::regex_replace(val, std::regex("\\\\"), "");
		char cValue = val[0];
		_value = cValue;
		set_value();
	}

	void parser::tinteger() {
		int intValue = std::stoi(_curToken.image);
		_value = intValue;
		set_value();
	}

	void parser::tfloat() {
		auto image = _curToken.image;
		std::transform(image.begin(), image.end(), image.begin(), ::tolower);
		auto strValue = image.length() > 0 and image.back() == 'f' ? image.substr(0, image.length() - 1) : image;
		float floatValue = std::stof(strValue);
		_value = floatValue;
		set_value();
	}

	void parser::tdouble() {
		auto image = _curToken.image;
		std::transform(image.begin(), image.end(), image.begin(), ::tolower);
		auto strValue = image.length() > 0 and image.back() == 'd' ? image.substr(0, image.length() - 1) : image;
		double doubleValue = std::stod(strValue);
		_value = doubleValue;
		set_value();
	}

	void parser::tbool() {
		bool boolValue = _curToken.image == "true";
		_value = boolValue;
		set_value();
	}

	void parser::tnull() {
		_value = nullptr;
		set_value();
	}

	void parser::set_value() {
		if (_context == CONTEXT_ARRAY) {
			_arrStack.top().push_back(_value);
		} else {
			parsedData.emplace(_key, _value);
		}
		next_token();
	}

	void parser::open_array() {
		if (_arrStack.size() == 0) {
			_context = CONTEXT_ARRAY;
			_arrStack.push(std::vector<std::any>());
		} else {
			auto newD = std::vector<std::any>();
			_arrStack.top().push_back(&newD);
			_arrStack.push(newD);
		}
	}

	void parser::close_array() {
		if (_arrStack.size() > 1) {
			_arrStack.pop();
		} else {
			_context = CONTEXT_KEY;
			auto list = _arrStack.top();
			std::any* arr;
			arr = new std::any[list.size()];
			for (int i = 0; i < list.size(); ++i) {
				arr[i] = list[i];
			}
			_arrStack.pop();
			parsedData.emplace(_key, arr);
		}
	}

	void parser::next_token() {
		if (++_curIndex < _tokens.size()) {
			_curToken = _tokens[_curIndex];
		}
	}

	void parser::consume_token(token_category category) {
		if (_curToken.category != category) {
			build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, TOKEN_IMAGE[(int)category]);
		}
		next_token();
	}

}
