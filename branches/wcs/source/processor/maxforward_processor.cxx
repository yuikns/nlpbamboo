#include "lexicon_factory.hxx"
#include "maxforward_processor.hxx"
#include "utf8.hxx"
#include <cassert>
#include <iostream>

MaxforwardProcessor::MaxforwardProcessor(IConfig *config)
{
	const char *s;

	config->get_value("lexicon_unigram", s);
	config->get_value("max_token_length", _max_token_length);
	_lexicon = LexiconFactory::load(s);

	_token = new wchar_t[_max_token_length + 1]; /* x4 for unicode */
}

MaxforwardProcessor::~MaxforwardProcessor()
{
	delete []_token;
	delete _lexicon;
}

void MaxforwardProcessor::_process(LexToken *token, std::vector<LexToken *> &out)
{
	size_t length, max_token_length, i, j;
	wchar_t *s;

	s = token->get_token();
	length = token->get_length();
	for (i = 0; i < length; i++) {
		max_token_length = (_max_token_length < length - i)?_max_token_length:length - i;
		for (j = max_token_length; j > 0; j--) {
			wcsncpy(_token, s + i, j);
			if (_lexicon->search(_token) > 0) break;
		}
		if (j == 0) {j = 1;}
		out.push_back(new LexToken(_token, LexToken::attr_cword));
		i = i + j - 1;
	}
}
