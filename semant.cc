#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <cassert>
#include <set>
#include <queue>
#include <algorithm> // only to use `find` in set

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

std::set<Symbol> unprocessed; // all unprocessed classes
// everything visited and processed will be accessible via find()

bool process_class(ClassTable *ct, Classes classes, Symbol s) { // process_class is like the dfs_visit method
  std::deque<Symbol> queued_classes; // dfs stack
  queued_classes.push_front(s);
  Symbol c;

  Class_ cl_for_error_reporting = NULL;
  for (int i = 0; i < classes->len(); i++) { 
    cl_for_error_reporting = classes->nth(i);
    if (cl_for_error_reporting->get_name() == s) {
      break;
    }
  }

  while (!queued_classes.empty()) {
    c = queued_classes.front();
    queued_classes.pop_front();

    if (semant_debug) cout << "processing " << c << endl;

    Class_ cl = NULL; // find it, else return error
    for (int i = 0; i < classes->len(); i++) {
      Class_ cla = classes->nth(i);
      if (semant_debug) {
        cout << "looking through cla ";
        cout << cla->get_name() << endl;
      }
      if (cla->get_name() == c) {
        cl = cla;
        break;
      }
    }


    if (cl == NULL) {
      ct->semant_error(cl_for_error_reporting);
      cerr << "Class " << c  << " does not exist"<< endl;
      return false;
    }


    if (unprocessed.find(c) != unprocessed.end()) { // if not already processed
      if (ct->tree->find(cl->get_parent()) == NULL) { // if parent is unprocessed, queue it
        if (find(queued_classes.begin(), queued_classes.end(), cl->get_parent()) != queued_classes.end())  { // if it's already on stack, we have a cyclic dependency
          ct->semant_error(cl_for_error_reporting);
          cerr << "Cyclic dependency found involving class " << cl->get_parent() << endl;
          return false;
        } else {
          queued_classes.push_front(c); // this will get processed again later
          queued_classes.push_front(cl->get_parent()); // parent will get processed first
        }
      } else { // add to inheritance tree
        ct->table2.insert({c, cl});
        InheritanceTree *parent = ct->tree->find(cl->get_parent());
        if (semant_debug) {
          cout << "Adding child " << c << " to parent " << cl->get_parent() << " at " << parent <<  endl;
        }
        parent->add_child(c);
        unprocessed.erase(unprocessed.find(c));
      }
    } // if it's already processed, we get back to next element on stack which inherits it
  }
  return true;
}

ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {
  /* Fill this in */
  table = new SymbolTable<Symbol, Class_>();
  table->enterscope();
  install_basic_classes();

  if (semant_debug) tree->levels(0);

  for (int i = 0; i < classes->len(); i++) {
    Class_ cl = classes->nth(i);
    unprocessed.insert(cl->get_name());
  }

  while (!unprocessed.empty()) {
    Symbol class_name = *unprocessed.begin(); // we don't need to remove, it will be removed after it's processed
    if (!process_class(this, classes, class_name)) {
      // there was some error, it has been reported and all
      break;
    }
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

  //table->addid(Object, &Object_class);
  table2.insert({Object, Object_class});
  //table->addid(Str, &Str_class);
  table2.insert({Str, Str_class});
  //table->addid(Int, &Int_class);
  table2.insert({Int, Int_class});
  //table->addid(Bool, &Bool_class);
  table2.insert({Bool, Bool_class});
  //table->addid(IO, &IO_class);
  table2.insert({IO, IO_class});

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

ostream& ClassTable::semant_error(Symbol classname)
{                                                             
    return semant_error(lookup_class(classname));
}    

ostream& ClassTable::semant_element_error(Symbol classname, tree_node *t)
{
    return semant_error(lookup_class(classname)->get_filename(), t);
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
  a = a == SELF_TYPE? c : a;
  b = b == SELF_TYPE? c : b;

  if (a == No_type) return b;
  if (b == No_type) return a;

  List<InheritanceTree> *chain_a = tree->ancestor_chain(a);
  List<InheritanceTree> *chain_b = tree->ancestor_chain(b);

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

    if (classtable->errors()) { // in case there are errors building up the inheritance tree, end
      cerr << "Compilation halted due to static semantic errors." << endl;
      exit(1);
    }


    if (semant_debug) {
      classtable->tree->levels(0);
      cout << " tree printed completely" << endl;
    }

    classtable->init_attr_meth(classtable->lookup_class(Object));
    classtable->init_attr_meth(classtable->lookup_class(Int));
    classtable->init_attr_meth(classtable->lookup_class(Str));
    classtable->init_attr_meth(classtable->lookup_class(Bool));
    classtable->init_attr_meth(classtable->lookup_class(IO));


    // set up attribute and method type environments
    for (int i = 0; i < classes->len(); i++) {
      Class_ cl = classes->nth(i);
      classtable->init_attr_meth(cl);
    }


    if (classtable->tree->find(Main) == NULL) {
      classtable->semant_error();
      cerr << "Class Main is not defined." << endl;
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

void ClassTable::init_attr_meth(Class_ cl) {
  typedeclarations->addid(cl->get_name(), new TypeEnvironment());

  if (semant_debug) cout << "Looking up ancestors of: " << cl->get_name() << endl;
  List<InheritanceTree> *ancestors = classtable->tree->ancestor_chain(cl->get_name());
  List<InheritanceTree> *lst = ancestors;
  if (semant_debug) cout << "Got: " << lst << endl;

  while (lst != NULL) {
    // inherited attributes can't be redefined (section 5, cool manual)
    Symbol ancestor_name = lst->hd()->get_symbol();
    Class_ ancestor_class = classtable->lookup_class(ancestor_name);

    if (semant_debug) {
      cout << "marco" << endl;
      cout << "One ancestor " << ancestor_name << endl;
      cout << "One ancestor " << ancestor_class << endl;
      cout << "One ancestor " << ancestor_class->get_name() << endl;
      cout << "polo 1" << endl;
    }
    ancestor_class->load_type_info(cl->get_name());
    lst = lst->tl();
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
  Symbol inferred = init->ias_type(e, c);
  classtable->assert_supertype(type_decl, inferred, c); // localized error if init expression doesn't match the declared type
}


void method_class::semant(TypeEnvironment *e, Symbol c) {
  //[Method]
  e->O->enterscope();
  e->O->addid(self, &SELF_TYPE); // so that self is "defined" and doesn't give errors
  std::set<Symbol> param_names;
  for (int i = 0; i < formals->len(); i++) {
    Formal f = formals->nth(i);
    f->semant(e, c); // Loads each formal into the environment
    if (param_names.find(f->get_name()) == param_names.end()) {
      param_names.insert(f->get_name());
    } else {
      classtable->semant_element_error(c, f);
      cerr << "Duplicate parameter name " << f->get_name() << endl;
    }
  }
  // now we can evaluate expressions
  Symbol inferred = expr->ias_type(e, c);
  classtable->assert_supertype(return_type, inferred, c); // error here would be localized
  e->O->exitscope();
}

void formal_class::semant(TypeEnvironment *e, Symbol c) {
  e->O->addid(name, &type_decl);
}

Symbol formal_class::get_name() {
  return name;
}

Symbol formal_class::get_type() {
  return type_decl;
}

void class__class::load_type_info(Symbol cl) {
  //cout << "Class being processed: " << name << endl;
  //cout << "polo 2" << endl;
  std::set<Symbol> attr_names;
  std::set<Symbol> method_names;
  for (int i = 0; i < features->len(); i++) {
    Feature f = features->nth(i);
    // error recovery strategy for duplicate attr / method names: continue semantic checks with the first definition 
    if (f->is_attribute()) {
      if (attr_names.find(f->get_name()) != attr_names.end()) {
        classtable->semant_element_error(cl, f);
        cerr << "Duplicate attribute declaration for " << cl << "::" << f->get_name() << "is not allowed in a class" << endl;
      } else {
        attr_names.insert(f->get_name());
        f->load_type_info(cl);
      }
    } else {
      if (method_names.find(f->get_name()) != method_names.end()) {
        classtable->semant_element_error(cl, f);
        cerr << "Duplicate method declaration for " << cl << "::" << f->get_name() << "is not allowed in a class" << endl;
      } else {
        method_names.insert(f->get_name());
        f->load_type_info(cl);
      }
    } 
  }
}

void attr_class::load_type_info(Symbol cl) {
  if (typedeclarations->probe(cl)) {
    if (typedeclarations->lookup(cl)->O->lookup(name) != NULL) {
      classtable->semant_element_error(cl, this);
      cerr << "Attribute " << cl << "::" << name << " already defined in an ancestor class" << endl;
    } else {
      typedeclarations->lookup(cl)->O->addid(name, &type_decl);
    }
  } else {
    classtable->semant_element_error(cl, this);
    cerr << "Class " <<  cl << " not found" << endl;
  }
}

bool method_class::check_compatibility(method_class *m) {
  //see cool manual section 6
  // number of args
  if (m->formals->len() != formals->len()) {
    return false;
  }
  // return types
  if (m->return_type != return_type) {
    return false;
  }

  // types, in order
  for (int i = 0; i < formals->len(); i++) {
    Formal f1 = formals->nth(i);
    Formal f2 = m->formals->nth(i);
    if (f1->get_type() != f2->get_type()) {
      return false;
    }
  }

  return true;
}

void method_class::load_type_info(Symbol cl) {
  if (typedeclarations->probe(cl)) {
    method_class *m = typedeclarations->lookup(cl)->M->lookup(name);
    if (m != NULL) {
      if (!check_compatibility(m)) {
        classtable->semant_element_error(cl, this);
        cerr << "Illegal method override from an ancestor class in "  << cl << "::" << name  << endl;
      }
    } else {
      typedeclarations->lookup(cl)->M->addid(name, this);
    }
  } else {
    classtable->semant_error(cl);
    cerr << "Method " << name << "Not found" << endl;
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
  if (name == self) {
    return SELF_TYPE;
  } else if (result != NULL) {
    //cout << "Lookup: " << *result << endl;
    return *result;
  } else {
    classtable->semant_error(c);
    cerr << "semantic error: " << name << "used but not defined" << endl;
    return No_type;
  }
};

// works for empty let declarations
Symbol no_expr_class::infer_type(TypeEnvironment *e, Symbol c) {
  return No_type;
};

Symbol isvoid_class::infer_type(TypeEnvironment *e, Symbol c) {
  e1->ias_type(e, c);
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
  if (e1->ias_type(e, c) == Bool) {
    return Bool;
  } else {
    classtable->semant_error(c);
    cerr << "Complement of expression which is not of type boolean" << endl;
    return No_type;
  }
};

Symbol leq_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int && e2->ias_type(e, c) == Int) {
    return Bool;
  } else {
    classtable->semant_error(c);
    cerr << "leq: both types are not integers" << endl;
    return No_type;
  }
};

// same as leq
Symbol eq_class::infer_type(TypeEnvironment *e, Symbol c) {
  e1->ias_type(e, c);
  e2->ias_type(e, c);
  return Bool;
};

// same as leq
Symbol lt_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int && e2->ias_type(e, c) == Int) {
    return Bool;
  } else {
    classtable->semant_error(c);
    cerr << "lt: both types are not integers" << endl;
    return No_type;
  }
};

Symbol neg_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int) {
    return Int;
  } else {
    classtable->semant_error(c);
    cerr << "neg: operand is not an integer" << endl;
    return No_type;
  }
};

