# Overview

There are three phases to the semantic checks

## Phase 1

Here we just go through all the classes declared by the programmer and make a map between class names (symbols) and class body. 

This is done because in the entire program, the classes are refered to using their names and not by passing the actual class object. This is close to how even in the AST, various places like parameter declarations all refer to the class by the name.

We also check for class declaration related errors in this phase: duplicate class declarations (including clashes with built-in classes), illegal inheritance from disallowed base classes.

This phase also powers the `assert_type_exists` method which is used in the other two phases.

## Phase 2

I've implemented an InheritanceTree class which holds the parent-child relation between classes. The design is clean and lets you get things like chain of ancestors (useful for attribute initialization) pretty easily.

Here we use DFS to detect dependency cycles, and when there are none we start loading the classes into an inheritancetree by processing the ancestors first. 

As we are building the inheritance tree, we initialize the type environments O and M by walking through class features.

## Phase 3

Using the type environment information we got from phase 2, we can now just simply systematically walk through all the classes and do type inference (from the cool type rules), do more element-based checks (such as duplicate parameter names, invalid let binding names).

This is where majority of the work is done and each instance of it is commented, and in some places where needed, I've provided references to the cool manual.

# Changes to cool-tree header files

We need to expose a few members of the type "Class_" since we need access to parent and current class information to build up the inheritance tree. 

We also do this for formal, branch, features (attr and method both). The `get_type` methods return pointers by design since storing in symbol table requires pointers .

There are two prominent methods on all expressions: `infer_type` and `ias_type` (infer and set type). The latter is just a wrapper around the former which also calls the `set_type` method on expressions. The `infer_type` method is what gets called in phase 3 and it implements most of the type rules from the cool manual.

# Error recovery and reporting

We are able to recover from type errors by using the strategy of assigning `No_type` to the expressions which give a type error.

For other errors like duplicate names, we're still able to recover from the error by not running the code which wil cause the error (like setting duplicate name in the type environment) but we continue with as many checks as we can. The details for each instance are commented.

We always report very precise line numbers, thanks to the function `semant_element_error` which I've defined and explained below

# Notable functions

`semant_element_error` takes a classname and the tree node, and then calls the built-in `semant_error(filename, tree_node)` to report errors with accurate line numbers.

`is_supertype_of` checks if a type is supertype of another. 

We also handle SELF_TYPE and No_type. This is done in ClassTable::is_supertype_of as a special case, and in general when two types are present, `is_supertype_of` simply look at the tree and finds if the supertype is an ancestor of the subtype.

`assert_supertype` is a wrapper around the above which also reports the error (accurate with line numbers)

`assert_type_valid` checks if type is loaded in the class table, and handles error reporting when it isn't and also for SELF_TYPE

`lowest_common_ancestor`
finds the least upper bound by following the lowest common ancestor algorithm (with adjustments for No_type and SELF_TYPE).

No test cases are failing

