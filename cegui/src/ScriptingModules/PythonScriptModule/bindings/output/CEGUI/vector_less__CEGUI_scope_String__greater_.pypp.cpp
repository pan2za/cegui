// This file has been generated by Py++.

#include "boost/python.hpp"
#include "indexing_suite/container_suite.hpp"
#include "indexing_suite/vector.hpp"
#include "python_CEGUI.h"
#include "vector_less__CEGUI_scope_String__greater_.pypp.hpp"

namespace bp = boost::python;

void register_vector_less__CEGUI_scope_String__greater__class(){

    { //::std::vector< CEGUI::String >
        typedef bp::class_< std::vector< CEGUI::String > > vector_less__CEGUI_scope_String__greater__exposer_t;
        vector_less__CEGUI_scope_String__greater__exposer_t vector_less__CEGUI_scope_String__greater__exposer = vector_less__CEGUI_scope_String__greater__exposer_t( "vector_less__CEGUI_scope_String__greater_" );
        bp::scope vector_less__CEGUI_scope_String__greater__scope( vector_less__CEGUI_scope_String__greater__exposer );
        vector_less__CEGUI_scope_String__greater__exposer.def( bp::indexing::vector_suite< std::vector< CEGUI::String > >() );
    }

}
