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

