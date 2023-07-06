#include "pch.h"


const static std::string TOKEN_IMAGE[14] = {
	"EOF",
	"key",
	"null constant",
	"string constant",
	"char constant",
	"integer constant",
	"float constant",
	"double constant",
	"bool constant",
	"[",
	"]",
	";",
	":",
	","
};

#undef EOF
#undef NULL

enum token_category {
	EOF = 0,
	KEY = 1,
	NULL = 2,
	STRING = 3,
	CHAR = 4,
	INTEGER = 5,
	FLOAT = 6,
	DOUBLE = 7,
	BOOL = 8,
	OPEN_ARRAY = 9,
	CLOSE_ARRAY = 10,
	END_OF_DATA = 11,
	DATA_SEP = 12,
	ARRAY_SEP = 13
};

class token {
public:
	token_category category;
	std::string image;
	int line;
	int collumn;

	token() = default;

	token(const token_category& category, const std::string& image, int line, int collumn)
		: category(category), image(image), line(line), collumn(collumn) {
	}

	bool operator==(const token& other) const
	{
		return category == other.category && image == other.image && line == other.line && collumn == other.collumn;
	}
};

enum symbols {
	HASH = '#',
	LEFT_BRACKETS = '[',
	RIGHT_BRACKETS = ']',
	DOT = '.',
	COMMA = ',',
	COLON = ':',
	SEMICOLON = ';',
	MINUS = '-',
	DQUOTE = '"',
	UNDERSCORE = '_',
	SPACE = ' ',
	QUOTE = '\'',
	NEWLINE = '\n',
	RETURN = '\r',
	TAB = '\t'
};

const char skip[] = { SPACE, TAB, NEWLINE, RETURN };

