%module client
%{
#include "client.hh"
#define SWIG_PYTHON_STRICT_BYTE_CHAR
using namespace stashcache;
%}

%typemap(out) std::optional< std::string > {
  if ( $1.has_value() )
  {
    std::optional< std::string >& tmp_ov = $1;
    $result = PyBytes_FromStringAndSize(tmp_ov.value().c_str(), tmp_ov.value().size());
  }
  else
  {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
};

%typemap(in) std::string {
  $1 = PyBytes_AsString($input);
};

%include "typemaps.i"

%apply (char *STRING, size_t LENGTH) { (const char* key, const size_t key_size) }
%apply (char *STRING, size_t LENGTH) { (const char* value, const size_t value_size) }

class Client {
    public:
    Client(const char *);
    bool set(const char * key, const size_t key_size, const char * value, const size_t value_size);
    std::optional<std::string> get(const char * key, const size_t key_size);
    void terminate(void);
};
