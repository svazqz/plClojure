CREATE FUNCTION pl_clojure_call_array(text, text[]) RETURNS text
AS '/extension/pl_clojure_handler', 'pl_clojure_call_array' LANGUAGE C STRICT;

DO $test$
DECLARE
    recovered text;
BEGIN
    IF pl_clojure_call_array('(fn [& args] (apply str args))', ARRAY['fixture', '-', 'value'])
          IS DISTINCT FROM 'fixture-value' THEN
        RAISE EXCEPTION 'Array values were not passed to Clojure in order';
    END IF;
    IF pl_clojure_call_array('(fn [& args] (count args))', ARRAY[]::text[])
          IS DISTINCT FROM '0' THEN
        RAISE EXCEPTION 'An empty array is expected to pass zero arguments';
    END IF;
    IF pl_clojure_call_array('(fn [& args] (pr-str (vec args)))', ARRAY['sample', NULL])
          IS DISTINCT FROM '["sample" nil]' THEN
        RAISE EXCEPTION 'Array NULL must arrive as Clojure nil';
    END IF;
    IF pl_clojure_call_array('(fn [& args] (count args))', NULL::text[]) IS NOT NULL THEN
        RAISE EXCEPTION 'A NULL SQL array should produce SQL NULL';
    END IF;

    BEGIN
        PERFORM pl_clojure_call_array('(fn [] (throw (Exception. "fixture error")))', ARRAY[]::text[]);
        RAISE EXCEPTION 'Expected Clojure evaluation to fail';
    EXCEPTION WHEN OTHERS THEN
        IF SQLERRM <> 'Failed to evaluate Clojure code' THEN
            RAISE;
        END IF;
    END;

    recovered := pl_clojure_call_array('(fn [x] x)', ARRAY['ok']);
    IF recovered IS DISTINCT FROM 'ok' THEN
        RAISE EXCEPTION 'A previous evaluation error must not poison the session';
    END IF;
END
$test$;
