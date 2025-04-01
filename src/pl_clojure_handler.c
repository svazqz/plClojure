#include "postgres.h"     // PostgreSQL core functionality
#include "fmgr.h"        // Function manager interface
#include "utils/builtins.h" // Built-in PostgreSQL utilities
#include "executor/spi.h"   // Server Programming Interface
#include <jni.h>         // Java Native Interface

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
    options[0].optionString = "-Djava.class.path=/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/clojure-1.9.0.jar:/opt/homebrew/opt/postgresql@14/lib/postgresql@14/lib/spec.alpha-0.1.143.jar";

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

// Main function that executes Clojure code
Datum pl_clojure_call(PG_FUNCTION_ARGS) {
    // Initialize JVM if not already running
    if (jvm == NULL) init_jvm();

    // Get input text from PostgreSQL and convert to C string
    text *clojure_code = PG_GETARG_TEXT_PP(0);
    char *code = text_to_cstring(clojure_code);

    // Load the Clojure runtime class (RT)
    jclass RT = (*env)->FindClass(env, "clojure/lang/RT");
    if (!RT) {
        ereport(ERROR, (errmsg("Failed to find Clojure RT class")));
    }

    // Get the static 'var' method from RT class
    // This method is used to look up Clojure vars (functions/values)
    jmethodID var = (*env)->GetStaticMethodID(env, RT, "var", 
        "(Ljava/lang/String;Ljava/lang/String;)Lclojure/lang/Var;");
    if (!var) {
        ereport(ERROR, (errmsg("Failed to find method 'var' in RT")));
    }

    // Get the 'load-string' function from clojure.core
    // This function evaluates Clojure code from a string
    jobject load_string_var = (*env)->CallStaticObjectMethod(env, RT, var,
        (*env)->NewStringUTF(env, "clojure.core"),
        (*env)->NewStringUTF(env, "load-string"));
    if (!load_string_var) {
        ereport(ERROR, (errmsg("Failed to get load-string var")));
    }

    // Get the Var class to access its invoke method
    jclass Var = (*env)->FindClass(env, "clojure/lang/Var");
    if (!Var) {
        ereport(ERROR, (errmsg("Failed to find Var class")));
    }

    // Get the invoke method that will execute our Clojure code
    jmethodID invoke = (*env)->GetMethodID(env, Var, "invoke",
        "(Ljava/lang/Object;)Ljava/lang/Object;");
    if (!invoke) {
        ereport(ERROR, (errmsg("Failed to find 'invoke' method in Var")));
    }

    // Execute the Clojure code by calling load-string
    jobject result = (*env)->CallObjectMethod(env, load_string_var, invoke,
        (*env)->NewStringUTF(env, code));

    // Convert the result to a string using Java's toString()
    jclass Object = (*env)->FindClass(env, "java/lang/Object");
    jmethodID toString = (*env)->GetMethodID(env, Object, "toString",
        "()Ljava/lang/String;");
    
    // Get the string representation of the result
    jstring resultStr = (jstring)(*env)->CallObjectMethod(env, result, toString);
    const char* str = (*env)->GetStringUTFChars(env, resultStr, NULL);
    
    // Convert the result to PostgreSQL text type
    text* output = cstring_to_text(str);
    
    // Clean up JNI resources
    (*env)->ReleaseStringUTFChars(env, resultStr, str);
    pfree(code);
    
    // Return the result to PostgreSQL
    PG_RETURN_TEXT_P(output);
}