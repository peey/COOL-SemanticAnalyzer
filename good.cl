class C {
	a : Int <- 1;
	b : Bool <- a <= 10;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
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
