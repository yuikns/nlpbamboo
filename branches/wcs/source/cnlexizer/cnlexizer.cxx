#include <ctype.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <cwchar>
#include <locale>

#include "cnlexizer.hxx"
#include "ascii_processor.hxx"
#include "maxforward_processor.hxx"
#include "unigram_processor.hxx"
#include "crf_processor.hxx"
#include "crf2_processor.hxx"
#include "combine_processor.hxx"
#include "single_combine_processor.hxx"

const char CNLexizer::_stream_name_prefix[] = "streamline_";

CNLexizer::CNLexizer(const char *file)
:_config(NULL), _in(&_lex_token[0]), _out(&_lex_token[1])
{
	const char *s;
	char *text, *token, delim[] = ",";
	size_t length;
	std::string str;

	_config = ConfigFactory::create("lua_config", file);
	_config->get_value("streamline", s);
	length = strlen(s) + strlen(_stream_name_prefix) + 1;
	text = new char[length];
	snprintf(text, length, "%s%s", _stream_name_prefix, s);
	_config->get_value(text, s);
	delete []text;
	text = new char[strlen(s) + 1];
	strcpy(text, s);
	for (token = strtok(text, delim); token; token = strtok(NULL, delim)) {
		str = token;
		_streamline.push_back(std::string(token));
		if (str == "ascii")
			_processors["ascii"] = new AsciiProcessor(_config);
		else if (str == "maxforward")
			_processors["maxforward"] = new MaxforwardProcessor(_config);
		else if (str == "unigram")
			_processors["unigram"] = new UnigramProcessor(_config);
		else if (str == "crf")
			_processors["crf"] = new CRFProcessor(_config);
		else if (str == "crf2")
			_processors["crf2"] = new CRF2Processor(_config);
		else if (str == "combine")
			_processors["combine"] = new CombineProcessor(_config);
		else if (str == "single_combine")
			_processors["single_combine"] = new SingleCombineProcessor(_config);
	}
	delete []text;

	std::locale::global(std::locale("zh_CN.UTF-8"));
}

size_t CNLexizer::process(char *t, const char *s)
{
	size_t length;
	wchar_t *wcs;

	length = mbstowcs(NULL, s, 0) + 1;
	wcs = new wchar_t[length];
	mbstowcs(wcs, s, length);
	_process(wcs);
	length = wcstombs(NULL, _result.c_str(), 0) + 1;
	wcstombs(t, _result.c_str(), length);
	delete []wcs;
}

void CNLexizer::_process(const wchar_t *s)
{
	size_t i, j, length;
	LexToken *token;

	_result.clear();
	if (*s == L'\0') return;
	_in->push_back(new LexToken(s));
	length = _streamline.size();
	for (i = 0; i < length; i++) {
		if (_processors[_streamline[i]]) {
			_out->clear();
			_processors[_streamline[i]]->process(*_in, *_out);
			/* switch in & out queue */
			_swap = _out;
			_out = _in;
			_in = _swap;
//			_dump_process_queue();
		} else {
			std::cerr << "Invalid module name: " << _streamline[i] << std::endl;
		}
	}

	length = _in->size();
	for (i = 0; i < length; i++) {
		_result.append((*_in)[i]->get_token());
		_result.append(L" ");
		delete (*_in)[i];
	}
	_in->clear();
}

