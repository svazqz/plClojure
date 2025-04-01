#!/bin/bash

# Stop on first error
set -e

# Connect to postgres database
psql postgres << 'EOF'

-- Drop existing functions if they exist
DROP FUNCTION IF EXISTS pl_clojure_call_array(text, anyarray);

-- Create the function
CREATE FUNCTION pl_clojure_call_array(text, anyarray) RETURNS text 
AS 'pl_clojure_handler', 'pl_clojure_call_array' LANGUAGE C STRICT;

-- Test 1: String concatenation with uppercase transformation
SELECT pl_clojure_call_array(
    '(fn [& args] (clojure.string/upper-case (apply str args)))',
    ARRAY['hello', ' ', 'world']
) as uppercase_test;

-- Test 2: Number manipulation
SELECT pl_clojure_call_array(
    '(fn [& args] (str "Sum: " (apply + (map #(Integer/parseInt %) args))))',
    ARRAY['10', '20', '30', '40']
) as sum_test;

-- Test 3: Complex string manipulation
SELECT pl_clojure_call_array(
    '(fn [& args] 
        (let [words args
              sorted (sort words)
              capitalized (map clojure.string/capitalize sorted)]
          (clojure.string/join ", " capitalized)))',
    ARRAY['world', 'hello', 'clojure', 'postgresql']
) as sort_capitalize_test;

-- Test 4: Nested function calls with filtering
SELECT pl_clojure_call_array(
    '(fn [& args]
        (let [numbers (map #(Integer/parseInt %) args)
              evens (filter even? numbers)
              doubled (map #(* 2 %) evens)]
          (str "Doubled evens: " (clojure.string/join ", " doubled))))',
    ARRAY['1', '2', '3', '4', '5', '6']
) as filter_double_test;

-- Test 5: String transformation with regex
SELECT pl_clojure_call_array(
    '(fn [& args]
        (let [text (apply str args)
              words (clojure.string/split text #"\s+")
              word-count (count words)]
          (format "Word count: %d, Characters: %d" 
                 word-count
                 (count text))))',
    ARRAY['The quick brown', ' ', 'fox jumps over', ' ', 'the lazy dog']
) as text_analysis_test;

-- Create test table and data
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    name TEXT,
    email TEXT,
    age INTEGER
);

-- Insert sample data
INSERT INTO users (name, email, age) VALUES
    ('John Doe', 'john@example.com', 30),
    ('Jane Smith', 'jane@example.com', 25),
    ('Bob Wilson', 'bob@example.com', 45),
    ('Alice Brown', 'alice@example.com', 35);

-- Test 6: Process user data with Clojure
WITH user_data AS (
    SELECT array_agg(name || ',' || email || ',' || age::text) as user_array
    FROM users
)
SELECT pl_clojure_call_array(
    '(fn [& records]
        (let [parsed-records (map #(clojure.string/split % #",") records)
              formatted (map (fn [[name email age]]
                             (format "%s (%s) - Age: %s" 
                                    (clojure.string/upper-case name)
                                    email
                                    age))
                           parsed-records)]
          (clojure.string/join "\n" formatted)))',
    user_array
) as formatted_users
FROM user_data;

-- Test 7: Calculate average age with Clojure
WITH age_data AS (
    SELECT array_agg(age::text) as age_array
    FROM users
)
SELECT pl_clojure_call_array(
    '(fn [& ages]
        (let [numbers (map #(Integer/parseInt %) ages)
              avg (/ (apply + numbers) (count numbers))]
          (format "Average age: %.2f" (double avg))))',
    age_array
) as average_age
FROM age_data;

-- Test 8: Find users by age range (table output)
WITH user_data AS (
    SELECT array_agg(name || ',' || age::text) as user_array
    FROM users
), 
filtered_data AS (
    SELECT unnest(string_to_array(pl_clojure_call_array(
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
    ), '|')) as record
    FROM user_data
)
SELECT 
    split_part(record, ',', 1) as name,
    split_part(record, ',', 2)::integer as age
FROM filtered_data
ORDER BY age;

-- Clean up
DROP TABLE IF EXISTS users;
DROP FUNCTION IF EXISTS pl_clojure_call_array(text, anyarray);

EOF