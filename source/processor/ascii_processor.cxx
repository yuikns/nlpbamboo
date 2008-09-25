#include <ctype.h>
#include <iostream>

#include "lex_token.hxx"
#include "ascii_processor.hxx"

AsciiProcessor::AsciiProcessor(IConfig *config)
{
	const char *s;
/*
	config->get_value("chinese_number_end", s);
	_load_lexicon(_chinese_number_end, s);
	config->get_value("chinese_punctuation", s);
	_load_lexicon(_chinese_punctuation, s);
	config->get_value("chinese_number", s);
	_load_lexicon(_chinese_number, s);
*/	
}

void AsciiProcessor::_process(LexToken *token, std::vector<LexToken *> &out)
{
	const wchar_t *s = token->get_token();
	wchar_t *stack, *top;
	size_t len;
	enum {
		state_unknow = 0,
		state_alpha,
		state_number,
		state_punctuation,
		state_end
	} state, last;

	if (token->get_length() == 0) return;
	stack = new wchar_t [strlen(s) + 1];
	*stack = L'\0';
	top = stack;
	state = last = state_unknow;
	while (true) {
		if (isalpha(*s)) state = state_alpha;
		else if (isdigit(*s)) state = state_number;
		else if (*s == '.' && last == state_number) state = state_number;
//		else if (_chinese_punctuation.search(wch) > 0 || ispunct(*wch)) state = state_punctuation;
//		else if (_chinese_number.search(wch) > 0) state = state_number;
//		else if (_chinese_number_end.search(wch) > 0 && last == state_number) state = state_number;
		else if (*s == '\0') state = state_end;
		else state = state_unknow;
		if (last != state && *stack) {
			LexToken::attr_t attr = LexToken::attr_unknow;
			if (last == state_alpha) attr = LexToken::attr_alpha;
			else if (last == state_number) attr = LexToken::attr_number;
			*top = '\0';
			out.push_back(new LexToken(stack, attr));
			top = stack;
			*stack = '\0';
			if (state == state_end) break;
		}
		*(top++) = *(s++);
		last = state;
	}
	delete []stack;
}
