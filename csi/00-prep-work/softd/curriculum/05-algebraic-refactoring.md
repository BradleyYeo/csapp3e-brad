# Algebraic Refactoring

Data structures and control flow obey ordinary high-school algebra. Counting type inhabitants quantifies state bloat, and algebraic factoring eliminates duplicated logic mechanically.

# Types as Algebraic Expressions

## Inhabitant Counting Rules

- Zero type (`!` / `Never`): `0` inhabitants.
- Unit type (`()`): `1` inhabitant.
- Boolean (`bool`): `2` inhabitants.
- Sum type (`enum E { A(X), B(Y) }`): `|E| = |X| + |Y|`.
- Product type (`struct S { x: X, y: Y }`): `|S| = |X| * |Y|`.
- Option (`Option<T>`): `|T| + 1`.
- Function (`fn(A) -> B`): `|B|^|A|`.

## Diagnosing Bloat With Cardinality

- Suppose an access-control grant has 3 principal kinds, 4 optional/nullable credential fields, and 2 independent booleans (`is_readonly`, `allow_exec`).
- Multiplying independent `Option` and `bool` fields inflates representable credential combinations exponentially even though most combinations are invalid.
- Turning independent optional fields into a tight sum of exact per-variant structs drops the illegal states to zero.

# Factoring Polynomials in Types and Code

## Distributivity (`A * (B + C) ~= A * B + A * C`)

- Unfactored form (`A * B + A * C`): Duplicating `scope`, `expires_at`, and `permission` across every variant of an enum encourages copy-pasted validation where one variant (`BreakGlass`) accidentally forgets the expiration check.
- Factored form (`A * (B + C)`): Pulling common envelope metadata (`scope`, `expires_at`, `permission`) into an outer product struct wrapped around an inner sum (`PrincipalCredential`) forces shared invariants to be checked once in one place.

## Factoring Control Flow

- Nested duplicated `if/else` branches behave like unfactored sums of products: factor out shared preconditions (`expires_at`, `scope`) above the `match`, leaving the `match` arms solely responsible for variant-specific logic.
- Classic patterns—Strategy, State, Command, and Visitor—are special cases of choosing between function/trait products (`&dyn Trait`) and explicit sum types (`enum`) depending on which dimension needs openness.

# Mental Model and Connections

- Algebraic factoring pairs with Unit 3's Representable/Valid Principle: use cardinality math to measure how many illegal states remain, and distributivity to decide whether fields belong inside the sum or in the outer product.
- Unit 5 lab preview: In [unit-05-algebraic-refactoring](../labs/unit-05-algebraic-refactoring/README.md), you shrink an access-control grant's state space, factor `GrantEnvelope` out of principal credentials, collapse contradictory booleans into `PermissionLevel`, and eliminate an omitted expiration check.

# Active Recall

- Compute the inhabitant count of `struct Bad { kind: K, a: Option<bool>, b: Option<bool> }` where `|K| = 3`, versus `enum Good { K1(bool), K2(bool), K3 }`.
- When should common fields be factored into an outer `struct` wrapping an `enum` (`A * (B + C)`) versus pushed inside each enum variant?
- How does factoring `A * B + A * C` into `A * (B + C)` help preserve "intent in a single place"?