Symbol divide_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int && e2->ias_type(e, c) == Int) {
    return Int;
  } else {
    classtable->semant_error(c);
    cerr << "division: both types are not integers" << endl;
    return No_type;
  }
};

// same as divide
Symbol mul_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int && e2->ias_type(e, c) == Int) {
    return Int;
  } else {
    classtable->semant_error(c);
    cerr << "mul: both types are not integers" << endl;
    return No_type;
  }
};

// same as divide
Symbol sub_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int && e2->ias_type(e, c) == Int) {
    return Int;
  } else {
    classtable->semant_error(c);
    cerr << "sub: both types are not integers" << endl;
    return No_type;
  }
};

// same as divide
Symbol plus_class::infer_type(TypeEnvironment *e, Symbol c) {
  if (e1->ias_type(e, c) == Int && e2->ias_type(e, c) == Int) {
    return Int;
  } else {
    classtable->semant_error(c);
    cerr << "sub: both types are not integers" << endl;
    return No_type;
  }
};

Symbol let_class::infer_type(TypeEnvironment *e, Symbol c) {
  Symbol T0dash = type_decl; // SELF_TYPE is accounted for here
  Symbol T1 = init->ias_type(e, c);
  classtable->assert_supertype(T0dash, T1, c); // type error in init expression is localized, doesn't affect other things
  e->O->enterscope();
  e->O->addid(identifier, &T0dash);
  Symbol T2 = body->ias_type(e, c);
  e->O->exitscope();
  return T2;
};

