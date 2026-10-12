# TSD Reference

## `ASSERT_X(EXPR)`

Assertion with program termination.

- `EXPR`: logical expression.

### Example
```C
ASSERT_X(1==1);
```

### Intended use

Used to check parameters in the test function scope.

## `ASSERT(EXPR)`

Assertion without program termination.

- `EXPR`: logical expression.

### Example
```C
ASSERT(1==1);
```

### Intended use

Used to check parameters in the test function scope.

## <a id="CONTEXT_FN_REF"></a>`CONTEXT_FN(NAME)`

Expands to:
```C
static void NAME(void *ContextData)
```
`ContextData` is a pointer to the data allocated by [CONTEXT](#Context_REF).

- `NAME`: function name.

### Example
```C
CONTEXT_FN(Setup) {
  // Cast to struct.
  MyStruct *Ref = ContextData;
  // Fake ApiError.
  RESET_FAKE(ApiError_Mock);
  ApiError_Mock_fake.custom_fake = ApiError_JMP;
  ApiError                       = ApiError_Mock;
  Ref->MyVar                     = 5;
  // [...]
}
CONTEXT_FN(Teardown) {
  (void)ContextData;
  // Asset no ApiError's
  ASSERT(ApiError_Mock_fake.call_count == 0);
  // Rewind ApiError.
  RESET_FN(ApiError);
  // [...]
}
```

### Intended use

Used to define global variables in the `C` file scope.

## <a id="Context_REF"></a>`CONTEXT(NAME, SETUP, TEARDOWN, ALLOCSIZE)`

Creates a context object.

- `NAME`: context name.
- `SETUP`: setup function name. Runs before the test is executed. Can be `NULL`.
- `TEARDOWN`: tear-down function name. Runs after the test is executed. Can be `NULL`.
- `ALLOCSIZE`: size of the data to be allocated by the context. Can be `0`. The memory is accessed via `void *ContextData` (see [CONTEXT_FN](#CONTEXT_FN_REF)).

### Intended use

Used to define global variables in the `C` file scope.

### Example
```C
CONTEXT(Context1, Setup, Teardown, sizeof(MyStruct));
CONTEXT(Context2, Setup, NULL, 0);
```

## `TEST(NAME)`

Creates a test object.

- `NAME`: test name.

### Example
```C
TEST(SimpleTest){
  ASSERT(1==1);
  // [...]
}
```

### Intended use

Used to define global variables in the `C` file scope.

## `TEST_F(CONTEXT, NAME)`

Creates a context-dependent test object.

- `CONTEXT`: context name.
- `NAME`: test name.

### Example
```C
TEST(Context1, ContextTest){
  // Cast to struct.
  MyStruct *Ref = ContextData;
  ASSERT(Ref->MyVar==5);
  // [...]
}
```

### Intended use

Used to define global variables in the `C` file scope.

## `TEST_E(NAME)`

Creates an externally-implemented test object.

- `NAME`: test name.

Note: may require a TSD language extension. See, e.g. `TSD_Fortran.h`.

### Example
```fortran
TEST_E(FortranTest)
#ifdef TSD_FORTRAN_IMPL

  use FortranFixture
  TSD_FORTRAN_INTERFACES
  call SetUp()
  ![...]
  call TearDown()

#endif
END_TEST_E
```

### Intended use

Used to define global tests in other programming languages.

## `TEST_SUITE(NAME, <TestList>)`

Creates and registers a test suite object from a list of tests. Only registered suites will be available to the test runner.

- `NAME`: test suite name.
- `<TestList>`: test list, separated by commas.

### Example
```C
TEST_SUITE(MySuite, SimpleTest, ContextTest);
```

### Intended use

Used to define global variables in the `C` file scope.
