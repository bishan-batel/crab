# Panic

A panic in crab is relatively the same as a panic in rust, or an *assertion failure* in C++. A panic occurring does not mean a recoverable error ocurred, but instead that some precondition has been broken and the author of the code chose to explicitly stop the program rather then allow UB.

Normally you would not call panic yourself, most times you interact with panics are ones caused by a failed [check](/pages/reference/assert/Check.md).
If you wish to call panic yourself, all you need is some `StringView` for a message and a `SourceLocation` (`std::source_location`) for the panic to display *where* this ocurred for easier debugging.

```cpp 
crab::assertion::panic("Test Vaue", SourceLocation::current());
```

## Changing the behavior of Panic
The default behavior of panic is to log the message to `std::cerr` and abort the program. You can change this default behavior to instead through a `runtime_exception` by defining `CRAB_THROW_ON_DEFAULT_PANIC=1`

```cpp
#define CRAB_THROW_ON_DEFAULT_PANIC 1
```

> [!WARNING]
> Note that this is **highly discouraged**, as like assertion failures - ignoring a panic and continuing program execution *may lead to undefined behavior*. One valid use case however is for unit tests where you want to assert that a given function will panic.

### User-Defined Panic Handlers

You can modify the global panic handler by calling `crab::assertion::panic_handler::set`, which will take in a `std::function<void(PanicInfo)>`, where `PanicInfo` is a struct containing details about the panic.

```cpp 
// set the handler
crab::assertion::panic_handler::set([](const crab::assertion::PanicInfo& info) {
    fmt::println("Panic Ocurred: {:?} at location {}", info.message, info.location);
    // note this is a bad thing to do, a panic means preconditions failed therefore 
    // it is not guarenteed the program state to be valid, all crab types assume 
    // that a panic means *end of execution*.
});

// cause a panic
crab::assertion::panic("Panic", SourceLocation::current());

// code below is valid because we did not exit the program in the handler
```