Symbol block_class::infer_type(TypeEnvironment *e, Symbol c) {
  // [Sequence]
  Symbol Tn = No_type;
  for (int i = 0; i < body->len(); i++) {
    Expression exp = body->nth(i);
    Tn = exp->ias_type(e, c); // this call is required because even though we're discarding the returned type, it has the necessary asserts
  }
  //cout << "set it to " << Tn << endl;
  return Tn;
};

Symbol typcase_class::infer_type(TypeEnvironment *e, Symbol c) {
  expr->ias_type(e, c);
  Symbol lubresult = No_type;
  for (int i = 0; i < cases->len(); i++) {
    Case branch = cases->nth(i);
    Symbol s = branch->infer_type(e, c); //note, I guess we don't need to set type here because branch_class is not an expression?
    lubresult = classtable->lowest_common_ancestor(s, lubresult, c);
  }
  return lubresult;
};

Symbol branch_class::infer_type(TypeEnvironment *e, Symbol c) {
  e->O->enterscope();
  e->O->addid(name, &type_decl);
  Symbol t = expr->ias_type(e, c);
  e->O->exitscope();
  return t;
}

Symbol loop_class::infer_type(TypeEnvironment *e, Symbol c) {
  // The manual gives [Loop-False] and [Loop-True] but we can't always determine what the condition will evaluate to at static time, so we take the lub/lowest_common_ancestor of both types
  Symbol pred_type = pred->ias_type(e, c);
  classtable->assert_supertype(Bool, pred_type, c); // this isn't specified in the manual explicitly, but implicitly having a false and a true rule means this. A type error here would be localized, doesn't affect loop body
  return classtable->lowest_common_ancestor(Bool, body->ias_type(e, c), c); // in case pred_type is No_type, we don't want that to affect loop's type's evaluation
};

