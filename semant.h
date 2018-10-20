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
    List<InheritanceTree> *children = NULL;
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
      cout << root << " at " << this << endl;
      List<InheritanceTree> *lst = children;
      cout << "============LEVELSSS========="<< n << "=========" << endl;
      while(lst != NULL) {
        if (true) {
          cout << "lhead: " << lst->hd() << endl;
          //cout << "lhead: " << lst->hd()->get_symbol() << endl;
        }
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
  ostream& semant_error(Symbol classname);
  ostream& semant_element_error(Symbol classname, tree_node *t);
  ostream& semant_error(Symbol filename, tree_node *t);
  std::map<Symbol, Class_> table2;
  InheritanceTree *tree;
  void init_attr_meth(Class_ cl);
  Class_ lookup_class(Symbol s) {
    auto it = table2.find(s);
    if (it == table2.end()) {
      // note: this is for debugging only. Lookup_class hasn't been used in an unsafe way anywhere in the code so far
      error_stream << "looked up class '" << s << "' does not exist in the table" << endl;
      return NULL;
    } else {
      Class_ cl = table2.find(s)->second;
      //cout << "marco -1" << endl;
      return cl;
    }
  };
  bool is_supertype_of(Symbol t1, Symbol t2, Symbol c);
  bool assert_supertype(Symbol t1, Symbol t2, Symbol c) {
    bool result = is_supertype_of(t1, t2, c);
    if(!result) {
      semant_error(c);
      error_stream << "Type Error: "<< t1 << " is not a supertype of " << t2 << endl;
    }
    return result;
  };
  bool check_type_exists(Symbol t);
  bool assert_type_valid(Symbol t, Symbol c, tree_node *m, bool self_allowed);
  Symbol lowest_common_ancestor(Symbol a, Symbol b, Symbol c);
};

// O(class) returns a base attribute environment a class will start with
SymbolTable<Symbol, method_class>* M(Symbol cl);

#endif
