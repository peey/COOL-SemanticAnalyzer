//
// The following include files must come first.

#ifndef COOL_TREE_HANDCODE_H
#define COOL_TREE_HANDCODE_H

#include <iostream>
#include "tree.h"
#include "cool.h"
#include "stringtab.h"
#include "symtab.h"

#define yylineno curr_lineno;
extern int yylineno;

inline Boolean copy_Boolean(Boolean b) {return b; }
inline void assert_Boolean(Boolean) {}
inline void dump_Boolean(ostream& stream, int padding, Boolean b)
	{ stream << pad(padding) << (int) b << "\n"; }

void dump_Symbol(ostream& stream, int padding, Symbol b);
void assert_Symbol(Symbol b);
Symbol copy_Symbol(Symbol b);

class Program_class;
typedef Program_class *Program;
class Class__class;
typedef Class__class *Class_;
class Feature_class;
typedef Feature_class *Feature;
class Formal_class;
typedef Formal_class *Formal;
class Expression_class;
typedef Expression_class *Expression;
class Case_class;
typedef Case_class *Case;

typedef list_node<Class_> Classes_class;
typedef Classes_class *Classes;
typedef list_node<Feature> Features_class;
typedef Features_class *Features;
typedef list_node<Formal> Formals_class;
typedef Formals_class *Formals;
typedef list_node<Expression> Expressions_class;
typedef Expressions_class *Expressions;
typedef list_node<Case> Cases_class;
typedef Cases_class *Cases;

class method_class;


#define Program_EXTRAS                          \
virtual void semant() = 0;			\
virtual void dump_with_types(ostream&, int) = 0; 

class TypeDeclarations {
  public:
    SymbolTable<Symbol, Symbol> *identifiers;
    SymbolTable<Symbol, method_class> *methods;
    TypeDeclarations() : identifiers(new SymbolTable<Symbol, Symbol>()), methods(new SymbolTable<Symbol, method_class>()) {
      identifiers->enterscope();
      methods->enterscope();
    };
};

#define program_EXTRAS                          \
void semant();     				\
void dump_with_types(ostream&, int);            

#define Class__EXTRAS                   \
virtual Symbol get_filename() = 0;      \
virtual void dump_with_types(ostream&,int) = 0; \
virtual Symbol get_name() = 0; \
virtual Symbol get_parent() = 0; \
virtual void semant() = 0;			\
virtual void load_type_info(Symbol) = 0;

#define class__EXTRAS                                 \
Symbol get_filename() { return filename; }             \
void dump_with_types(ostream&,int);  \
Symbol get_name(); \
Symbol get_parent(); \
void semant();			\
void load_type_info(Symbol);

#define Feature_EXTRAS                                        \
virtual void dump_with_types(ostream&,int) = 0; \
virtual void load_type_info(Symbol) = 0; \
virtual Symbol get_name() = 0;

// this is defined for method and attribute
#define Feature_SHARED_EXTRAS                                       \
void dump_with_types(ostream&,int);    \
void load_type_info(Symbol); \
Symbol get_name();

#define method_EXTRAS \
Formals get_formals(); \
Symbol get_return_type();

#define Formal_EXTRAS                              \
virtual void dump_with_types(ostream&,int) = 0;


#define formal_EXTRAS                           \
void dump_with_types(ostream&,int);


#define Case_EXTRAS                             \
virtual void dump_with_types(ostream& ,int) = 0;


#define branch_EXTRAS                                   \
void dump_with_types(ostream& ,int);


#define Expression_EXTRAS                    \
Symbol type;                                 \
Symbol get_type() { return type; }           \
Expression set_type(Symbol s) { type = s; return this; } \
virtual void dump_with_types(ostream&,int) = 0;  \
void dump_type(ostream&, int);               \
Expression_class() { type = (Symbol) NULL; } \
virtual Symbol infer_type(SymbolTable<Symbol, Symbol>, SymbolTable<Symbol, TypeDeclarations>, Symbol) = 0; // O, M (and attrs) and C

#define Expression_SHARED_EXTRAS           \
void dump_with_types(ostream&,int);


#define object_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define no_expr_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define isvoid_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define new__EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define string_const_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define bool_const_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> , SymbolTable<Symbol, TypeDeclarations> , Symbol );

#define int_const_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define comp_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define leq_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define eq_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define lt_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define neg_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define divide_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define mul_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define sub_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define plus_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c);

#define let_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define block_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define typcase_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define loop_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define cond_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define dispatch_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define static_dispatch_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#define assign_EXTRAS \
Symbol infer_type(SymbolTable<Symbol, Symbol> a, SymbolTable<Symbol, TypeDeclarations> b, Symbol c) {return NULL;};

#endif