Symbol cond_class::infer_type(TypeEnvironment *e, Symbol c) {
  //[If-True] and [If-False]
  classtable->assert_supertype(Bool, pred->ias_type(e, c), c); // type error, if any, is localized
  return classtable->lowest_common_ancestor(then_exp->ias_type(e, c), else_exp->ias_type(e, c), c); // takes care of no else
};

Symbol dispatch_class::infer_type(TypeEnvironment *e, Symbol c) {
  Symbol T0 = expr->ias_type(e, c);
  Symbol T0dash = T0;
  if (T0 == SELF_TYPE) {
    T0dash = c;
  }


  TypeEnvironment *edash = typedeclarations->lookup(T0dash);
  //T0dash will always exist because it's either inferred or is c

  /*
  cout << "core dumped yet?" << endl;
  cout << "T0 " << edash << endl;
  cout << "edash " << T0dash << endl;
  */

  method_class *m =  edash->M->lookup(name); // unclean...

  Formals formals = m->get_formals();

  if(formals->len() != actual->len()) {
    classtable->semant_element_error(c, this);
    cerr << "Mismatch between length of specified arguments and number of parameters that the function " << name << " accepts." << endl;
    return No_type; // unsure how to recover from this error, so we won't
  }

  Symbol Tnplus1 = m->get_return_type();

  if (Tnplus1 == SELF_TYPE) {
    Tnplus1 = T0;
  }

  for (int i = 0; i < formals->len(); i++) {
    Formal formal = formals->nth(i);
    Expression a = actual->nth(i);
    classtable->assert_supertype(formal->get_type(), a->ias_type(e, c), c); // type error here is localized and we should be able to continue checking type of other params
  }

  return Tnplus1;
};

Symbol static_dispatch_class::infer_type(TypeEnvironment *e, Symbol c) {

  Symbol T0 = expr->ias_type(e, c);

  TypeEnvironment *edash = typedeclarations->lookup(type_name);

  if (edash == NULL) {
    classtable->semant_element_error(c, this);
    cerr << "The class " << type_name << " does not exist." << endl;
    return No_type; // unsure how to recover from this error
  }

  method_class *m =  edash->M->lookup(name);

  Formals formals = m->get_formals();

  assert(formals->len() == actual->len());

  Symbol Tnplus1 = m->get_return_type();

  if (Tnplus1 == SELF_TYPE) {
    Tnplus1 = T0;
  }

  for (int i = 0; i < formals->len(); i++) {
    Formal formal = formals->nth(i);
    Expression a = actual->nth(i);
    classtable->assert_supertype(formal->get_type(), a->ias_type(e, c), c); // localized error if an arg doesn't match param
  }

  return Tnplus1;
};

Symbol assign_class::infer_type(TypeEnvironment *e, Symbol c) {
  // also do semantic checks here
  if (name == self) {
    classtable->semant_element_error(c, this); // type error is localized, we don't update self in any way, so we can recover and proceed to other errors
    cerr << "Cannot assign to 'self'." << endl;
  }

  //[ASSIGN]
  Symbol Tdash = expr->ias_type(e, c); 
  Symbol *T = e->O->lookup(name); 
  if (T != NULL) {
    if (classtable->assert_supertype(*T, Tdash, c)) {
      return Tdash;
    } else {
      return *T;
    }
  } else {
    classtable->semant_element_error(c, this);
    cerr << "Symbol " << name << " used before it was defined "<< endl;
    return No_type; 
  }
};
