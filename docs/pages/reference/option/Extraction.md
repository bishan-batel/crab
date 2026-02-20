# Extraction

Using the `get` method on an option is only ever valid if the option does contain a value, 
if it doesn't the program will [Panic](/pages/reference/Panic).

```cpp 
// try to get an environment variable SOME_ENV_VARIABL
Option<String> value = crab::env::get_as_string("SOME_ENV_VARIABLE");

if (value.is_some()) {
  fmt::println("SOME_ENV_VARIABLE={}", value.get());
}
```

More specifically, any `Option<T>::get()` returns a `T&` or `const T&`. Option *does not allow interior mutability*, ie. if you have a const option, then
get will return a const reference. If the option is mutable, get returns a mutable reference. 

> [!WARNING] 
> To prevent dangling references - you are *not* able to call get on an rvalue `Option` (one in the process of being moved). Therefore code like this would **not compile**:
>```cpp 
>// this statement is ill formed and will not compile. 
>// you cannot get a reference to temporary 
>const String& ref = Option<String>{"Message"}.get(); 
>```

?>If you are trying to wring out as much performance as possible in a place where you know the option will not be `None`, then you can use the [unsafe](/pages/reference/Unsafe) version `get_unchecked` which *only panic when compiling in debug mode*.

The way to fully extract (move) the value out of an option is by calling `unwrap`. Note that `unwrap` is *rvalue qualified* meaning you can only use in on an option that has been moved.

```cpp 
Option<String> name = "Kishan";

String name2 = crab::move(name).unwrap();

// IMPORTANT NOTE, after a move the Option will *always* contain none!
crab_check(name.is_none());
```

Similar to `get`, `unwrap` will panic if the option does not contain a value, hence you should *always check*.
