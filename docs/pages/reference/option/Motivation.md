# When to Use Option?

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


?> NOTE: Instead of passing a pointer to an array and a length it is more idiomatic in modern C++ to be using `std::span` and `std::array` and avoid raw C-style arrays.  

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


The usage of this method would look instead like this:

```cpp 
i32 array[10] = { 0 };

Option<usize> result = linear_search(array, 10, 1);

array[result.get()] = 0; // This will panic and abort the program
```

This still results in undefined behavior, but the goal of Option is to be able to easily see that your are not doing a check. What an `i32` means in a given environment is very context dependent, but if you ever see an `Option`, you know that you know that is it is invalid to access it (`get()`, `unwrap()`) *unless you checked it has a value*. This is why unlike `std::optional`, crab's `Option` does not allow the use of the dereferencing or arrow operator. It is slightly more verbose than `std::optional` but because of this it is very easy to read at a glance what you are working with is an option and not just a pointer (In my experience, this makes it easier to do code reviews when reading a PR).
