class C {
	a : Int <- 1;
	b : Bool <- a <= 10;
	init(x : Int, y : Bool) : C {
       {
		a <- x;
		b <- y;
    while y loop {
      a <- a + x;
    } pool;
    self;
   }
	};

	init2(x : Int, y : Bool) : Object {
       {
		a <- x;
		b <- y;
    while y loop {
      a <- a + x;
    } pool;
    init(x, y);
    init3(x, y);
   }
	};

	init3(x : Int, y : Bool) : Object {
   {
     let x : Bool <- b in {
        x <- true;
        y <- false;
        b <- x;
     };
     case self of
        n : D => (new C);
        n : C => (new D);
        n : Main => (new C);
           esac;
   }
	};
};

class D inherits C {
  d: Int;
};

Class Main inherits IO {
	main():C {
	  (new C).init(1,true)

	};
};
