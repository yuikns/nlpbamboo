#include <cassert>
#include <iostream>

#include "lexicon_factory.hxx"
#include "single_combine_processor.hxx"

SingleCombineProcessor::SingleCombineProcessor(IConfig *config)
{
	const char *s;

	config->get_value("lexicon_combine", s);
	_lexicon = LexiconFactory::load(s);

	_max_token_length = 2;
	_combine = new wchar_t[_max_token_length + 1]; /* x4 for unicode */
}

SingleCombineProcessor::~SingleCombineProcessor()
{
	delete []_combine;
	delete _lexicon;
}

void SingleCombineProcessor::process(std::vector<LexToken *> &in, std::vector<LexToken *> &out)
{
	size_t i, j, length, num;

	if (in.empty()) return;
	length = in.size();
	for (i = 0; i < length; i++) {
		*_combine = '\0';
		if (in[i]->get_length() == 1) {
			wcscpy(_combine, in[i - 1]->get_token());
			wcscpy(_combine + in[i - 1]->get_length(), in[i]->get_token());
			if (_lexicon->search(_combine) > 0) {
				out.pop_back();
				out.push_back(new LexToken(_combine, LexToken::attr_cword));
				delete in[i - 1];
				delete in[i];
				++i;
				continue;
			}
		}
		out.push_back(in[i]);
	}
}
