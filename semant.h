#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include <map>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class InheritanceTree {
  private:
    Symbol root;
    List<InheritanceTree> *children;
  public:
    Symbol get_symbol() {
      return root;
    };

    InheritanceTree(Symbol r) {
      root = r;
    };

    InheritanceTree *find(Symbol cl) {
      if (cl == root) {
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

    List<InheritanceTree> *ancestor_chain(Symbol cl) {
      //cout << "looking up ancestors, args are " << root << " and " << cl << endl;
      if (root == cl) { // BASE CONDITION 1
        return new List<InheritanceTree>(this, NULL);
      } else if (children == NULL) { // BASE CONDITION 2, leaf but not a match
        return NULL;
      } else {
        List<InheritanceTree> *lst = children;
        while(lst != NULL) {
          List<InheritanceTree> *result = lst->hd()->ancestor_chain(cl);
          if (result != NULL) {
            return new List<InheritanceTree>(this, result); // add self to result
          }
          lst = lst->tl();
        }
        return NULL; // same as base condition 2, dead code
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
  std::map<Symbol, Class_> table2;
  InheritanceTree *tree;
  Class_ lookup_class(Symbol s) {
    return table2.find(s)->second;
  };
  bool is_supertype_of(Symbol t1, Symbol t2, Symbol c);
};

// O(class) returns a base attribute environment a class will start with
SymbolTable<Symbol, method_class>* M(Symbol cl);

#endif
