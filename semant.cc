#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <cassert>

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
};



/**
 * Type loading
 */

Symbol class__class::get_name() {
  return name;
}

Symbol class__class::get_parent() {
  return parent;
}

Symbol method_class::get_return_type() {
  return return_type;
}

Symbol attr_class::get_name() {
  return name;
}


Symbol method_class::get_name() {
  return name;
}

Formals method_class::get_formals() {
  return formals;
}


/*
bool class__class::is_subtype_of(ClassTable ct, Symbol supertype) {
  if (!ct->table->probe(supertype)) {
    error_stream << "lookup of invalid class" << end;
    return false;
  } else {
    if (name == supertype) {
      return true;
    } else if (parent != No_class) {
      return ct->table->lookup(parent)->is_subtype_of(supertype);
    } else {
      return false;
    }
  }
}
*/

/**
 * End type loading
 */
ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    /* Fill this in */
    table = new SymbolTable<Symbol, Class_>();
    table->enterscope();
    install_basic_classes();

    for (int i = 0; i < classes->len(); i++) {
      Class_ cl = classes->nth(i);
      table->addid(cl->get_name(), &cl);
      table2.insert({cl->get_name(), cl});
      InheritanceTree *parent = tree->find(cl->get_parent());
      //cout << "parent: " << cl->get_parent() << endl;
      parent->add_child(cl->get_name());
    }
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    // what? Why? What? What's "<basic class>"?
    // are we going to implement type_name() etc as cool methods?
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);

  table->addid(Object, &Object_class);
  table2.insert({Object, Object_class});
  table->addid(Str, &Str_class);
  table2.insert({Object, Object_class});
  table->addid(Int, &Int_class);
  table2.insert({Object, Object_class});
  table->addid(Bool, &Bool_class);
  table2.insert({Object, Object_class});
  table->addid(IO, &IO_class);
  table2.insert({Object, Object_class});

  tree = new InheritanceTree(Object);
  tree->add_child(Str);
  tree->add_child(Int);
  tree->add_child(Bool);
  tree->add_child(IO);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

bool ClassTable::is_supertype_of(Symbol t1, Symbol t2, Symbol c) {
  if (t2 == No_type) return true;

  if (t1 == SELF_TYPE && t2 == SELF_TYPE) return true; // cool manual section 4.1 says SELF_TYPEx <= SELF_TYPEx, presumably because both were encountered in the same class environment

  if (t2 == SELF_TYPE) {
    return is_supertype_of(t1, c, c);
  }

  if (t1 == SELF_TYPE) {
    return false; // the only cases where this is true have already been covered above
  }

  InheritanceTree *node1 = tree->find(t1);
  if (node1->find(t2) != NULL) {
    return true;
  } else {
    return false;
  }
}

