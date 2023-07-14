# C++ Handler

BPS is a key-value data storing structure. This handler provides a BPS data structure for C++.


## Guides and Documentation

The documentation of BPS and this handler can be found [here](https://bps-lib.github.io/). It contains all guides and detailed documentation.


## BPS Handler Operations

### BPS Class

#### Parsing operations

The `BPS` class has two methods to transform data. The method `parse()` will parse a string containing data in BPS notation. The method `plain()` will parse a `std::map<std::string, std::any>` in a string containing the data in BPS notation.

```cpp
public void Foo()
{
    std::string bps_notation_data = "bar:255;";

    // Parsing a string in a std::map<std::string, std::any>
    std::map<std::string, std::any> file = BPS.parse(bps_notation_data);
    
    // Writing in the console a string representation of a std::map<std::string, std::any>
    std::cout << BPS.plain(file);
}
```
