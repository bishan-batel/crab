For most types, the memory representation for `Option<T>` would be something akin to the following,
```cpp
template<typename T>
struct Option {
  T value;
  bool has_value;
};
```

The size of an option for a common type `T` would be the byte size of `T` plus the size of a bool (typically 1 byte but fun [fact this is actually implementation defined](https://en.cppreference.com/w/cpp/language/types.html#Boolean_type)!). However the layout shown is technically wrong, as instead of storing a `T` directly, the default storage for option will instead store the raw bytes of `T` as a `std::array<u8, sizeof(T)>`, and *invokes constructors & destructors manually* . What this means is if an Option contains no value - there is semantically no object contained - and treating it as such would be undefined behavior.


## Reference Types
An optional reference has a special layout / niche optimisation - the data storage for a `T&` will always be the same size as a `T*`. This means that `Option<T&>` and `T*` are essentially interchangeable. The major difference between using these two types would be `Option` denotes the possibility of 'no value' (*null*) explicitly, which (in my own experience using this library on teams), helps a lot with readability (especially in a code review).