bool is_skip(char c)
{
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

static class lexer {
private:
	static std::vector<token> tokens;

	// control vars
	static std::string _input;
	static char _curChar;
	static int _curIndex;
	static int _curLine;
	static int _curCollumn;

public:
	lexer() {
		tokens = std::vector<token>();
		_input = std::string();
		_curIndex = 0;
		_curLine = 1;
		_curCollumn = 1;
	}

	static std::vector<token> parse(std::string input) {
		_input = input;

		NextChar();
		while (!end_of_input()) {
			// to skip the skip chars
			if (is_skip(_curChar)) {
				if (_curChar == symbols::NEWLINE) {
					next_line();
				}
				NextChar();
				continue;
			}

			// to skip comments
			if (_curChar == HASH) {
				while (!end_of_input() and _curChar != symbols::NEWLINE) {
					NextChar();
				}
				next_line();
				NextChar();
			} else if (std::isalpha(_curChar) or _curChar == symbols::UNDERSCORE) { // key, boolean or null
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				NextChar();

				// loops the key
				while (!end_of_input() and _curChar != symbols::COLON and (_curChar == symbols::UNDERSCORE or std::isalnum(_curChar))) {
					lexeme += _curChar;
					NextChar();
				}

				// true or false
				if (lexeme == "true" or lexeme == "false") {
					tokens.push_back(token(token_category::BOOL, lexeme, _curLine, initCol));
				} else if (lexeme == "null") { // null 
					tokens.push_back(token(token_category::NULL, lexeme, _curLine, initCol));
				} else { // key
					tokens.push_back(token(token_category::KEY, lexeme, _curLine, initCol));
				}
			}
			// open array
			else if (_curChar == symbols::LEFT_BRACKETS) {
				tokens.push_back(token(token_category::OPEN_ARRAY, std::string(&_curChar), _curLine, _curCollumn));
				NextChar();
			} else if (_curChar == symbols::RIGHT_BRACKETS) { // close array
				tokens.push_back(token(token_category::CLOSE_ARRAY, std::string(&_curChar), _curLine, _curCollumn));
				NextChar();
			} else if (_curChar == symbols::SEMICOLON) { // end of data
				tokens.push_back(token(token_category::END_OF_DATA, std::string(&_curChar), _curLine, _curCollumn));
				NextChar();
			} else if (_curChar == symbols::COMMA) { // array sep
				tokens.push_back(token(token_category::ARRAY_SEP, std::string(&_curChar), _curLine, _curCollumn));
				NextChar();
			} else if (_curChar == symbols::COLON) { // data sep
				tokens.push_back(token(token_category::DATA_SEP, std::string(&_curChar), _curLine, _curCollumn));
				NextChar();
			} else if (_curChar == symbols::DQUOTE) { // string
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				auto beforeChar = _curChar;
				NextChar();
				while (!end_of_input() and (_curChar != symbols::DQUOTE or beforeChar == '\\')) {
					beforeChar = _curChar;
					lexeme += _curChar;
					NextChar();
				}
				if (_curChar != symbols::DQUOTE) {
					throw std::invalid_argument(build_lexer_error_message("String was not closed", _curLine, _curCollumn));
				}
				lexeme += _curChar;
				tokens.push_back(token(token_category::STRING, lexeme, _curLine, initCol));
				NextChar();
			} else if (_curChar == symbols::QUOTE) { // char
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				NextChar();
				if (_curChar == '\\') {
					lexeme += _curChar;
					NextChar();
					if (_curChar != symbols::QUOTE) {
						std::stringstream msg;
						msg << "Unexpected character '";
						msg << _curChar;
						msg << "' encountered";
						throw std::invalid_argument(build_lexer_error_message(msg.str(), _curLine, _curCollumn));
					}
				}
				lexeme += _curChar;
				NextChar();
				if (_curChar != symbols::QUOTE) {
					throw std::invalid_argument(build_lexer_error_message("Char was not closed", _curLine, _curCollumn));
				}
				lexeme += _curChar;
				tokens.push_back(token(token_category::CHAR, lexeme, _curLine, initCol));
				NextChar();
			} else if (std::isdigit(_curChar) or _curChar == symbols::DOT or _curChar == symbols::MINUS) { // numeric
				auto lexeme = std::string(&_curChar);
				auto initCol = _curCollumn;
				auto dotted = _curChar == symbols::DOT;
				NextChar();
				while (!end_of_input() and (std::isdigit(_curChar) or _curChar == symbols::DOT)) {
					if (_curChar == symbols::DOT) {
						if (dotted) {
							throw std::invalid_argument(build_lexer_error_message("Double dot encountered", _curLine, _curCollumn));
						} else {
							dotted = true;
						}
					}
					lexeme += _curChar;
					NextChar();
				}
				if (std::tolower(_curChar) == 'f' or std::tolower(_curChar) == 'd') {
					lexeme += _curChar;
					NextChar();
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
			}
			else
			{
				std::stringstream msg;
				msg << "Invalid character '";
				msg << _curChar;
				msg << "' encountered";
				throw std::invalid_argument(build_lexer_error_message(msg.str(), _curLine, _curCollumn));
			}
		}

		tokens.push_back(token(token_category::EOF, std::string(), -1, -1));

		return tokens;
	}

	static bool end_of_input() {
		return _curIndex > _input.length();
	}

	static void next_line()
	{
		++_curLine;
		_curCollumn = 0;
	}

	static void NextChar()
	{
		if (_curIndex < _input.length())
		{
			_curChar = _input[_curIndex];
			++_curCollumn;
		}
		++_curIndex;
	}
};


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


class parser {
private:
	static std::map<std::string, void*> parsedData;

	// control vars
	static std::vector<token> _tokens;
	static token _curToken;
	static int _curIndex;

	static std::string _key;
	static void* _value;
	static std::stack<std::list<void*>> _arrStack;

	static const int CONTEXT_KEY = 0;
	static const int CONTEXT_ARRAY = 1;
	static int _context;

	parser() = default;

public:
	parser() {
		parsedData = std::map<std::string, void*>();
		_curIndex = -1;
		_arrStack = std::stack<std::list<void*>>();
		_context = CONTEXT_KEY;
	}

	static std::map<std::string, void*> parse(std::string data)
	{
		_tokens = lexer::parse(data);
		Start();
		return parsedData;
	}

private:
	static void Start()
	{
		NextToken();
		Statement();
		ConsumeToken(token_category::EOF);
	}

	static void Statement()
	{
		switch (_curToken.category)
		{
		case token_category::KEY:
			Key();
			break;
		default:
			break;
		}
	}

	static void Key()
	{
		_key = _curToken.image;
		NextToken();
		ConsumeToken(token_category::DATA_SEP);
		Value();
		ConsumeToken(token_category::END_OF_DATA);
		Statement();
	}

	static void Value()
	{
		switch (_curToken.category)
		{
		case token_category::OPEN_ARRAY:
			OpenArray();
			NextToken();
			Array();
			break;
		case token_category::STRING:
			String();
			break;
		case token_category::CHAR:
			Char();
			break;
		case token_category::INTEGER:
			Integer();
			break;
		case token_category::FLOAT:
			Float();
			break;
		case token_category::DOUBLE:
			Double();
			ArraySel();
			break;
		case token_category::BOOL:
			Bool();
			break;
		case token_category::NULL:
			Null();
			break;
		default:
			build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, "a value or array");
		}
	}

	static void Array()
	{
		switch (_curToken.category)
		{
		case token_category::OPEN_ARRAY:
			OpenArray();
			NextToken();
			Array();
			break;
		case token_category::STRING:
			String();
			ArraySel();
			break;
		case token_category::CHAR:
			Char();
			ArraySel();
			break;
		case token_category::INTEGER:
			Integer();
			ArraySel();
			break;
		case token_category::FLOAT:
			Float();
			ArraySel();
			break;
		case token_category::DOUBLE:
			Double();
			ArraySel();
			break;
		case token_category::BOOL:
			Bool();
			ArraySel();
			break;
		case token_category::NULL:
			Null();
			ArraySel();
			break;
		default:
			build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, "a value or array");
		}
	}

	static void ArraySel()
	{
		switch (_curToken.category)
		{
		case token_category::ARRAY_SEP:
			NextToken();
			Array();
			break;
		case token_category::CLOSE_ARRAY:
			CloseArray();
			NextToken();
			ArraySel();
			break;
		case token_category::END_OF_DATA:
		case token_category::EOF:
			break;
		default:
			build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, "',', ']' or ';'");
		}
	}

	static void String()
	{
		_value = &_curToken.image.substr(1, _curToken.image.length() - 2);
		SetValue();
	}

	static void Char()
	{
		auto val = _curToken.image.substr(1, _curToken.image.length() - 2);
		val = val.replace(val.begin(), val.end(), "\\", "");
		_value = (char*)(val.c_str()[0]);
		SetValue();
	}

	static void Integer()
	{
		_value = (int*)std::stoi(_curToken.image);
		SetValue();
	}

	static void Float()
	{
		auto image = _curToken.image;
		std::transform(image.begin(), image.end(), image.begin(), ::tolower);
		auto strValue = std::equal(image.rbegin(), image.rend(), 'f') ? image.substr(0, image.length() - 1) : image;
		_value = (float*)std::stof(strValue);
		SetValue();
	}

	static void Double()
	{
		auto image = _curToken.image;
		std::transform(image.begin(), image.end(), image.begin(), ::tolower);
		auto strValue = std::equal(image.rbegin(), image.rend(), 'd') ? image.substr(0, image.length() - 1) : image;
		_value = (double*)std::stod(strValue);
		SetValue();
	}

	static void Bool()
	{
		_value = (bool*)(_curToken.image == "true");
		SetValue();
	}

	static void Null()
	{
		_value = nullptr;
		SetValue();
	}

	static void SetValue()
	{
		if (_context == CONTEXT_ARRAY)
		{
			_arrStack.top().push_back(_value);
		}
		else
		{
			parsedData.emplace(_key, _value);
		}
		NextToken();
	}

	static void OpenArray()
	{
		if (_arrStack.size() == 0)
		{
			_context = CONTEXT_ARRAY;
			_arrStack.push(std::list<void*>());
		}
		else
		{
			auto newD = std::list<void*>();
			_arrStack.top().push_back(&newD);
			_arrStack.push(newD);
		}
	}

	static void CloseArray()
	{
		if (_arrStack.size() > 1)
		{
			_arrStack.pop();
		}
		else
		{
			_context = CONTEXT_KEY;
			auto list = _arrStack.top();
			void* arr[list.size()];
			std::copy(list.begin(), list.end(), arr);
			_arrStack.pop();
			parsedData.emplace(_key, arr);
		}
	}

	// parser controls

	static void NextToken()
	{
		if (++_curIndex < _tokens.size())
		{
			_curToken = _tokens[_curIndex];
		}
	}

	static void ConsumeToken(token_category category)
	{
		if (_curToken.category != category)
		{
			build_parser_error_message(_curToken.image, _curToken.line, _curToken.collumn, TOKEN_IMAGE[(int)category]);
		}
		NextToken();
	}
};

#define EOF -1
#define NULL 0
