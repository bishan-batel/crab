# Introduction
`Option<T>` is a type that possibly contains a value of `T. 

```cpp 
void some_function(Option<String> name) {
  // name could have a value or be empty
}
```
In the case where a value of `Option<T>` contains a value - we say that it is `Some`. If not, then the given option is `None`. Constructing an option with a value can be done through its constructor by giving it the value to place inside. 
```cpp 
Option<i32> value = 10;
crab_check(value.is_some()); // is_some returns if the option contains a value or not

value = 42; // we can also just reassign it
crab_check(value.is_some());
```


Default constructing the option will make it `None`, however it is also possible to more explicitly denote this with `crab::none`.

```cpp 
Option<i32> value;
crab_check(value.is_none()); // is_none returns if the option is empty

// explicitly setting it to none
value = crab::none; 
Option<i32> other_value = crab::none;
crab_check(value.is_none()); 
```

If we know that the option does contain a value, we can access it (get a reference) using the `get` method.

```cpp
void print_name(StringView first_name, Option<StringView> last_name) {
  fmt::print("{}", first_name);

  if (last_name.is_some()) {
    fmt::print(" {}", last_name.get());
  }

  fmt::print("\n"); // newline
}
```

Using the `get` method on an option is only ever valid if the option does contain a value, if it doesn't the program will [Panic](/pages/reference/Panic);

?>If you are trying to wring out as much performance as possible in a place where you know the option will not be `None`, then you can use the [unsafe](/pages/reference/Unsafe) version `get_unchecked` which *only panic when compiling in debug mode*.

## Monadic Operations
The following in some high level overview of the different operations you can use with `Option`, if you wish for more detailed descriptiosn then refer to the doxygen API docs.

### Map and Flat Map

### Predicates

### Boolean Operations
#### Or
Two `Option`s of the same type can be used in an `or` (`||`) expression, which will act similar to 'null coelescing' operators in other languages. If the first `Option` contains a value, the expression will evaluate to that option, if the second `Option` contains a valeu, then the expression will evaluated to the second. If both options contain `None` then the expression evaluates to none.

```cpp 
Option<i32> a{10};
Option<i32> b{30};
Option<i32> c{};

// This works because Option can be explicitly casted to a bool, 
// meaning this is the same as crab_check((a or b).is_some())
crab_check(a or b); 

// both options are Some
crab_check(a or b == a);
crab_check((a or b).unwrap() == 10);

// right option is None
crab_check(a or c == a);
crab_check((a or c).unwrap() == 10);

// left option is None
crab_check(c or a == a);
crab_check((c or a).unwrap() == 10);

// both options are none
crab_check((c or c).is_none());

```

#### And
#### Xor

### Comparisons
#### Equal / Not Equal
#### Ordering

### crab::fallible

`crab::fallible` (or more explicitly `crab::opt::fallible` to specify the `Option` overload) is a function that operates similar to `Option::zip`. `crab::fallible` takes in any number of *functors* that each return some `Option`. `crab::fallible` will run these one by one unless one returns `None`. If any functor returned `None`, `crab::fallible` will return `None` as well - but if all suceeded then all produced options are unwrapped and placed into a `Tuple` (`std::tuple`) and returned as a `Some` value.

```cpp 
class Person final { 

  [[nodiscard]] auto get_name() const -> Option<const String&> {
    return name.as_ref();
  }

private:
  Option<String> name;
};

i32 main() {

  Person p1{"Kishan"};
  Person p2{"Patel"};
  Person p3{};

  Option<Tuple<const String&, const String&>> result = crab::fallible(
    [&p1]() { return p1.get_name(); },
    [&p2]() { return p2.get_name(); },
  );

  crab_check(result.is_some());

  auto [p1_name, p2_name] = crab::move(result).unwrap();
  crab_check(p1_name == "Kishan" and p2_name == "Patel");

  result = crab::fallible(
    [&p1]() { return p1.get_name(); },
    [&p3]() { return p3.get_name(); } // this value would return none, so this whole thing will short circuit
  );
  crab_check(result.is_none());
}
```
The use case for `crab::fallible` is a bit more sparse than its `Result` counterpart, but it can help when calling many methods that rely on previous preconditions.
