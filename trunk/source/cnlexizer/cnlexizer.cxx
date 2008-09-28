#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <dlfcn.h>

#include "cnlexizer.hxx"

CNLexizer::CNLexizer(const char *file)
:_config(NULL), _in(&_token_fifo[0]), _out(&_token_fifo[1])
{
	std::vector<std::string>::iterator it;
	std::string module;

	_lazy_create_config(file);
	_config->get_value("process_chain", _process_chain);
	_config->get_value("processor_root", processor_root);

	for (it = _process_chain.begin(); it != _process_chain.end(); it++) {
		_create_processor_t create = NULL;
		void *handle = NULL;
		Processor *processor = NULL;

		module.clear();
		module.append(processor_root).append("/").append(*it).append(".so");
		std::cerr << "loading processor " << module << std::endl;
		if (!(handle = dlopen(module.c_str(), RTLD_NOW)))
			throw std::runtime_error(std::string(dlerror()));
		if (!(create = (_create_processor_t)dlsym(handle, "create_processor")))
			throw std::runtime_error(std::string(dlerror()));
		if (!(processor = create(_config)))
			throw std::runtime_error(std::string("can not create processor: ") + *it);

		_processors.push_back(processor);
		_dl_handles.push_back(handle);
	}
}

CNLexizer::~CNLexizer()
{
	size_t i;

	i = _processors.size();
	while(i--) delete _processors[i];

	i = _dl_handles.size();
	while(i--) dlclose(_dl_handles[i]);

	delete _config;
}

void CNLexizer::_lazy_create_config(const char *custom)
{
	int i;
	bool flag = false;
	struct stat buf;
	std::vector<std::string> v;

	v.push_back(custom);
	v.push_back("cnlexizer.cfg");
	v.push_back("etc/cnlexizer.cfg");
	v.push_back("/opt/cnlexizer.cfg");
	v.push_back("/etc/cnlexizer.cfg");
	for (i = 0; i < v.size(); i++) {
		std::cerr << "loading configuration " << v[i] << " ... ";
		if (stat(v[i].c_str(), &buf) == 0) {
			std::cerr << "found." << std::endl;
			_config = ConfigFactory::create(v[i].c_str());
			flag = true;
			break;
		} else {
			std::cerr << "not found." << std::endl;
		}
	}

	if (!flag)
		throw new std::runtime_error("can not find configuration");
}

size_t CNLexizer::process(char *t, const char *s)
{
	char *neo, *p = t;
	size_t i, length;

	*t = '\0';
	length = utf8::length(s);
	_in->clear();
	if (length > _in->capacity()) {
		_in->reserve(length);
		_out->reserve(length);
	}
	_in->push_back(new LexToken(s));
	length = _processors.size();
	for (i = 0; i < length; i++) {
		_out->clear();
		_processors[i]->process(*_in, *_out);
		/* switch in & out queue */
		_swap = _out;
		_out = _in;
		_in = _swap;
	}

	length = _in->size();
	for (i = 0; i < length; i++) {
		strcpy(p, (*_in)[i]->get_orig_token());
		p += (*_in)[i]->get_orig_bytes();
		*(p++) = ' ';
		*p = '\0';
		delete (*_in)[i];
	}
	return p - t;
}