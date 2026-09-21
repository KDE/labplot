# Adding Python Bindings for a New C++ Class

Checklist for exposing a new LabPlot C++ class to Python via Shiboken6.
Three files must be updated, plus `bindings.h` for the header include.

---

## 1. `lib/python/bindings.h`

Add the `#include` for the class header. Group it with similar classes.

```cpp
// helper classes
#include "src/backend/somemodule/MyClass.h"
```

---

## 2. `lib/python/bindings.xml`

Add the type system entry. Use `<rejection>` to suppress all auto-detected members,
then `<declare-function>` to opt-in only what you want to expose.

```xml
<rejection class="MyClass" field-name="*"/>
<rejection class="MyClass" function-name="*"/>
<object-type name="MyClass" disable-wrapper="yes">
    <enum-type name="SomeEnum"/>
    <declare-function signature="MyClass(const QString&amp;)" return-type="void"/>
    <declare-function signature="doSomething()" return-type="void"/>
    <!-- add further methods -->
</object-type>
```

**Key XML rules:**
- Use `&amp;` for `&` inside XML attribute values (e.g. `const QString&amp;`).
- Prefer `<declare-function>` over `<modify-function>`: the latter only modifies
  auto-detected signatures and silently does nothing when detection fails.
- `return-type` is only valid on `<declare-function>`, not `<modify-function>`.
- If the class has inner enums you want exposed, add `<enum-type name="…"/>` inside
  the `<object-type>` block.
- If the `AspectType` enum has a value for the new class, add it to the
  `<reject-enum-value>` list at the top of the file if it should not appear in
  the SDK enum.

---

## 3. `lib/python/CMakeLists.txt` — `generated_sources`

Add the two generated files Shiboken will produce (`.cpp` and `.h`).
Group them with similar classes.

```cmake
# helper classes
${CMAKE_CURRENT_BINARY_DIR}/pylabplot/myclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/pylabplot/myclass_wrapper.h
```

The naming convention is always the lowercase class name with `_wrapper` suffix.
Inner classes (e.g. `XYFitCurve::FitData`) become separate files:
`xyfitcurve_fitdata_wrapper.cpp`.

---

## 4. `cmake/PythonScripting.cmake` — `shiboken_scripting_generated_sources`

**This list is separate from step 3 and is easy to forget.**
It drives the `ENABLE_PYTHON_SCRIPTING` in-app build path (the files are compiled
directly into `liblabplotbackendlib`). Missing an entry here causes a linker error:

```
"init_MyClass(_object*)", referenced from:
    exec_pylabplot(_object*) in liblabplotbackendlib.a(pylabplot_module_wrapper.cpp.o)
```

Add the same two lines in the same position as in step 3:

```cmake
# helper classes
${CMAKE_CURRENT_BINARY_DIR}/pylabplot/myclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/pylabplot/myclass_wrapper.h
```

> **Rule:** whenever you touch `lib/python/CMakeLists.txt` (step 3), make the
> identical change in `cmake/PythonScripting.cmake` (step 4).

---

## Quick checklist

| File | What to add |
|------|-------------|
| `lib/python/bindings.h` | `#include` for the class header |
| `lib/python/bindings.xml` | `<rejection>` + `<object-type>` block |
| `lib/python/CMakeLists.txt` | `*_wrapper.cpp` and `*_wrapper.h` in `generated_sources` |
| `cmake/PythonScripting.cmake` | Same `.cpp`/`.h` pair in `shiboken_scripting_generated_sources` |

After editing, re-run CMake (configure step) and rebuild. Shiboken regenerates
the wrapper sources automatically during the build.
