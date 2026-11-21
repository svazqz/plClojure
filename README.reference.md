# PL/Clojure - Clojure Procedural Language for PostgreSQL

PL/Clojure allows you to write PostgreSQL functions in Clojure. It provides a bridge between PostgreSQL and the JVM, enabling you to leverage Clojure's powerful features within your database functions.

## Features

- Write PostgreSQL functions using Clojure
- Full access to Clojure's standard library and Java interop
- Support for array arguments and text return values
- Seamless integration with PostgreSQL's type system

## Prerequisites

- PostgreSQL 14 or later
    - Install with Homebrew:
    ```bash
    brew install postgresql@14
    ```
- Java Development Kit (JDK) 8 or later
- Clojure 1.9.0 or later
- Bazel build system

## Installation

1. Run install.sh:
```bash
chmod u+x install.sh
./install.sh
```

## Examples
### String Manipulation
```sql
SELECT pl_clojure_call_array(
    '(fn [& args] 
        (let [words args
              sorted (sort words)
              capitalized (map clojure.string/capitalize sorted)]
          (clojure.string/join ", " capitalized)))',
    ARRAY['world', 'hello', 'clojure', 'postgresql']
);
 ```
```

### Working with Database Data
```sql
WITH user_data AS (
    SELECT array_agg(name || ',' || age::text) as user_array
    FROM users
)
SELECT pl_clojure_call_array(
    '(fn [& records]
        (let [parsed (map #(clojure.string/split % #",") records)
              filtered (filter (fn [[_ age]]
                               (let [age-num (Integer/parseInt age)]
                                 (and (>= age-num 25) (<= age-num 35))))
                             parsed)]
          (clojure.string/join "|"
            (map (fn [[name age]]
                   (format "%s,%s" name age))
                 filtered))))',
    user_array
);
 ```
```

## Development
Run the test suite:

```bash
./test.sh
 ```

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing
1. Fork the repository
2. Create your feature branch ( git checkout -b feature/amazing-feature )
3. Commit your changes ( git commit -m 'Add some amazing feature' )
4. Push to the branch ( git push origin feature/amazing-feature )
5. Open a Pull Request

## Acknowledgments
- PostgreSQL community
- Clojure community
- JNI contributors