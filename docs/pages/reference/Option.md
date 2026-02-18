# Option

Option is the most fleshed out and indepth type in crab. TLDR: `std::optional` with a more expressive API, support for reference types, niche optimizations, and explicitness.

## Introduction

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

## When to Use Option?

The primary use of option is to avoid using sentinal values, which are values that denote some 'empty' state. A common example would be something like an implementation of linear search.

A traditional C style function to find the index with a given value in an array would look something like:
```cpp
i32 linear_search(const i32* array, usize length, i32 target_value) {

  for (usize i = 0; i < length; i++) {
    if (array[i] == target_value) {
      return i; // found target, return index
    }
  }
  
  // return -1 to denote failure
  return -1;
}
```

This code is perfectly functional, but their are a couple of semantic issues when using this function. There are things like how because it can return -1 it returns an `i32` for a index rather than a `usize`, but the biggest issue is that this function can fail in a non obvious way. The failure case is obvious when looking at the body of the function, its where it can't an index with the given value so it returns -1. However, if we just look at the function signature (what you would see in a header),

```cpp 
i32 linear_search(const i32* array, usize length, i32 target_value);
```

It is not obvious how this function could fail. Most C/C++ programmers could probaly guess, but if the only way to know how a function fails is to read the source or rely on documentation, then I believe that to be a subpar API. 

This issue also arises with how we typically deal with errors/failure in C++, this form of handling makes it extremely easy to elide proper error handling. At a glance, it is difficult to see that the author of this snippet is not properly handling a failure case.

```cpp 
i32 array[10] = { 0 };
i32 i = linear_search(array, 10, 1);

array[i] = 0; // UB
```

This brings us to `Option`. The goal of `Option` in many such cases is to encode the idea of 'missing value' / 'failure' into the type system.

```cpp 
Option<usize> linear_search(const i32* array, usize length, i32 target_value) {
  for (usize i = 0; i < length; i++) {
    if (array[i] == target_value) {
      return i;
    }
  }

  return crab::none; // could not find index
}
```



## Bells and Whistles

### Monadic Operations

#### crab::fallible
