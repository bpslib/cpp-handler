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
		T_KEY = 1,
		T_NULL = 2,
		T_STRING = 3,
		T_CHAR = 4,
		T_INTEGER = 5,
		T_FLOAT = 6,
		T_BOOL = 7,
		T_OPEN_ARRAY = 8,
		T_CLOSE_ARRAY = 9,
		T_END_OF_DATA = 10,
		T_DATA_SEP = 11,
		T_ARRAY_SEP = 12
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

	const char skip[] = { SPACE, TAB, NEWLINE, RETURN };

	bool is_skip(char);

	std::string build_lexer_error_message(std::string, int, int);

	static class lexer {
	private:
		static std::vector<token> _tokens;

		// control vars
		static std::string _input;
		static char _curr_char;
		static int _curr_index;
		static int _curr_line;
		static int _curr_collumn;

		static void init();

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
		static std::map<std::string, std::any> _parsed_data;

		// control vars
		static std::vector<token> _tokens;
		static token _curr_token;
		static int _curr_index;

		static std::string _key;
		static std::any _value;
		static std::stack<std::vector<std::any>*> _arr_stack;

		static const int CONTEXT_KEY;
		static const int CONTEXT_ARRAY;
		static int _context;

		static void init();

	public:
		static std::map<std::string, std::any> parse(std::string);

	private:
		static void start();

		static void statement();

		static void key();
		static void value();

		static void tarray();
		static void array_selector();

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

	class plain {
	private:
		static std::stringstream _plain_string_builder;
		static void init();

	public:
		static std::string parse(std::map<std::string, std::any>);

	private:
		static void parse_value(std::any);
		static void parse_array(std::vector<std::any>);
	};

}
