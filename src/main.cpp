#include <jni.h>
#include <iostream>
#include <cstring>

int main() {
    JavaVM *jvm;
    JNIEnv *env;
    JavaVMInitArgs vm_args;
    JavaVMOption options[1];
    
    options[0].optionString = "-Djava.class.path=lib/spec.alpha-0.1.143.jar:lib/clojure-1.9.0.jar";
    
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;
    
    if (JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args) != JNI_OK) {
        std::cerr << "Error: No se pudo crear la JVM." << std::endl;
        return 1;
    }
    
    jclass Clojure = env->FindClass("clojure/java/api/Clojure");
    if (!Clojure) {
        std::cerr << "Error: No se pudo encontrar la clase Clojure." << std::endl;
        jvm->DestroyJavaVM();
        return 1;
    }
    
    jmethodID var = env->GetStaticMethodID(Clojure, "var", "(Ljava/lang/Object;Ljava/lang/Object;)Lclojure/lang/IFn;");
    jobject load_string = env->CallStaticObjectMethod(Clojure, var, env->NewStringUTF("clojure.core"), env->NewStringUTF("load-string"));
    jmethodID invoke = env->GetMethodID(env->GetObjectClass(load_string), "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");
    env->CallObjectMethod(load_string, invoke, env->NewStringUTF("(println (str \"Hello, World! - \" (+ 1 2)))"));
    
    jvm->DestroyJavaVM();
    return 0;
}