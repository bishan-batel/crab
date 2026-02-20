# Extraction

Using the `get` method on an option is only ever valid if the option does contain a value, if it doesn't the program will [Panic](/pages/reference/Panic);

More specifically, any `Option<T>::get()` returns a `T&` or `const T&`. Option *does not allow interior mutability*, ie.

?>If you are trying to wring out as much performance as possible in a place where you know the option will not be `None`, then you can use the [unsafe](/pages/reference/Unsafe) version `get_unchecked` which *only panic when compiling in debug mode*.
