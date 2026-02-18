# Crab

NOT a rust fanfic (ok kinda)


Example of `Option<T>`

```c
void main() {
  Option<i32> a = crab::some(a);

  fmt::println("{}", a);

  crab_check(a.is_some());

  i32 x = crab::move(a).unwrap();
}
```
