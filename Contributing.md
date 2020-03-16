# Contributing to CT Lib

## <a name="cpp-code-style"></a>C++ Coding Style and Formatting

Sections:
- [General](#cpp-code-style-general)
- [Headers and Includes](#cpp-code-style-headers)
- [Brackets](#cpp-code-style-brackets)
- [Naming](#cpp-code-style-name)
- [Classes and Structs](#cpp-code-style-classes)
- [Long Lines Formatting](#cpp-code-style-long-lines)

### <a name="cpp-code-style-general"></a>General

Here are a few quick rules before getting into more details.
- The code is currently using the C++17 standard.
- It is preferable if lines does not exceeds 100 characters, but it is not required.
- The indentation style used is 4 spaces per level.
- Use `nullptr` when possible, `NULL` otherwise.
- Do not use `using namespace [x];`.
- Prefer the prefix form for increment/decrement over the postfix form. `++i`
- Pointers and references are against the type, not the name.
  - OK: `int* myIntPtr;` `int& myIntRef;`
  - Not OK: `int *myIntPtr;` `int &myIntRef;`
- When declaring multiple variables in a single statement, do not mix pointers, references, and
  regular variables.
  - OK: `int* myPtr0, * myPtr1, * myPtr2;`
  - Not OK: `int myInt, * myPtr, & myRef;`
  - Also not OK: `int* myPtr0, *myPtr1, *myPtr2;`
- Never omit curly brackets on one line bodies.
  - OK:
    ```cpp
    if (condition)
    {
        // ...
    }
    ```
  - Not OK:
    ```cpp
    if (condition)
        // ...
    ```
    ```cpp
    if (condition) // ...
    ```

### <a name="cpp-code-style-headers"></a>Headers and Includes

- The include guard used by this project is `#pragma once`.
- The file extention for headers is `.hpp`.
- Includes in a file should be grouped as follow:
  - The header of the source file.
  - Standard library headers.
  - A group for each external library headers (e.g., Qt).
  - System specific headers.
  - Other headers of this project
- The headers within each group should be ordered alphabetically.
- Remove duplicate or unnecessary includes.

### <a name="cpp-code-style-brackets"></a>Brackets

The body of namespaces, classes, enums, structs, and functions must have its opening curly bracket
on a new line.

**OK**:
```cpp
namespace MyNamespace
{
    // ...
}
```

**_Not_ OK**:
```cpp
namespace MyNamespace {
    // ...
}
```

**OK**:
```cpp
class MyClass
{
    // ...
};
```

**_Not_ OK**:
```cpp
struct MyStruct {
    // ...
};
```

Lambdas can have their curly brackets on the same line only if the body contains a single small
statement. Place the curly brackets on new lines otherwise.

**OK**:
```cpp
auto myLambda = [](int number) -> int { return number * 2; };
```

**_Not_ OK**:
```cpp
auto myLambda = [](int number) -> int { return number * 2 /* many many more operations just to take more characters :) */; };
```

**OK**:
```cpp
auto myLambda = [](int number) -> int
{
    return number * 2;
};
```

**_Not_ OK**:
```cpp
auto myLambda = [](int number) -> int {
    return number * 2;
};
```

### <a name="cpp-code-style-name"></a>Naming

The name of namespaces, classes, enums, and structs should be in upper CamelCase. Any acronym in
names should be in full upper case.

```cpp
namespace MyNamespace
class MyClass
enum MyHTMLColors
```

The name of compile-time constants and macros should be in MACRO_CASE.

```cpp
#define MY_MACRO 123
constexpr int MAX_COUNT = 200;
```

The name of function and variables should be in camelCase. Acronyms should be in upper case except
if at the start of the name.

```cpp
void myFunc();
int myInt = 0;
```

### <a name="cpp-code-style-classes"></a>Classes and Structs

- If making a [POD](http://en.wikipedia.org/wiki/Plain_Old_Data_Structures), use a struct. Use a
class otherwise.

- Friend declarations should be at the beginning of the class, and the order should be: classes,
operator overloads, then functions.

- The order of access modifiers  should be: `public`, `protected`, then `private`.

- The order of member declarations should be: compile-time constants, static functions,
constructors and destructor, operator overloads, functions, static variables, then variables.

- The Qt `signal` should be between `public` and `protected` when applicable.

Here is an example class declaration with the ordering described above.

```cpp
class MyClass : public MyBase
{
    friend class MyFriendClass;
    
    friend int myFriendFunction(int number);
    
public:
    static MyClass myStaticFunc(int number);
    
    MyClass();
    virtual ~MyClass();
    
    int getValue() const;
    
    operator bool() const;
    
    bool myBool;
    
protected:
    MyClass(int value);
    
    MyClass operator++();
    
    void setValue(int value);
    
private:
    void myPrivateFunc();
    
    static int myStaticInt;
    
    int value;
};
```

### <a name="cpp-code-style-long-lines"></a>Long Lines Formatting

When dealing with long lines of code (function with long headers, constructors with long
initializer lists, etc.), keeping everything on the same line can get really confusing. Since this
section is very long, here is are some quick links to specific topics:

- [Function Headers](#cpp-code-style-long-lines-funcs)
- [Constructor Initializer Lists](#cpp-code-style-long-lines-constr)
- [Function Calls](#cpp-code-style-long-lines-calls)
- [Chained Operators](#cpp-code-style-long-lines-ops)

#### <a name="cpp-code-style-long-lines-funcs"></a>Function Headers

When a function header is longer than 100 characters, the header must be rewritten in one of the
following way:
- Break the header onto multiple lines.
  - The parentheses must be on their own line.
  - Each parameter is on a new line, with 1 level of indentation more than the header.
- When multiple functions requires the same parameters, you can use a `struct` with its members
being the parameters.

**OK**:
```cpp
VeryLongClassName VeryLongClassName::veryLongFunctionName
(
    int initialSize,
    float ratio,
    char* data,
    size_t dataSize,
    bool order = true,
    double factor = 0.5
)
{
    // ...
}
```

**_Not_ OK**:
```cpp
VeryLongClassName VeryLongClassName::veryLongFunctionName(int initialSize,
                                                          float ratio,
                                                          char* data,
                                                          size_t dataSize,
                                                          bool order,
                                                          double factor)
{
    // ...
}
```

**OK**:
```cpp
struct FuncParams
{
    int initialSize;
    float ratio;
    char* data;
    size_t dataSize;
    bool order = true;
    double factor = 0.5;
};

VeryLongClassName VeryLongClassName::veryLongFunctionName0(FuncParams& p)
{
    // ...
}

VeryLongClassName VeryLongClassName::veryLongFunctionName1(FuncParams& p)
{
    // ...
}
```

#### <a name="cpp-code-style-long-lines-constr"></a>Constructor Initializer Lists

When the constructor header is longer than 100 characters, follow the same pattern used for
[function headers](#cpp-code-style-long-lines-funcs).

The member initializer list should be formatted as follow:
- The colon should always be separated from the parenthesis by a single space.
- If the header plus the initializer list is less than 100 characters, then everything can be on
the same line. [[1]](#cpp-code-style-long-lines-constr-1)
- If the header is less than 100 characters, but adding the initializer list would surpass that
limit, then put the initialization of the base class and members on a new line each.
[[2]](#cpp-code-style-long-lines-constr-2)
- If the header is more than 100 characters, then format the header the same way as
[function headers](#cpp-code-style-long-lines-funcs), and the initializer list as described in the
previous point. [[3]](#cpp-code-style-long-lines-constr-3)

Examples:

<a name="cpp-code-style-long-lines-constr-1"></a><sub>1.</sub>
```cpp
ClassName::ClassName(int value) : BaseClass(value)
{
    // ...
}
```

<a name="cpp-code-style-long-lines-constr-2"></a><sub>2.</sub>
```cpp
ClassName::ClassName(int value, float ratio, bool order) :
    BaseClass(value),
    ratio{ratio},
    order{order}
{
    // ...
}
```

<a name="cpp-code-style-long-lines-constr-3"></a><sub>3.</sub>
```cpp
VeryLongAndTediousClassName::VeryLongAndTediousClassName
(
    int value,
    float ratio,
    bool order
) :
    BaseClass(value),
    ratio{ratio},
    order{order}
{
    // ...
}
```

#### <a name="cpp-code-style-long-lines-calls"></a>Function Calls

Long function calls should be formatted in one of the following ways:
- One argument per line, with 1 level of indentation. The opening parenthesis can be on either the
same line or new line. [[1]](#cpp-code-style-long-lines-calls-1)
- Same as the above, but some arguments grouped when it makes sense.
[[2]](#cpp-code-style-long-lines-calls-2)
- Put as many arguments as possible (within 100 characters) per line, with 1 level of indentation.
The opening parenthesis can be on either the same line or a new line.
[[3]](#cpp-code-style-long-lines-calls-3)

Examples:

<a name="cpp-code-style-long-lines-calls-1"></a><sub>1.</sub>
```cpp
void myFunction()
{
    // ...
    veryLongAndTediousFunctionCallWithLotsOfParams(
        x,
        y,
        100,
        SomeClass::someStaticPointer->someMember.someMethod(),
        nullptr
    );
    veryLongAndTediousFunctionCallWithLotsOfParams
    (
        x,
        y,
        100,
        SomeClass::someStaticPointer->someMember.someMethod(),
        nullptr
    );
    // ...
}
```

<a name="cpp-code-style-long-lines-calls-2"></a><sub>2.</sub>
```cpp
void myFunction()
{
    // ...
    veryLongAndTediousFunctionCallWithLotsOfParams
    (
        x, y, // Here X and Y are grouped
        100,
        SomeClass::someStaticPointer->someMember.someMethod(),
        nullptr
    );
    // ...
}
```

<a name="cpp-code-style-long-lines-calls-3"></a><sub>3.</sub>
```cpp
void myFunction()
{
    // ...
    veryLongAndTediousFunctionCallWithLotsOfParams
    (
        x, y, z, dx, dy, dz, 100, someLocal.getValue(), "c-string",
        SomeClass::someStaticPointer->someMember.someMethod(), nullptr
    );
    // ...
}
```

#### <a name="cpp-code-style-long-lines-ops"></a>Chained Operators

Operator chains can be formatted in the one of the following ways:
- Break the operation into multiple lines, with 1 level of indentation for each new line.
  - The operators must be at the beginning of the next line, not at the end of the current.
  - Try to break the operation by 'parentheses groups' if possible.
- Break the operation into multiple variables.
  - Use this only if you can find intuitive names for the variables, since you are creating
  additional locals.

```cpp
int myFunction(int a, int b, int c)
{
    // ...
    return ((a + someLocal.getInt()) * b)
        - (c / (someLocal.processInt(b * b)))
        % someOtherLocal.getInt();
    // ^^^ Note the operators at the beginning of the next line.
}
```

```cpp
int myFunction(int a, int b, int c)
{
    // ...
    int tmp0 = (a + someLocal.getInt()) * b;
    int tmp1 = c / (someLocal.processInt(b * b));
    return tmp0 - tmp1 % someOtherLocal.getInt();
    // Use this only if you can find intuitive names for the variables,
    // since else you are just creating locals for no reason.
}
```

[Back to Table of Contents](#cpp-code-style)
