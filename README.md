# Crab
Ignore the 'pp' in the github title, mansilla working conditions did not like the original name. 



## Why?
This is a collection of utility template classes that replaces certain aspects of the STL.


## All Features 

### Preamble 
A [header file](include/preamble.hpp) for 
type aliases to make code more readable.

```cpp
#include <crabpp/preamble.hpp>

int main() {
    u32 a = 42;
    
    // Vector (std::vector<T, ...>)
    Vec<i32> numbers{};
    numbers.push_back{4};
    
    // Map (std::unordered_map<K, V, ...>)
    Dictionary<usize, String> dict;
    
    // Unit (0-sized) Type 
    std::variant<i32, unit> int_or_none;
    
    // conversion operator from degrees -> radians
    f32 radians = 90_deg; 
    
    // ... many more
}
```

### Ref

### Box

### Rc 

### Range 

### Option 

### Result 

### Pattern Matching
