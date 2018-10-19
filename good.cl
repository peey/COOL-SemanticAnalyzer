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
   }
	};
};

class D inherits C {
  d: Int;
};

Class Main {
	main():C {
	  (new C).init(1,true)
	};
};
