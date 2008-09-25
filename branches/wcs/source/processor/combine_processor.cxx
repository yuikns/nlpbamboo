#include "lexicon_factory.hxx"
#include "combine_processor.hxx"
#include <cassert>
#include <iostream>

CombineProcessor::CombineProcessor(IConfig *config)
{
	const char *s;

	config->get_value("lexicon_combine", s);
	config->get_value("max_token_length", _max_token_length);
	_lexicon = LexiconFactory::load(s);

	_token = new wchar_t[_max_token_length + 1];
	_combine = new wchar_t[_max_token_length + 1];
}

CombineProcessor::~CombineProcessor()
{
	delete []_combine;
	delete []_token;
	delete _lexicon;
}

void CombineProcessor::process(std::vector<LexToken *> &in, std::vector<LexToken *> &out)
{
	size_t i, j, length, num, consume;

	if (in.empty()) return;
	length = in.size();
	for (i = 0; i < length; i++) {
		*_combine = '\0';
		*_token = '\0';
		for (j = i; j < length; j++) {
			consume = wcslen(_combine);
			if (consume + in[j]->get_length() <= _max_token_length) {
				wcscpy(_combine + consume, in[j]->get_token());
				if (_lexicon->search(_combine) > 0) {
					wcscpy(_token, _combine);
					num = j;
				}
			}
		}
		if (*_token) {
			out.push_back(new LexToken(_token, LexToken::attr_cword));
			for (j = i; j <= num; j++)
				delete in[j];
			i = num;
		} else {
			out.push_back(in[i]);
		}
	}
}