Symbol ClassTable::lowest_common_ancestor(Symbol a, Symbol b, Symbol c) {
  //citation: lowest common ancestor algorithm refered from https://stackoverflow.com/a/6342546/1412255. Implementation is mine.
  // TODO verify self type logic 
  List<InheritanceTree> *chain_a = tree->ancestor_chain(a == SELF_TYPE? c : a);
  List<InheritanceTree> *chain_b = tree->ancestor_chain(b == SELF_TYPE? c : b);

  List<InheritanceTree> *lst1 = chain_a;
  List<InheritanceTree> *lst2 = chain_b;

  InheritanceTree* lca = lst1->hd(); // will always be object

  while (lst1 != NULL && lst2 != NULL && lst1->hd() == lst2->hd()) {
    lca = lst1->hd();
    lst1 = lst1->tl();
    lst2 = lst2->tl();
  }
  // as soon as they're unequal, we've divereged from the lowest common ancestor and we return the one we have

  return lca->get_symbol();
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
ClassTable *classtable;
SymbolTable<Symbol, TypeEnvironment> *typedeclarations = new SymbolTable<Symbol, TypeEnvironment>();

void program_class::semant()
{
    typedeclarations->enterscope(); // INIT
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);

    //classtable->tree->levels(0);

    // set up attribute and method type environments
    for (int i = 0; i < classes->len(); i++) {
      Class_ cl = classes->nth(i);
      typedeclarations->addid(cl->get_name(), new TypeEnvironment());

      //cout << "Looking up ancestors of: " << cl->get_name() << endl;
      List<InheritanceTree> *ancestors = classtable->tree->ancestor_chain(cl->get_name());
      List<InheritanceTree> *lst = ancestors;
      //cout << "Got: " << lst << endl;

      while (lst != NULL) {
        // inherited attributes can't be redefined (section 5, cool manual)
        Symbol ancestor_name = lst->hd()->get_symbol();
        Class_ ancestor_class = classtable->lookup_class (ancestor_name);

        //cout << "One ancestor " << ancestor_name << endl;
        ancestor_class->load_type_info(cl->get_name());
        lst = lst->tl();
      }
    }

    /* some semantic analysis code may go here */
    //for every method, and attribute (with init) in every class, analyze
    for (int i = 0; i < classes->len(); i++) {
      Class_ cl = classes->nth(i);
      cl->semant(typedeclarations->lookup(cl->get_name()));
    }


    if (classtable->errors()) {
      cerr << "Compilation halted due to static semantic errors." << endl;
      exit(1);
    }
}

void class__class::semant(TypeEnvironment *e) {
  //cout << "Now Processing: "<< name << endl;
  for (int i = 0; i < features->len(); i++) {
    Feature f = features->nth(i);
    f->semant(e, name);
  }
}

void attr_class::semant(TypeEnvironment *e, Symbol c) {
  //works for both [Attr-Init] and [Attr-No-Init]
  Symbol inferred = init->infer_type(e, c);
  assert(classtable->is_supertype_of(type_decl, inferred, c));
}

void method_class::semant(TypeEnvironment *e, Symbol c) {
  //[Method]
  e->O->enterscope();
  for (int i = 0; i < formals->len(); i++) {
    Formal f = formals->nth(i);
    f->semant(e, c); // Loads each formal into the environment
    //TODO check that params aren't duplicate
  }
  // now we can evaluate expressions
  Symbol inferred = expr->infer_type(e, c);
  assert(classtable->is_supertype_of(return_type, inferred, c));
  e->O->exitscope();
}

void formal_class::semant(TypeEnvironment *e, Symbol c) {
  e->O->addid(name, &type_decl);
}

void class__class::load_type_info(Symbol cl) {
  //cout << "Class being processed: " << name << endl;
  for (int i = 0; i < features->len(); i++) {
    Feature f = features->nth(i);
    f->load_type_info(cl);
  }
}

void attr_class::load_type_info(Symbol cl) {
  //cout << "Control reaches here" << endl;
  if (typedeclarations->probe(cl)) {
    //cout << "Adding attr: " << name << endl;
    typedeclarations->lookup(cl)->O->addid(name, &type_decl);
  } else {
    cerr << "Not found" << endl;
  }
}

void method_class::load_type_info(Symbol cl) {
  //cout << "Method: " << name <<  endl;
  if (typedeclarations->probe(cl)) {
    typedeclarations->lookup(cl)->M->addid(name, this);
  } else {
    cerr << "Not found" << endl;
  }
}

/*
SymbolTable<Symbol, Symbol>* O(Symbol cl) {
  return attributes;
};
*/

Symbol object_class::infer_type(TypeEnvironment *e, Symbol c) {
  /*
  cout << "Class: " << c << endl;
  e->O->dump();
  cout << "dump end" << endl;
  cout << "Name: " << name << endl;
  */
  Symbol *result = e->O->lookup(name);
  if (result != NULL) {
    //cout << "Lookup: " << *result << endl;
    return *result;
  } else if (name == self) {
    return SELF_TYPE;
  } else {
    cerr << "semantic error: used but not defined" << endl;
    return No_type;
  }
};

