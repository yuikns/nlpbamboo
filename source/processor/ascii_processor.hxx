#ifndef ASCII_PROCESSOR_HXX
#define ASCII_PROCESSOR_HXX

#include <cwchar>

#include "processor.hxx"
#include "ilexicon.hxx"
#include "datrie.hxx"

class AsciiProcessor: public Processor {
protected:
	DATrie _chinese_number_end;
	DATrie _chinese_punctuation;
	DATrie _chinese_number;
	AsciiProcessor();
	bool _can_process(LexToken *token)
	{
		return (token->get_attr() == LexToken::attr_unknow);
	}
	void _process(LexToken *token, std::vector<LexToken *> &out);
/*	
	void _load_lexicon(DATrie &trie, const char *s)
	{
		size_t length, i;
		wchar_t *wcs;

		length = mbstowcs(NULL, s, 0);
		wcs = new wchar_t[length + 1];
		mbstowcs(wcs, s, length);

		for (i = 0; i < length; i++) {
			trie.insert(wcs[i], 1);
		}
		delete []wcs;
	}
*/	
public:
	AsciiProcessor(IConfig *config);
	~AsciiProcessor() {};
};

#endif // ASCII_PROCESSOR_HXX
