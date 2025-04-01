#include "postgres.h"     // PostgreSQL core functionality
#include "fmgr.h"        // Function manager interface
#include "utils/builtins.h" // Built-in PostgreSQL utilities
#include "executor/spi.h"   // Server Programming Interface
#include <jni.h>         // Java Native Interface
#include "utils/array.h"  // Add this include for array handling
#include "access/htup_details.h"  // Add this for array functions
#include "catalog/pg_type.h"      // Add this for array functions
#include "utils/lsyscache.h"  // Add this for get_typlenbyvalalign

// PostgreSQL module magic macro - required for all loadable modules
PG_MODULE_MAGIC;

// Global JVM variables that persist between function calls
static JavaVM *jvm = NULL;  // Java Virtual Machine instance
static JNIEnv *env = NULL;  // JNI environment for thread

// Initialize the Java Virtual Machine with Clojure classpath
static void init_jvm(void) {
    if (jvm != NULL) return;  // Only initialize once

    JavaVMInitArgs vm_args;
    JavaVMOption options[1];

    // Set classpath to find Clojure JAR files
    options[0].optionString = "-Djava.class.path=/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/clojure-1.9.0.jar:"
                             "/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/spec.alpha-0.1.143.jar:"
                             "/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/core.specs.alpha-0.1.24.jar";

    // Configure JVM initialization parameters
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = JNI_FALSE;

    // Create the JVM
    if (JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args) != JNI_OK) {
        ereport(ERROR, (errmsg("Failed to create JVM")));
    }
}

// Register function info for PostgreSQL
PG_FUNCTION_INFO_V1(pl_clojure_call);

// Add new function declaration
PG_FUNCTION_INFO_V1(pl_clojure_call_array);

Datum pl_clojure_call_array(PG_FUNCTION_ARGS) {
    if (jvm == NULL) init_jvm();

    text *clojure_code = PG_GETARG_TEXT_PP(0);
    ArrayType *arr = PG_GETARG_ARRAYTYPE_P(1);
    
    // Get array elements
    Datum *elements;
    bool *nulls;
    int nelements;
    deconstruct_array(arr, TEXTOID, -1, false, 'i',
                     &elements, &nulls, &nelements);

    // Build Clojure vector string from array elements
    StringInfoData args_buf;
    initStringInfo(&args_buf);
    appendStringInfoChar(&args_buf, '[');
    
    for (int i = 0; i < nelements; i++) {
        if (i > 0) appendStringInfoString(&args_buf, " ");
        if (!nulls[i]) {
            char *str = TextDatumGetCString(elements[i]);
            appendStringInfoChar(&args_buf, '"');
            appendStringInfoString(&args_buf, str);
            appendStringInfoChar(&args_buf, '"');
            pfree(str);
        } else {
            appendStringInfoString(&args_buf, "nil");
        }
    }
    appendStringInfoChar(&args_buf, ']');

    // Create the Clojure code
    char *code = text_to_cstring(clojure_code);
    StringInfoData buf;
    initStringInfo(&buf);
    appendStringInfo(&buf, "(do (require 'clojure.core) (let [f %s args %s] (apply f args)))", 
                    code, args_buf.data);

    // Get Clojure runtime
    jclass RTClass = (*env)->FindClass(env, "clojure/lang/RT");
    if (!RTClass) {
        (*env)->ExceptionDescribe(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to find Clojure RT class")));
    }

    // Get var method
    jmethodID varMethod = (*env)->GetStaticMethodID(env, RTClass, "var",
        "(Ljava/lang/String;Ljava/lang/String;)Lclojure/lang/Var;");
    if (!varMethod) {
        (*env)->ExceptionDescribe(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to find var method")));
    }

    // Create Java string from Clojure code
    jstring jcode = (*env)->NewStringUTF(env, buf.data);
    if (!jcode) {
        (*env)->ExceptionDescribe(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to create Java string")));
    }

    // Get the Var class and invoke method
    jclass VarClass = (*env)->FindClass(env, "clojure/lang/Var");
    if (!VarClass) {
        (*env)->ExceptionDescribe(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to find Var class")));
    }

    jmethodID invokeMethod = (*env)->GetMethodID(env, VarClass, "invoke",
        "(Ljava/lang/Object;)Ljava/lang/Object;");
    if (!invokeMethod) {
        (*env)->ExceptionDescribe(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to find invoke method")));
    }

    // Get load-string var and evaluate
    jobject loadStringVar = (*env)->CallStaticObjectMethod(env, RTClass, varMethod,
        (*env)->NewStringUTF(env, "clojure.core"),
        (*env)->NewStringUTF(env, "load-string"));
    if (!loadStringVar || (*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to get load-string var")));
    }

    // Evaluate the code
    jobject result = (*env)->CallObjectMethod(env, loadStringVar, invokeMethod, jcode);
    if (!result || (*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to evaluate Clojure code")));
    }

    // Get result's class and toString method
    jclass resultClass = (*env)->GetObjectClass(env, result);
    if (!resultClass) {
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to get result class")));
    }

    jmethodID toStringMethod = (*env)->GetMethodID(env, resultClass, "toString", "()Ljava/lang/String;");
    if (!toStringMethod) {
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to get toString method")));
    }

    // Convert result to string
    jstring resultStr = (jstring)(*env)->CallObjectMethod(env, result, toStringMethod);
    if (!resultStr || (*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        pfree(code);
        pfree(args_buf.data);
        pfree(buf.data);
        ereport(ERROR, (errmsg("Failed to convert result to string")));
    }

    const char* str = (*env)->GetStringUTFChars(env, resultStr, NULL);
    text* output = cstring_to_text(str);
    
    // Cleanup
    (*env)->ReleaseStringUTFChars(env, resultStr, str);
    pfree(code);
    pfree(args_buf.data);
    pfree(buf.data);
    
    PG_RETURN_TEXT_P(output);
}