// works for empty let declarations
Symbol no_expr_class::infer_type(TypeEnvironment *e, Symbol c) {
  return No_type;
};

Symbol isvoid_class::infer_type(TypeEnvironment *e, Symbol c) {
  return Bool;
};

//TODO according to grammar, new can't be passed params, but check again
Symbol new__class::infer_type(TypeEnvironment *e, Symbol c) {
  return type_name;
};

Symbol string_const_class::infer_type(TypeEnvironment *e, Symbol c) {
  return Str;
};

Symbol bool_const_class::infer_type(TypeEnvironment *e, Symbol c) {
  return Bool;
};

Symbol int_const_class::infer_type(TypeEnvironment *e, Symbol c) {
  return Int;
};

// complement
Symbol comp_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Bool) {
    return Bool;
  } else {
    cerr << "type error comp" << endl;
    return No_type;
  }
};

Symbol leq_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Bool;
  } else {
    cerr << "type error leq" << endl;
    return No_type;
  }
};

// same as leq
Symbol eq_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Bool;
  } else {
    cerr << "type error eq" << endl;
    return No_type;
  }
};

// same as leq
Symbol lt_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Bool;
  } else {
    cerr << "type error lt" << endl;
    return No_type;
  }
};

Symbol neg_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int) {
    return Int;
  } else {
    cerr << "type error neg" << endl;
    return No_type;
  }
};

Symbol divide_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Int;
  } else {
    cerr << "type error divide" << endl;
    return No_type;
  }
};

// same as divide
Symbol mul_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Int;
  } else {
    cerr << "type error mul" << endl;
    return No_type;
  }
};

// same as divide
Symbol sub_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Int;
  } else {
    cerr << "type error sub" << endl;
    return No_type;
  }
};

// same as divide
Symbol plus_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->infer_type(e, c) == Int && e2->infer_type(e, c) == Int) {
    return Int;
  } else {
    cerr << "type error plus" << endl;
    return No_type;
  }
};

Symbol let_class::infer_type(TypeEnvironment *e, Symbol c) {
  Symbol T0dash = type_decl; // SELF_TYPE is accounted for here
  Symbol T1 = init->infer_type(e, c);
  assert(classtable->is_supertype_of(T0dash, T1, c));
  e->O->enterscope();
  e->O->addid(identifier, &T0dash);
  Symbol T2 = body->infer_type(e, c);
  e->O->exitscope();
  return T2;
};

Symbol block_class::infer_type(TypeEnvironment *e, Symbol c) {
  // [Sequence]
  Symbol Tn = No_type;
  for (int i = 0; i < body->len(); i++) {
    Expression exp = body->nth(i);
    Tn = exp->infer_type(e, c); // this call is required because even though we're discarding the returned type, it has the necessary asserts
  }
  return Tn;
};

Symbol typcase_class::infer_type(TypeEnvironment *e, Symbol c) {return No_type;};

Symbol loop_class::infer_type(TypeEnvironment *e, Symbol c) {
  // The manual gives [Loop-False] and [Loop-True] but we can't always determine what the condition will evaluate to at static time, so we take the lub/lowest_common_ancestor of both types
  Symbol pred_type = pred->infer_type(e, c);
  assert(classtable->is_supertype_of(Bool, pred_type, c)); // this isn't specified in the manual explicitly, but implicitly having a false and a true rule means this
  return classtable->lowest_common_ancestor(pred_type, body->infer_type(e, c), c);
};

Symbol cond_class::infer_type(TypeEnvironment *e, Symbol c) {return No_type;};

Symbol dispatch_class::infer_type(TypeEnvironment *e, Symbol c) {return No_type;};

Symbol static_dispatch_class::infer_type(TypeEnvironment *e, Symbol c) {return No_type;};

Symbol assign_class::infer_type(TypeEnvironment *e, Symbol c) {return No_type;};
