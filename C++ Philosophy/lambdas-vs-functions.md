# Key Differences Between Lambda Functions and Normal Functions

| **Aspect**              | **Lambda Function**                                                  | **Normal Function**                                               |
|-------------------------|----------------------------------------------------------------------|-------------------------------------------------------------------|
| **Definition Overhead**  | Lambdas are **inline** and defined where they are used.              | Requires a separate function definition in the code.             |
| **Variable Captures**    | Lambdas can capture variables from their surrounding scope.          | Normal functions cannot access variables from their enclosing scope unless passed explicitly. |
| **Memory Usage**         | If a lambda captures variables, it creates a **closure**, which uses additional memory. | Normal functions don't have capture overhead.                   |
| **Inlining Potential**   | Lambdas can often be **inlined** by the compiler, reducing function call overhead. | Normal functions can also be inlined, but not as easily (depends on compiler optimization). |
| **Reusability**          | Typically used for single-use purposes; no reuse outside their scope. | Normal functions are reusable globally or within their scope.    |

---
