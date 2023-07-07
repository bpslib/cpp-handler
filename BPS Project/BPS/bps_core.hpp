#pragma once

#include "pch.h"


namespace bps_core {

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

	enum token_category {
		T_EOF = 0,
		KEY = 1,
		T_NULL = 2,
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

		token(const token_category&, const std::string&, int, int);

		bool operator==(const token&) const;
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

	bool is_skip(char);

	std::string build_lexer_error_message(std::string, int, int);

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
		static std::vector<token> tokenize(std::string);

	private:
		static bool end_of_input();
		static void next_line();
		static void next_char();
	};


	std::string build_parser_error_message(std::string, int, int, std::string);


	class parser {
	private:
		static std::map<std::string, std::any> parsedData;

		// control vars
		static std::vector<token> _tokens;
		static token _curToken;
		static int _curIndex;

		static std::string _key;
		static std::any _value;
		static std::stack<std::vector<std::any>> _arrStack;

		static const int CONTEXT_KEY;
		static const int CONTEXT_ARRAY;
		static int _context;

	public:
		static std::map<std::string, std::any> parse(std::string);

	private:
		static void start();

		static void statement();

		static void Key();
		static void value();

		static void tarray();
		static void array_sel();

		static void tstring();
		static void tchar();
		static void tinteger();
		static void tfloat();
		static void tdouble();
		static void tbool();
		static void tnull();

		static void set_value();

		static void open_array();
		static void close_array();

		// parser controls

		static void next_token();
		static void consume_token(token_category);
	};

}
