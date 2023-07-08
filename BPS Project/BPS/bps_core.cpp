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

	std::vector<token> lexer::_tokens = std::vector<token>();
	std::string lexer::_input = std::string();
	char lexer::_curr_char;
	int lexer::_curr_index = 0;
	int lexer::_curr_line = 1;
	int lexer::_curr_collumn = 1;

	std::vector<token> lexer::tokenize(std::string input) {
		_input = input;

		next_char();
		while (!end_of_input()) {
			// to skip the skip chars
			if (is_skip(_curr_char)) {
				if (_curr_char == symbols::NEWLINE) {
					next_line();
				}
				next_char();
				continue;
			}

			// to skip comments
			if (_curr_char == HASH) {
				while (!end_of_input() and _curr_char != symbols::NEWLINE) {
					next_char();
				}
				next_line();
				next_char();
			} else if (std::isalpha(_curr_char) or _curr_char == symbols::UNDERSCORE) { // key, boolean or null
				auto lexeme = std::string(&_curr_char);
				auto initCol = _curr_collumn;
				next_char();

				// loops the key
				while (!end_of_input() and _curr_char != symbols::COLON and (_curr_char == symbols::UNDERSCORE or std::isalnum(_curr_char))) {
					lexeme += _curr_char;
					next_char();
				}

				// true or false
				if (lexeme == "true" or lexeme == "false") {
					_tokens.push_back(token(token_category::BOOL, lexeme, _curr_line, initCol));
				} else if (lexeme == "null") { // null 
					_tokens.push_back(token(token_category::T_NULL, lexeme, _curr_line, initCol));
				} else { // key
					_tokens.push_back(token(token_category::KEY, lexeme, _curr_line, initCol));
				}
			}
			// open array
			else if (_curr_char == symbols::LEFT_BRACKETS) {
				_tokens.push_back(token(token_category::OPEN_ARRAY, std::string(&_curr_char), _curr_line, _curr_collumn));
				next_char();
			} else if (_curr_char == symbols::RIGHT_BRACKETS) { // close array
				_tokens.push_back(token(token_category::CLOSE_ARRAY, std::string(&_curr_char), _curr_line, _curr_collumn));
				next_char();
			} else if (_curr_char == symbols::SEMICOLON) { // end of data
				_tokens.push_back(token(token_category::END_OF_DATA, std::string(&_curr_char), _curr_line, _curr_collumn));
				next_char();
			} else if (_curr_char == symbols::COMMA) { // array sep
				_tokens.push_back(token(token_category::ARRAY_SEP, std::string(&_curr_char), _curr_line, _curr_collumn));
				next_char();
			} else if (_curr_char == symbols::COLON) { // data sep
				_tokens.push_back(token(token_category::DATA_SEP, std::string(&_curr_char), _curr_line, _curr_collumn));
				next_char();
			} else if (_curr_char == symbols::DQUOTE) { // string
				auto lexeme = std::string(&_curr_char);
				auto initCol = _curr_collumn;
				auto beforeChar = _curr_char;
				next_char();
				while (!end_of_input() and (_curr_char != symbols::DQUOTE or beforeChar == '\\')) {
					beforeChar = _curr_char;
					lexeme += _curr_char;
					next_char();
				}
				if (_curr_char != symbols::DQUOTE) {
					throw std::invalid_argument(build_lexer_error_message("String was not closed", _curr_line, _curr_collumn));
				}
				lexeme += _curr_char;
				_tokens.push_back(token(token_category::STRING, lexeme, _curr_line, initCol));
				next_char();
			} else if (_curr_char == symbols::QUOTE) { // char
				auto lexeme = std::string(&_curr_char);
				auto initCol = _curr_collumn;
				next_char();
				if (_curr_char == '\\') {
					lexeme += _curr_char;
					next_char();
				}
				lexeme += _curr_char;
				next_char();
				if (_curr_char != symbols::QUOTE) {
					throw std::invalid_argument(build_lexer_error_message("Char was not closed", _curr_line, _curr_collumn));
				}
				lexeme += _curr_char;
				_tokens.push_back(token(token_category::CHAR, lexeme, _curr_line, initCol));
				next_char();
			} else if (std::isdigit(_curr_char) or _curr_char == symbols::DOT or _curr_char == symbols::MINUS) { // numeric
				auto lexeme = std::string(&_curr_char);
				auto initCol = _curr_collumn;
				auto dotted = _curr_char == symbols::DOT;
				next_char();
				while (!end_of_input() and (std::isdigit(_curr_char) or _curr_char == symbols::DOT)) {
					if (_curr_char == symbols::DOT) {
						if (dotted) {
							throw std::invalid_argument(build_lexer_error_message("Double dot encountered", _curr_line, _curr_collumn));
						} else {
							dotted = true;
						}
					}
					lexeme += _curr_char;
					next_char();
				}
				if (std::tolower(_curr_char) == 'f' or std::tolower(_curr_char) == 'd') {
					lexeme += _curr_char;
					next_char();
				}
				std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::tolower);
				// float, double or int
				if (lexeme.find('f')) {
					_tokens.push_back(token(token_category::FLOAT, lexeme, _curr_line, initCol));
				} else if (lexeme.find(symbols::DOT) or lexeme.find('d')) {
					_tokens.push_back(token(token_category::DOUBLE, lexeme, _curr_line, initCol));
				} else {
					_tokens.push_back(token(token_category::INTEGER, lexeme, _curr_line, initCol));
				}
			} else {
				std::stringstream msg;
				msg << "Invalid character '";
				msg << _curr_char;
				msg << "' encountered";
				throw std::invalid_argument(build_lexer_error_message(msg.str(), _curr_line, _curr_collumn));
			}
		}

		_tokens.push_back(token(token_category::T_EOF, std::string(), -1, -1));

		return _tokens;
	}

	bool lexer::end_of_input() {
		return _curr_index > _input.length();
	}

	void lexer::next_line() {
		++_curr_line;
		_curr_collumn = 0;
	}

	void lexer::next_char() {
		if (_curr_index < _input.length()) {
			_curr_char = _input[_curr_index];
			++_curr_collumn;
		}
		++_curr_index;
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

	std::map<std::string, std::any> parser::_parsed_data = std::map<std::string, std::any>();
	std::vector<token> parser::_tokens;
	token parser::_curr_token;
	int parser::_curr_index = -1;
	std::string parser::_key;
	std::any parser::_value;
	std::stack<std::vector<std::any>> parser::_arr_stack = std::stack<std::vector<std::any>>();
	const int parser::CONTEXT_KEY = 0;
	const int parser::CONTEXT_ARRAY = 1;
	int parser::_context = CONTEXT_KEY;

	std::map<std::string, std::any> parser::parse(std::string data) {
		_tokens = lexer::tokenize(data);
		start();
		return _parsed_data;
	}

	void parser::start() {
		next_token();
		statement();
		consume_token(token_category::T_EOF);
	}

	void parser::statement() {
		switch (_curr_token.category) {
			case token_category::KEY:
				Key();
				break;
			default:
				break;
		}
	}

	void parser::Key() {
		_key = _curr_token.image;
		next_token();
		consume_token(token_category::DATA_SEP);
		value();
		consume_token(token_category::END_OF_DATA);
		statement();
	}

	void parser::value() {
		switch (_curr_token.category) {
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
				build_parser_error_message(_curr_token.image, _curr_token.line, _curr_token.collumn, "a value or array");
		}
	}

	void parser::tarray() {
		switch (_curr_token.category) {
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
				build_parser_error_message(_curr_token.image, _curr_token.line, _curr_token.collumn, "a value or array");
		}
	}

	void parser::array_sel() {
		switch (_curr_token.category) {
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
				build_parser_error_message(_curr_token.image, _curr_token.line, _curr_token.collumn, "',', ']' or ';'");
		}
	}

	void parser::tstring() {
		std::string strValue = _curr_token.image.substr(1, _curr_token.image.length() - 2);
		_value = strValue;
		set_value();
	}

	void parser::tchar() {
		std::string val = _curr_token.image.substr(1, _curr_token.image.length() - 2);
		val = std::regex_replace(val, std::regex("\\\\"), "");
		char cValue = val[0];
		_value = cValue;
		set_value();
	}

	void parser::tinteger() {
		int intValue = std::stoi(_curr_token.image);
		_value = intValue;
		set_value();
	}

	void parser::tfloat() {
		auto image = _curr_token.image;
		std::transform(image.begin(), image.end(), image.begin(), ::tolower);
		auto strValue = image.length() > 0 and image.back() == 'f' ? image.substr(0, image.length() - 1) : image;
		float floatValue = std::stof(strValue);
		_value = floatValue;
		set_value();
	}

	void parser::tdouble() {
		auto image = _curr_token.image;
		std::transform(image.begin(), image.end(), image.begin(), ::tolower);
		auto strValue = image.length() > 0 and image.back() == 'd' ? image.substr(0, image.length() - 1) : image;
		double doubleValue = std::stod(strValue);
		_value = doubleValue;
		set_value();
	}

	void parser::tbool() {
		bool boolValue = _curr_token.image == "true";
		_value = boolValue;
		set_value();
	}

	void parser::tnull() {
		_value = nullptr;
		set_value();
	}

	void parser::set_value() {
		if (_context == CONTEXT_ARRAY) {
			_arr_stack.top().push_back(_value);
		} else {
			_parsed_data.emplace(_key, _value);
		}
		next_token();
	}

	void parser::open_array() {
		if (_arr_stack.size() == 0) {
			_context = CONTEXT_ARRAY;
			_arr_stack.push(std::vector<std::any>());
		} else {
			auto newD = std::vector<std::any>();
			_arr_stack.top().push_back(&newD);
			_arr_stack.push(newD);
		}
	}

	void parser::close_array() {
		if (_arr_stack.size() > 1) {
			_arr_stack.pop();
		} else {
			_context = CONTEXT_KEY;
			std::vector<std::any> vector = _arr_stack.top();
			size_t vector_size = vector.size();
			std::any* arr = new std::any[vector_size];
			for (int i = 0; i < vector_size; ++i) {
				arr[i] = vector[i];
			}
			_arr_stack.pop();
			_parsed_data.emplace(_key, arr);
		}
	}

	void parser::next_token() {
		if (++_curr_index < _tokens.size()) {
			_curr_token = _tokens[_curr_index];
		}
	}

	void parser::consume_token(token_category category) {
		if (_curr_token.category != category) {
			build_parser_error_message(_curr_token.image, _curr_token.line, _curr_token.collumn, TOKEN_IMAGE[(int)category]);
		}
		next_token();
	}

}
