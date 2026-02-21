Formattability of any type $T$ is transitive through `Option`, which is to say that Option is able to be formatted if the type it contains is also formattable. Note that 'formatability' is based on if `fmtlib` considers it formattable (fmtlib supports `std::format` so if it works for that it should work with `fmt`~).

```cpp
Option<i32> a = 10;
crab_check(fmt::format("{}", a) == "Some(10)");
a = crab::none;
crab_check(fmt::format("{}", a) == "None");
```

To make your type formattable, consult the [fmtlib docs](https://fmt.dev/11.0/api/#formatting-user-defined-types).
