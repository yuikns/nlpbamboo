#include "lexicon_factory.hxx"
#include "crf_processor.hxx"
#include "utf8.hxx"
#include <cassert>
#include <iostream>
#include <cstdio>
#include <stdexcept>

CRFProcessor::CRFProcessor(IConfig *config)
{
	const char *s;

	config->get_value("crf_ending_tags", _ending_tags);
	config->get_value("crf_model", s);
	_tagger = CRFPP::createTagger(s);
	_token = new char[8];
}

CRFProcessor::~CRFProcessor()
{
	delete []_token;
	delete _tagger;
}

void CRFProcessor::_process(LexToken *token, std::vector<LexToken *> &out)
{
	size_t length, i, j;
	wchar_t *s;
	enum { max_buf_len=16 };
	wchar_t buf[max_buf_len];


	_tagger->clear();
	s = token->get_token();
	length = token->get_length();
	for (i = 0; i < length; i++) {
		if(wctomb(_token, s[i])==-1)
			continue;
		_tagger->add(_token);
	}
	
	if (!_tagger->parse()) throw std::runtime_error("crf parse failed!");

	_result.clear();
	for (i = 0; i < _tagger->size(); ++i) {
		_result.append(_tagger->x(i, 0));
		if (strstr(_ending_tags, _tagger->y2(i))) {
			mbstowcs(buf, _result.c_str(), max_buf_len-1);
			out.push_back(new LexToken(buf, LexToken::attr_cword));
			_result.clear();
		}
	}

}
