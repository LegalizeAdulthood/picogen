#ifndef TO_CALLABLE_HH_INCLUDED_20120322
#define TO_CALLABLE_HH_INCLUDED_20120322

#include <string>
#include "phase3/Program.h"
#include "template/Instantiation.h"

namespace quatsch { namespace compiler { namespace phase5 {
        using extern_template::extern_function;
        extern_function to_callable (phase3::Program const &);
} } }

#endif // TO_CALLABLE_HH_INCLUDED_20120322