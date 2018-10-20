
(*
 *  Cons inherits all operations from List. We can reuse only the cons
 *  method though, because adding an element to the front of an emtpy
 *  list is the same as adding it to the front of a non empty
 *  list. All other methods have to be redefined, since the behaviour
 *  for them is different from the empty list.
 *
 *  Cons needs two attributes to hold the integer of this list
 *  cell and to hold the rest of the list.
 *
 *  The init() method is used by the cons() method to initialize the
 *  cell.
 *)

class Foo {
   car : Int;	-- The element in this list cell
   cdr : Main;	-- The element in this list cell

};

class Main {

   car : Int;	-- The element in this list cell
   cdr : Main;	-- The element in this list cell

   init(i : Int, rest : Main, pest: Int, jest: Main, lest: Foo) : Main {
      {
	 car <- i;
	 cdr <- new Main;
	 self;
      }
   };

};

