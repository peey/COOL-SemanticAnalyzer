#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class InheritanceTree {
  private:
    char* root;
    List<InheritanceTree> *children;
  public:
    InheritanceTree(Symbol r) {
      root = r->get_string();
    };

    InheritanceTree(char* r) {
      root = r;
    };

    InheritanceTree *find(Symbol cl) {
      return find(cl->get_string());
    };

    InheritanceTree *find(char* cl) {
      if (strcmp(root, cl) == 0) {
        return this;
      } else {
        List<InheritanceTree> *lst = children;
        while(lst != NULL) {
          InheritanceTree *result = lst->hd()->find(cl);
          if (result == NULL) {
            lst = lst->tl();
          } else {
            return result;
          }
        }
        return NULL;
      }
    };

    void add_child(Symbol child) {
      add_child(child->get_string());
    }

    void add_child(char* child) {
      children = new List<InheritanceTree>(new InheritanceTree(child), children);
    }

    void levels(int n) {
      cout << root << endl;
      List<InheritanceTree> *lst = children;
      cout << "============LEVELSSS========="<< n << "=========" << endl;
      while(lst != NULL) {
        lst->hd()->levels(n - 1);
        lst = lst->tl();
      }
    }
};

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  SymbolTable<Symbol, Class_> *table;
  InheritanceTree *tree;
};

class TypeDeclarations {
  public:
    SymbolTable<Symbol, Symbol> *identifiers;
    SymbolTable<Symbol, method_class> *methods;
    TypeDeclarations() : identifiers(new SymbolTable<Symbol, Symbol>()), methods(new SymbolTable<Symbol, method_class>()) {
      identifiers->enterscope();
      methods->enterscope();
    };
};

#endif

